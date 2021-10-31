#include "Core.hpp"
#include "Connector.hpp"
#include "Singleton.hpp"
#include "Queue.hpp"
#include "ByteOrder.hpp"
#include "SessionManager.hpp"

#include "spin_mutex.hpp"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <utility>
#include <chrono>
#include <cstddef>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <thread>
#include <mutex>

NETWORK_BEGIN

ETERFREE
BYTE_ORDER

class TCPSession : public SessionManager::Session
{
	std::optional<IDType> _id;  // 标识
	SOCKET _socket;             // 套接字
	SOCKADDR_IN _address;       // 地址
	InputStream _inputStream;   // 输入流
	OutputStream _outputStream; // 输出流

public:
	TCPSession() noexcept = default;
	TCPSession(SOCKET _socket, const SOCKADDR_IN& _address) noexcept
		: _socket(_socket), _address(_address) {}

	virtual explicit operator bool() const noexcept override
	{
		return _id.has_value();
	}

	inline void setID(IDType _id) noexcept
	{
		this->_id = _id;
	}

	virtual std::optional<IDType> getID() const noexcept override
	{
		return _id;
	}

	inline void setSocket(SOCKET _socket) noexcept
	{
		this->_socket = _socket;
	}

	virtual SocketType getSocket() const noexcept override
	{
		return _socket;
	}

	inline void setAddress(const SOCKADDR_IN& _address) noexcept
	{
		this->_address = _address;
	}

	virtual InputStream& getInputStream() noexcept override
	{
		return _inputStream;
	}

	virtual OutputStream& getOutputStream() noexcept override
	{
		return _outputStream;
	}
};

struct Connector::Structure
{
	enum class State
	{
		EMPTY,
		INITIALIZED,
		LISTENING,
		REINITIALIZED,
	};

	struct Address
	{
		IDType _id;
		PortType _port;
		SOCKADDR_IN _address;
	};

	INTERNET_PROTOCOL _type; // IP类型

	std::optional<PortType> _port; // 端口
	spin_mutex _portMutex;         // 端口互斥元

	SOCKET _socket;       // 套接字
	SOCKADDR_IN _address; // 地址
	bool _listening;      // 监听状态

	Queue<Address> _addressQueue;        // 地址队列
	std::vector<fd_set> _connectSet;     // 连接集合
	std::map<SOCKET, Address> _mappings; // 连接映射

	int _length;                          // 发送长度
	SessionManager::SendFunctor _functor; // 发送函数子

	std::vector<fd_set> _fdSet;              // 文件描述符集合
	std::map<SocketType, SizeType> _counter; // 套接字计数器

	std::thread _thread;     // 线程
	volatile bool _running;  // 执行状态
	std::mutex _threadMutex; // 线程互斥元

	Structure(INTERNET_PROTOCOL _type);

	static inline auto getMaxSocket(const fd_set& _fdSet)
	{
		auto end = _fdSet.fd_array + _fdSet.fd_count;
		auto iterator = std::max_element(_fdSet.fd_array, end);
		return static_cast<int>(iterator != end ? *iterator : INVALID_SOCKET);
	}

	static void addSocket(std::vector<fd_set>& _fdSet, SocketType _socket) noexcept;

	static void removeSocket(std::vector<fd_set>& _fdSet, const std::vector<SocketPair>& _pairs) noexcept;

	SizeType increase(SocketType _socket);
	SizeType getCount(SocketType _socket);

	void clear();
};

Connector::Structure::Structure(INTERNET_PROTOCOL _type)
	: _type(_type), _listening(false), _length(0), _running(false)
{
#define FUNCTION STRING([Connector::Structure::Structure(INTERNET_PROTOCOL)])
	_functor = [this](SocketType _socket, const char* _data, int _size)
	{
		if (_data == nullptr || _size <= 0)
			return 0;

		auto socket = static_cast<SOCKET>(_socket);
		_length = ::send(socket, _data, _size, 0);
		if (_length != SOCKET_ERROR)
			return _length;

		std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": send data failed!" << std::endl;
		return 0;
	};
#undef FUNCTION
}

void Connector::Structure::addSocket(std::vector<fd_set>& _fdSet, SocketType _socket) noexcept
{
	auto socket = static_cast<SOCKET>(_socket);
	for (auto& set : _fdSet)
		if (set.fd_count < FD_SETSIZE)
		{
			FD_SET(_socket, &set);
			return;
		}

	_fdSet.emplace_back(fd_set());
	FD_ZERO(&_fdSet.back());
	FD_SET(_socket, &_fdSet.back());
}

