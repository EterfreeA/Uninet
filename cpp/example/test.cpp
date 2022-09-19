#include "Framework.h"
#include "Log/Logger.h"

#include <cstdlib>
//#include <chrono>
#include <memory>
#include <vector>
#include <iostream>

int main()
{
	USING_ETERFREE_SPACE;

	auto logger = Logger::get(Logger::MODE::MULTI_THREAD);
	if (logger)
	{
		Logger::Limit limit;
		limit._quantity = 3;
		limit._size = 64;
		logger->setLimit(limit);

		logger->setFile(Logger::File{ "log", "framework", ".txt" });
	}

	for (auto index = 0; index < 3; ++index)
	{
		using VectorType = std::vector<int>;
		//std::unique_ptr<VectorType> vector(make<VectorType>(1, 0));
		std::unique_ptr<VectorType> vector(make<VectorType>(UINT64_MAX, 0));
		if (vector == nullptr)
		{
			if (logger)
			{
				logger->input(Logger::LEVEL::ERROR, \
					std::source_location::current(), \
					[](std::string& _description) -> decltype(_description) \
					{ return _description = "vector is nullptr"; });
				logger->execute();
			}
			continue;
		}

		for (const auto& element : *vector)
			std::cout << element << ' ';
		std::cout << "\b\n";
	}

	//if (logger)
	//	logger->setFile(Logger::File{ "log", "framework", ".txt" });

	//using namespace std::chrono;
	//std::timespec ts;
	//if (std::timespec_get(&ts, TIME_UTC) == TIME_UTC)
	//	std::cout << ts.tv_sec << ' ' << ts.tv_nsec << std::endl;

	//auto time = time_point_cast<sys_time<nanoseconds>>(system_clock::now());
	//std::cout << time << std::endl;
	return EXIT_SUCCESS;
}
