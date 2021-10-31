#pragma once

#include <functional>
#include <utility>
#include <optional>
#include <memory>
#include <list>
#include <map>
#include <mutex>

#include "Singleton.hpp"
#include "Stream.hpp"
#include "Network.hpp"

NETWORK_BEGIN

class SessionManager
{
	friend SessionManager& Singleton<SessionManager>::getInstance();

public:
	struct Index
	{
		SocketPair _pair;

		Index() noexcept = default;
		Index(const SocketPair& _pair) noexcept : _pair(_pair) {}

		Index& operator=(const Index& _another) noexcept
		{
			if (&_another != this)
				this->_pair = _another._pair;
		}

		inline bool operator==(const Index& _another) const noexcept
		{
			return this->_pair == _another._pair;
		}

		inline bool operator<(const Index& _another) const noexcept
		{
			return this->_pair.first < _another._pair.first
				|| this->_pair.first == _another._pair.first
				&& this->_pair.second < _another._pair.second;
		}
	};

	class Session
	{
	public:
		virtual ~Session() = default;
		virtual explicit operator bool() const { return false; }
		virtual std::optional<IDType> getID() const = 0;
		virtual SocketType getSocket() const = 0;
		virtual InputStream& getInputStream() = 0;
		virtual OutputStream& getOutputStream() = 0;
	};

	class Proxy
	{
	public:
		using SessionPair = std::pair<Index, std::shared_ptr<Session>>;

	private:
		SessionManager& _manager;
		SessionPair _pair;

	public:
		Proxy(SessionManager& _manager, const SessionPair& _pair) noexcept
			: _manager(_manager), _pair(_pair) {}

		~Proxy();

		inline const Index& getIndex() const noexcept
		{
			return _pair.first;
		}

		inline Session& getSession() noexcept
		{
			return *_pair.second;
		}
	};

	friend Proxy::~Proxy();

	using SessionMap = std::map<Index, std::shared_ptr<Session>>;
	using SessionPair = Proxy::SessionPair;
	using MessageQueue = std::list<SessionPair>;
	using SendFunctor = std::function<int(SocketType, const char*, int)>;

private:
	SessionMap _sessions;
	std::map<IDType, std::shared_ptr<Session>> _idMap;
	std::mutex _sessionMutex;

	SessionMap _mappings;
	MessageQueue _queue;
	std::mutex _queueMutex;

private:
	void onPush(const Index& _index);
	void onPop(const Index& _index);

private:
	SessionManager() = default;

public:
	bool onConnect(const Index& _index, std::shared_ptr<Session> _session);
	void onDisconnect(const Index& _index);
	void onReceive(const Index& _index, const char* _data, SizeType _size);
	void onSend(const Index& _index, SizeType _size, SendFunctor& _functor);

public:
	SessionManager(const SessionManager&) = delete;
	SessionManager& operator=(const SessionManager&) = delete;

	void clear();
	std::unique_ptr<Proxy> dispatch();
};

NETWORK_END
