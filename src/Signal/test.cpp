#include "Signal.hpp"

#include <chrono>
#include <iostream>
#include <thread>

void test(int signal)
{
	std::cerr << "Received interrupt signal " << signal << '.' << std::endl;
}

int main()
{
	using eterfree::Signal;
	//// 外部中断，通常为用户所起始（如Ctrl + C）
	//Signal::set(SIGINT, SIG_DFL);
	//// 非法程序映像，例如非法指令
	//Signal::set(SIGILL, SIG_DFL);
	//// 错误算术运算，例如除以零
	//Signal::set(SIGFPE, SIG_DFL);
	//// 非法内存访问（分段错误）
	//Signal::set(SIGSEGV, SIG_DFL);
	//// 发送给程序的终止请求
	//Signal::set(SIGTERM, SIG_DFL);
	//// Ctrl-Break序列
	//Signal::set(SIGBREAK, SIG_DFL);
	//// 异常终止条件，例如为 std::abort() 所起始
	//Signal::set(SIGABRT, SIG_DFL);

	Signal::insert(SIGINT, 1, test);
	Signal::insert(SIGINT, 2, test);
	Signal::erase(SIGINT, 1);

	//volatile int integer = 0;
	//integer = 1 / integer;

	auto start = std::chrono::high_resolution_clock::now();
	using namespace std::chrono_literals; // C++14
	std::this_thread::sleep_for(2s);
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = stop - start;
	std::cout << "Sleeped for " << elapsed.count() << " milliseconds." << std::endl;
	return 0;
}
