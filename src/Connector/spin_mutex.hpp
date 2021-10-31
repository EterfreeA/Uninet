#pragma once

#include <atomic>

class spin_mutex
{
	std::atomic_flag _flag;

public:
	constexpr spin_mutex() noexcept {}
	spin_mutex(const spin_mutex&) = delete;
	spin_mutex& operator=(const spin_mutex&) = delete;

	void lock() noexcept
	{
		while (_flag.test_and_set());
	}

	void lock() volatile noexcept
	{
		while (_flag.test_and_set());
	}

	bool try_lock() noexcept
	{
		return !_flag.test_and_set();
	}

	bool try_lock() volatile noexcept
	{
		return !_flag.test_and_set();
	}

	void unlock() noexcept
	{
		_flag.clear();
	}

	void unlock() volatile noexcept
	{
		_flag.clear();
	}
};
