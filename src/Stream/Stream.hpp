#pragma once

#include <string>
#include <queue>

class OutputStream
{
	using QueueType = std::queue<std::string>;

public:
	using SizeType = std::string::size_type;

private:
	std::string buffer;
	QueueType queue;

public:
	bool empty() const
	{
		return buffer.empty() && queue.empty();
	}

	const char* data(SizeType& length);

	void push(const char* data, SizeType length)
	{
		queue.emplace(data, length);
	}

	void push(const std::string& string)
	{
		queue.emplace(string);
	}

	void pop(SizeType length)
	{
		buffer.erase(0, length);
	}

	void clear();
};

class InputStream
{
	using QueueType = std::queue<std::string>;

public:
	using SizeType = std::string::size_type;

private:
	SizeType capacity = 0, offset = 0;
	std::string buffer;
	QueueType queue;

public:
	bool empty() const
	{
		return queue.empty();
	}

	void push(const char* data, SizeType length);

	void push(const std::string& string)
	{
		push(string.data(), string.size());
	}

	bool pop(std::string& string);

	void clear();
};