void Connector::Structure::removeSocket(std::vector<fd_set>& _fdSet, const std::vector<SocketPair>& _pairs) noexcept
{
	for (const auto& pair : _pairs)
		if (pair.second < _fdSet.size())
		{
			auto& set = _fdSet[pair.second];
			FD_CLR(pair.first, &set);
		}
}

SizeType Connector::Structure::increase(SocketType _socket)
{
	auto iterator = _counter.find(_socket);
	if (iterator != _counter.end())
		return ++iterator->second;

	SizeType counter = 1;
	_counter.emplace(_socket, counter);
	return counter;
}

SizeType Connector::Structure::getCount(SocketType _socket)
{
	auto iterator = _counter.find(_socket);
	return iterator != _counter.end() ? iterator->second : 0;
}

void Connector::Structure::clear()
{
	// 关闭监听套接字
	if (_listening)
	{
		::closesocket(_socket);
		_listening = false;
	}

	// 清空地址队列
	_addressQueue.clear();

	// 关闭连接套接字
	for (const auto& set : _connectSet)
		for (decltype(set.fd_count) index = 0; index < set.fd_count; ++index)
			::closesocket(set.fd_array[index]);

	// 清空连接套接字集合
	_connectSet.clear();

	// 清空连接映射
	_mappings.clear();

	// 关闭文件描述符
	for (const auto& set : _fdSet)
		for (decltype(set.fd_count) index = 0; index < set.fd_count; ++index)
			::closesocket(set.fd_array[index]);

	// 清空文件描述符集合
	_fdSet.clear();

	// 重置套接字计数器
	_counter.clear();
}

void Connector::onListen(DataType& _data)
{
#define FUNCTION STRING([Connector::onListen(DataType&)])
	std::unique_lock lock(_data->_portMutex);
	if (!_data->_port)
		return;

	auto port = _data->_port.value();
	lock.unlock();

	if (_data->_listening && port != ntoh<PortType, PortType>(_data->_address.sin_port))
	{
		::closesocket(_data->_socket);
		_data->_listening = false;
	}

	if (!_data->_listening)
	{
		// 创建套接字
		_data->_socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_data->_socket == INVALID_SOCKET)
		{
			std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": initialize socket failed!" << std::endl;
			return;
		}

		// 初始化地址
		auto& address = _data->_address;
		::memset(&address, 0, sizeof address);       // 置零
		address.sin_family = PF_INET;                // 协议
		address.sin_addr.s_addr = htonl(INADDR_ANY); // 指定通配地址，匹配任何IP地址
		address.sin_port = hton(port);               // 端口

		// 绑定地址
		if (::bind(_data->_socket, reinterpret_cast<LPSOCKADDR>(&address), sizeof address) == SOCKET_ERROR)
		{
			std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": bind address failed!" << std::endl;
			::closesocket(_data->_socket);
			return;
		}

		// 监听连接
		if (::listen(_data->_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			std::cerr << FUNCTION << "Error" << ::WSAGetLastError() << ": listen socket failed!" << std::endl;
			::closesocket(_data->_socket);
			return;
		}

		_data->_listening = true;
	}
#undef FUNCTION
}

void Connector::onAccept(DataType& _data)
{
#define FUNCTION STRING([Connector::onAccept(DataType&)])
	SOCKADDR_IN address;
	auto size = static_cast<int>(sizeof address); // 地址长度
	if (SOCKET socket = ::accept(_data->_socket, reinterpret_cast<LPSOCKADDR>(&address), &size); \
		socket == INVALID_SOCKET)
	{
		if (_data->_running)
			std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": accept socket failed!" << std::endl;
	}
	else //if (socket > 2) // 0-stdin 1-stdout 2-stderr
	{
		std::cout << FUNCTION << "Accept socket[" << socket << "] successful!" << std::endl;

		// 非阻塞模式
		u_long flag = 1;
		if (::ioctlsocket(socket, FIONBIO, &flag) == SOCKET_ERROR)
			std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": set non-blocking for socket[" << socket << "] failed!" << std::endl;

		auto count = _data->increase(socket);
		Singleton<SessionManager>::getInstance().onConnect(SocketPair(socket, count), std::make_shared<TCPSession>(socket, address));
		Structure::addSocket(_data->_fdSet, socket);
	}
	//else
	//	std::cout << FUNCTION << "Ignore socket[" << socket << "]!" << std::endl;
#undef FUNCTION
}

