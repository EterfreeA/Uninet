#include <chrono>
#include <thread>

#include "Connector.hpp"

NETWORK

int main()
{
	// server
	Connector connector;
	connector.listen();
	connector.start();
	std::this_thread::sleep_for(std::chrono::seconds(10));

	// client
	//Connector connector;
	//for (int index = 0; index < 100; ++index)
	//	connector.connect(index, "127.0.0.1", 0);
	//connector.start();
	//std::this_thread::sleep_for(std::chrono::seconds(3));
	return 0;
}
