#include "Stream.hpp"
#include "ByteOrder.hpp"

#include <cstdint>

void OutputStream::Proxy::lock() const
{
	if (!_locked)
	{
		_stream.mutex().lock();
		_locked = true;
	}
}

const char* OutputStream::data(SizeType& _size)
{
	while (_buffer.size() < _size && !_queue.empty())
	{
		constexpr auto size = UINT32_MAX - sizeof(uint32_t);
		auto& data = _queue.front();
		if (size < _buffer.size() \
			|| size - _buffer.size() < data.size())
			break;

		auto length = ByteOrder::hton(static_cast<uint32_t>(data.size()));
		_buffer.append(reinterpret_cast<const char*>(&length), sizeof length).append(data);
		_queue.pop();
	}

	_size = _buffer.size();
	return _buffer.data();
}

void OutputStream::clear()
{
	std::lock_guard lock(_mutex);
	_buffer.clear();
	if (!_queue.empty())
	{
		QueueType queue;
		_queue.swap(queue);
	}
}

void InputStream::push(const char* _data, SizeType _size)
{
	bool completed = false;

	std::lock_guard lock(_mutex);
	_buffer.append(_data, _size);

	do
	{
		if (_capacity <= 0)
		{
			constexpr auto size = sizeof(uint32_t);
			if (_buffer.size() - _offset < size)
				break;

			auto length = *reinterpret_cast<const uint32_t*>(_buffer.data() + _offset);
			_capacity = ByteOrder::ntoh<uint32_t, uint32_t>(length);
			_offset += size;
		}

		if (_buffer.size() - _offset >= _capacity)
		{
			_queue.emplace(_buffer, _offset, _capacity);
			_offset += _capacity;
			_capacity = 0;
			completed = true;
		}
	} while (_capacity == 0);

	if (completed)
	{
		_buffer.erase(0, _offset);
		_offset = 0;
	}
}

bool InputStream::pop(std::string& _string)
{
	std::lock_guard lock(_mutex);
	if (_queue.empty())
		return false;

	_string = _queue.front();
	_queue.pop();
	return true;
}

void InputStream::clear()
{
	std::lock_guard lock(_mutex);
	_capacity = _offset = 0;
	_buffer.clear();
	if (!_queue.empty())
	{
		QueueType queue;
		_queue.swap(queue);
	}
}