void Connector::onConnect(DataType& _data)
{
#define FUNCTION STRING([Connector::connect(DataType&)])
	if (_data->_addressQueue.empty())
		return;

	auto result = _data->_addressQueue.pop();
	if (!result)
		return;

	// 初始化套接字
	auto socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket == INVALID_SOCKET)
	{
		// WSAGetLastError 返回当前线程进行最后一次Windows Sockets API函数调用的错误代码
		std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": initialize socket failed!" << std::endl;
		return;
	}

	// 初始化地址
	auto& address = result.value();
	SOCKADDR_IN fromAddress;
	::memset(&fromAddress, 0, sizeof fromAddress);   // 置零
	fromAddress.sin_family = PF_INET;                // Internet协议族
	fromAddress.sin_addr.s_addr = htonl(INADDR_ANY); // 自动获取本机地址
	fromAddress.sin_port = hton(address._port);      // 端口 0-自动分配端口

	// 绑定地址
	if (::bind(socket, reinterpret_cast<LPSOCKADDR>(&fromAddress), sizeof fromAddress) == SOCKET_ERROR)
	{
		std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": bind address failed!" << std::endl;
		::closesocket(socket);
		return;
	}

	SOCKADDR_IN& toAddress = address._address;

	// 阻塞模式
	//if (::connect(socket, reinterpret_cast<LPSOCKADDR>(&toAddress), sizeof(SOCKADDR)) == SOCKET_ERROR)
	//{
	//	std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": establish connection failed!" << std::endl;
	//	::closesocket(socket);
	//	return;
	//}

	// 非阻塞模式
	u_long flag = 1;
	if (::ioctlsocket(socket, FIONBIO, &flag) == SOCKET_ERROR)
		std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": set non-blocking for socket[" << socket << "] failed!" << std::endl;

	// 非阻塞模式
	if (::connect(socket, reinterpret_cast<LPSOCKADDR>(&toAddress), sizeof toAddress) != SOCKET_ERROR)
	{
		auto count = _data->increase(socket);
		auto session = std::make_shared<TCPSession>(socket, toAddress);
		session->setID(address._id);
		Singleton<SessionManager>::getInstance().onConnect(SocketPair(socket, count), session);
		Structure::addSocket(_data->_fdSet, socket);
		return;
	}

	if (auto error = ::WSAGetLastError(); error != WSAEWOULDBLOCK)
	{
		std::cerr << FUNCTION << "Error " << error << ": establish connection failed!" << std::endl;
		::closesocket(socket);
		return;
	}

	Structure::addSocket(_data->_connectSet, socket);
	_data->_mappings.emplace(static_cast<SocketType>(socket), address);
#undef FUNCTION
}

void Connector::onDisconnect(DataType& _data, SocketType _socket)
{
#define FUNCTION STRING([Connector::onDisconnect(DataType&, SocketType)])
	std::cout << FUNCTION << "Disconnect for socket[" << _socket << "]!" << std::endl;
	Singleton<SessionManager>::getInstance().onDisconnect(SocketPair(_socket, _data->getCount(_socket)));
	auto socket = static_cast<SOCKET>(_socket);
	::closesocket(socket);
#undef FUNCTION
}

bool Connector::onReceive(DataType& _data, SocketType _socket)
{
#define FUNCTION STRING([Connector::onReceive(DataType&, SocketType)])
	static thread_local char buffer[SOCKET_BUFFER_SIZE]; // 接收缓冲区
	auto socket = static_cast<SOCKET>(_socket);
	auto length = ::recv(socket, buffer, sizeof buffer, 0);
	if (length == 0)
	{
		onDisconnect(_data, _socket);
		return false;
	}

	if (length > 0)
		Singleton<SessionManager>::getInstance().onReceive(SocketPair(_socket, _data->getCount(_socket)), buffer, length);
	else if (auto error = errno; length < 0 && error != EINTR && error != EAGAIN && error != EWOULDBLOCK)
	{
		std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": receive data failed!" << std::endl;
		onDisconnect(_data, _socket);
		return false;
	}
	return true;
#undef FUNCTION
}

bool Connector::onSend(DataType& _data, SocketType _socket)
{
#define FUNCTION STRING([Connector::onSend(DataType&, SocketType)])
	_data->_length = 0;
	Singleton<SessionManager>::getInstance().onSend(SocketPair(_socket, _data->getCount(_socket)), SOCKET_BUFFER_SIZE, _data->_functor);
	if (_data->_length == SOCKET_ERROR)
	{
		onDisconnect(_data, _socket);
		return false;
	}
	return true;
#undef FUNCTION
}

