#include "Signal.h"

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
	//// �ⲿ�жϣ�ͨ��Ϊ�û�����ʼ����Ctrl + C��
	//Signal::set(SIGINT, SIG_DFL);
	//// �Ƿ�����ӳ������Ƿ�ָ��
	//Signal::set(SIGILL, SIG_DFL);
	//// �����������㣬���������
	//Signal::set(SIGFPE, SIG_DFL);
	//// �Ƿ��ڴ���ʣ��ֶδ���
	//Signal::set(SIGSEGV, SIG_DFL);
	//// ���͸��������ֹ����
	//Signal::set(SIGTERM, SIG_DFL);
	//// Ctrl-Break����
	//Signal::set(SIGBREAK, SIG_DFL);
	//// �쳣��ֹ����������Ϊ std::abort() ����ʼ
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
