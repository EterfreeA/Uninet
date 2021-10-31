#include "Core.hpp"
#include "Network.hpp"
#include "Singleton.hpp"

#include <WinSock2.h>

#include <iostream>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")

NETWORK_BEGIN

class WSA
{
	friend WSA& Singleton<WSA>::getInstance();

private:
	bool _valid;
	WSADATA wsaData; // Winsock.dll data

private:
	WSA() noexcept : _valid(false) {}

public:
	~WSA()
	{
		WSACleanup();
	}

	explicit operator bool() const noexcept
	{
		return _valid;
	}

	bool init()
	{
#define FUNCTION STRING([WSA::init()])
		if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
		{
			std::cerr << FUNCTION << "Initialize WSAData failed!" << std::endl;
			return false;
		}

		return _valid = true;
#undef FUNCTION
	}
};

bool initNetwork()
{
	static std::mutex mutex;
	std::lock_guard lock(mutex);

	WSA& wsa = Singleton<WSA>::getInstance();
	return wsa || wsa.init();
}

NETWORK_END
