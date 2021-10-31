#pragma once

#include <memory>
#include <string>
#include <queue>
#include <mutex>

class OutputStream
{
	using QueueType = std::queue<std::string>;

public:
	class Proxy;
	friend class Proxy;

	using SizeType = std::string::size_type;

private:
	std::string _buffer;
	QueueType _queue;
	mutable std::mutex _mutex;

private:
	inline std::mutex& mutex() noexcept { return _mutex; }

	inline bool empty() const noexcept
	{
		return _buffer.empty() && _queue.empty();
	}

	const char* data(SizeType& _size);

	inline void pop(SizeType _size)
	{
		_buffer.erase(0, _size);
	}

public:
	void push(const char* data, SizeType _size)
	{
		std::lock_guard lock(_mutex);
		_queue.emplace(data, _size);
	}

	void push(const std::string& _string)
	{
		std::lock_guard lock(_mutex);
		_queue.emplace(_string);
	}

	inline std::unique_ptr<Proxy> pop()
	{
		return std::make_unique<Proxy>(*this);
	}

	void clear();
};

class OutputStream::Proxy
{
public:
	using SizeType = OutputStream::SizeType;

private:
	OutputStream& _stream;
	mutable bool _locked;

private:
	void lock() const;
	void unlock()
	{
		_stream.mutex().unlock();
		_locked = false;
	}

public:
	Proxy(OutputStream& _stream) noexcept
		: _stream(_stream), _locked(false) {}

	~Proxy()
	{
		if (_locked)
			_stream.mutex().unlock();
	}

	bool empty() const
	{
		lock();
		return _stream.empty();
	}

	const char* data(SizeType& _size)
	{
		lock();
		return _stream.data(_size);
	}

	void pop(SizeType _size)
	{
		_stream.pop(_size);
		unlock();
	}
};

class InputStream
{
	using QueueType = std::queue<std::string>;

public:
	using SizeType = std::string::size_type;

private:
	SizeType _capacity = 0, _offset = 0;
	std::string _buffer;
	QueueType _queue;
	mutable std::mutex _mutex;

public:
	bool empty() const
	{
		std::lock_guard lock(_mutex);
		return _queue.empty();
	}

	void push(const char* _data, SizeType _size);

	inline void push(const std::string& _string)
	{
		push(_string.data(), _string.size());
	}

	bool pop(std::string& _string);

	void clear();
};
