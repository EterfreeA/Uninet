#pragma once

#include <memory>

#include "Network.hpp"

NETWORK_BEGIN

class Connector
{
	struct Structure;
	using DataType = std::shared_ptr<Structure>;

private:
	DataType _data;

private:
	static void onListen(DataType& _data);
	static void onAccept(DataType& _data);
	static void onConnect(DataType& _data);
	static void onDisconnect(DataType& _data, SocketType _socket);
	static bool onReceive(DataType& _data, SocketType _socket);
	static bool onSend(DataType& _data, SocketType _socket);

private:
	static void execute(DataType _data);

public:
	Connector(INTERNET_PROTOCOL _type = INTERNET_PROTOCOL::IPv4);

	Connector(const Connector&) = delete;

	~Connector() { stop(); }

	Connector& operator=(const Connector&) = delete;

	// 监听
	bool listen(PortType _port = 9999);

	// 连接
	bool connect(IDType _id, const char* _ip, PortType _fromPort, PortType _toPort = 9999);

	// 启动
	bool start();

	// 终止
	void stop();
};

NETWORK_END
