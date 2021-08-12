#include "Stream.hpp"
#include "ByteOrder.hpp"

#include <cstdint>

const char* OutputStream::data(SizeType& length)
{
	while (buffer.size() < length && !queue.empty())
	{
		constexpr auto size = UINT32_MAX - sizeof(uint32_t);
		auto& data = queue.front();
		if (size < buffer.size() \
			|| size - buffer.size() < data.size())
			break;

		auto length = ByteOrder::hton(static_cast<uint32_t>(data.size()));
		buffer.append(reinterpret_cast<const char*>(&length), sizeof length).append(data);
		queue.pop();
	}

	length = buffer.size();
	return buffer.data();
}

void OutputStream::clear()
{
	buffer.clear();
	if (!queue.empty())
	{
		QueueType queue;
		this->queue.swap(queue);
	}
}

void InputStream::push(const char* data, SizeType length)
{
	bool completed = false;
	buffer.append(data, length);
	do
	{
		if (capacity <= 0)
		{
			constexpr auto size = sizeof(uint32_t);
			if (buffer.size() - offset < size)
				break;

			auto length = *reinterpret_cast<const uint32_t*>(buffer.data() + offset);
			capacity = ByteOrder::ntoh<uint32_t, uint32_t>(length);
			offset += size;
		}

		if (buffer.size() - offset >= capacity)
		{
			queue.emplace(buffer, offset, capacity);
			offset += capacity;
			capacity = 0;
			completed = true;
		}
	} while (capacity == 0);

	if (completed)
	{
		buffer.erase(0, offset);
		offset = 0;
	}
}

bool InputStream::pop(std::string& string)
{
	if (queue.empty())
		return false;

	string = queue.front();
	queue.pop();
	return true;
}

void InputStream::clear()
{
	capacity = offset = 0;
	buffer.clear();
	if (!queue.empty())
	{
		QueueType queue;
		this->queue.swap(queue);
	}
}