void Connector::execute(DataType _data)
{
#define FUNCTION STRING([Connector::execute(DataType)])
	fd_set readSet; // 可读套接字集合
	fd_set writeSet; // 可写套接字集合
	fd_set exceptSet; // 异常套接字集合

	struct timeval listenTimeout { 0, 1000 }; // 监听超时
	struct timeval otherTimeout { 0, 0 }; // 其他超时
	std::vector<SocketPair> removePairs; // 移除套接字缓存

	while (_data->_running)
	{
		// 降低处理器占用率
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		// 监听连接
		onListen(_data);

		if (_data->_listening)
		{
			FD_ZERO(&readSet);
			FD_SET(_data->_socket, &readSet);
			auto counter = ::select(static_cast<int>(_data->_socket + 1), &readSet, NULL, NULL, &listenTimeout);
			if (counter < 0)
				std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": select listening socket failed!" << std::endl;
			else if (counter > 0 && FD_ISSET(_data->_socket, &readSet))
				onAccept(_data);
		}

		// 建立连接
		onConnect(_data);

		// 轮询套接字读写状态
		for (decltype(_data->_fdSet.size()) index = 0; index < _data->_fdSet.size(); ++index)
		{
			readSet = _data->_fdSet[index];
			if (readSet.fd_count <= 0)
				continue;

			// 轮询套接字是否准备就绪
			auto maxSocket = Structure::getMaxSocket(readSet);
			if (maxSocket == INVALID_SOCKET)
				continue;

			writeSet = exceptSet = readSet;
			auto counter = ::select(maxSocket + 1, &readSet, &writeSet, &exceptSet, &otherTimeout);
			if (counter == 0)
				continue;
			if (counter < 0)
			{
				std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": select socket failed!" << std::endl;
				continue;
			}

			// 可读套接字
			for (decltype(readSet.fd_count) count = 0; count < readSet.fd_count; ++count)
			{
				// 若套接字准备就绪
				auto socket = readSet.fd_array[count];
				if (FD_ISSET(socket, &readSet))
				{
					if (!onReceive(_data, socket))
					{
						FD_CLR(socket, &writeSet);
						FD_CLR(socket, &exceptSet);
						removePairs.emplace_back(static_cast<SocketType>(socket), static_cast<SizeType>(index));
					}

					if (--counter <= 0)
						break;
				}
			}

			// 可写套接字
			if (counter > 0)
				for (decltype(writeSet.fd_count) count = 0; count < writeSet.fd_count; ++count)
				{
					auto socket = writeSet.fd_array[count];
					if (FD_ISSET(socket, &writeSet))
					{
						if (!onSend(_data, socket))
						{
							FD_CLR(socket, &exceptSet);
							removePairs.emplace_back(static_cast<SocketType>(socket), static_cast<SizeType>(index));
						}

						if (--counter <= 0)
							break;
					}
				}

			// 异常套接字
			if (counter > 0)
				for (decltype(exceptSet.fd_count) count = 0; count < exceptSet.fd_count; ++count)
				{
					auto socket = exceptSet.fd_array[count];
					if (FD_ISSET(socket, &exceptSet))
					{
						onDisconnect(_data, socket);
						removePairs.emplace_back(static_cast<SocketType>(socket), static_cast<SizeType>(index));

						if (--counter <= 0)
							break;
					}
				}
		}

		// 移除断开连接和异常套接字
		Structure::removeSocket(_data->_fdSet, removePairs);
		removePairs.clear();

		for (decltype(_data->_connectSet.size()) index = 0; index < _data->_connectSet.size(); ++index)
		{
			writeSet = _data->_connectSet[index];
			if (writeSet.fd_count <= 0)
				continue;

			auto maxSocket = Structure::getMaxSocket(writeSet);
			if (maxSocket == INVALID_SOCKET)
				continue;

			exceptSet = writeSet;
			auto counter = select(maxSocket + 1, NULL, &writeSet, &exceptSet, &otherTimeout);
			if (counter == 0)
				continue;
			if (counter < 0)
			{
				std::cerr << FUNCTION << "Error " << ::WSAGetLastError() << ": select socket failed!" << std::endl;
				continue;
			}

			// 可写套接字
			for (decltype(writeSet.fd_count) count = 0; count < writeSet.fd_count; ++count)
			{
				// 若套接字准备就绪
				auto socket = writeSet.fd_array[count];
				if (FD_ISSET(socket, &writeSet))
				{
					auto iterator = _data->_mappings.find(socket);
					if (iterator == _data->_mappings.end())
					{
						std::cerr << FUNCTION << "Mapping address not found for socket[" << socket << "]!" << std::endl;
						::closesocket(socket);
						FD_CLR(socket, &exceptSet);
					}
					else
					{
						std::cout << FUNCTION << "Connect successful for socket[" << socket << "]!" << std::endl;
						const auto& address = iterator->second;
						auto count = _data->increase(socket);
						auto session = std::make_shared<TCPSession>(socket, address._address);
						session->setID(address._id);
						_data->_mappings.erase(iterator);

						Singleton<SessionManager>::getInstance().onConnect(SocketPair(socket, count), session);
						Structure::addSocket(_data->_fdSet, socket);
					}

					removePairs.emplace_back(static_cast<SocketType>(socket), static_cast<SizeType>(index));
					if (--counter <= 0)
						break;
				}
			}

			// 异常套接字
			if (counter > 0)
				for (decltype(exceptSet.fd_count) count = 0; count < exceptSet.fd_count; ++count)
				{
					auto socket = exceptSet.fd_array[count];
					if (FD_ISSET(socket, &exceptSet))
					{
						int error, size = static_cast<decltype(error)>(sizeof error);
						getsockopt(socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &size); // 获得错误号
						std::cerr << FUNCTION << "Error " << error << ": establish connection failed!" << std::endl;

						::closesocket(socket);
						removePairs.emplace_back(static_cast<SocketType>(socket), static_cast<SizeType>(index));
						if (--counter <= 0)
							break;
					}
				}
		}

		// 移除断开连接和异常套接字
		Structure::removeSocket(_data->_connectSet, removePairs);
		removePairs.clear();
	}

	// 清空连接
	SessionManager& sessionManager = Singleton<SessionManager>::getInstance();
	sessionManager.clear();

	_data->clear();

	std::lock_guard lock(_data->_portMutex);
	_data->_port = std::nullopt;
#undef FUNCTION
}

Connector::Connector(INTERNET_PROTOCOL _type)
	: _data(std::make_shared<Structure>(_type)) {}

// 监听
bool Connector::listen(PortType _port)
{
#define FUNCTION STRING([Connector::listen(PortType)])
	if (_data->_type != INTERNET_PROTOCOL::IPv4)
	{
		std::cerr << FUNCTION << "Not support internet protocol: " << static_cast<uint8_t>(_data->_type) << std::endl;
		return false;
	}

	std::lock_guard lock(_data->_portMutex);
	return (_data->_port = _port).has_value();
#undef FUNCTION
}

// 连接
bool Connector::connect(IDType _id, const char* _ip, PortType _fromPort, PortType _toPort)
{
#define FUNCTION STRING([Connector::connect(const char*, PortType, PortType)])
	if (_data->_type != INTERNET_PROTOCOL::IPv4)
	{
		std::cerr << FUNCTION << "Not support internet protocol: " << static_cast<uint8_t>(_data->_type) << std::endl;
		return false;
	}

	// 初始化地址
	SOCKADDR_IN address;
	::memset(&address, 0, sizeof address);             // 置零
	address.sin_family = PF_INET;                      // 协议
	//toAddress.sin_addr.s_addr = inet_addr(_ip);
	inet_pton(PF_INET, _ip, &address.sin_addr.s_addr); // 地址
	address.sin_port = hton(_toPort);                  // 端口

	return _data->_addressQueue.push(Structure::Address{ _id, _fromPort, address }).has_value();
#undef FUNCTION
}

// 启动
bool Connector::start()
{
#define FUNCTION STRING([Connector::start()])
	std::lock_guard lock(_data->_threadMutex);
	if (_data->_running)
	{
		std::cerr << FUNCTION << "The thread is running!" << std::endl;
		return false;
	}

	if (!initNetwork())
		return false;

	_data->_running = true;
	_data->_thread = std::thread(Connector::execute, _data);
	return true;
#undef FUNCTION
}

// 终止
void Connector::stop()
{
	std::lock_guard lock(_data->_threadMutex);
	if (!_data->_running)
		return;

	_data->_running = false;

	// 等待线程退出
	if (_data->_thread.joinable())
		_data->_thread.join();
}

NETWORK_END
