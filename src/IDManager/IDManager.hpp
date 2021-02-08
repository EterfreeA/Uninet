#pragma once

#include <vector>
#include <mutex>

#include "IDContainer.hpp"

template <typename _IDType, typename _IDContainer = IDLineContainer<_IDType>>
class IDManager
{
public:
	using IDType = _IDType;
	using IDContainer = _IDContainer;

private:
	IDContainer _container; // ID容器
	IDType _extra, _digit; // 雪花算法：额外计数与占用位数（二进制）
	std::mutex _mutex; // 互斥锁

private:
	// 计算最小ID
	static IDType getBegin(IDType _decimal, IDType _binary) noexcept
	{
		// 计算指定位数的最小值
		IDType begin = 1;
		for (IDType index = 1; index < _decimal; ++index)
			begin *= 10;

		// 保证最小ID不低于指定位数
		return begin + ~(static_cast<IDType>(-1) << _binary) >> _binary;
	}

	// 计算最大ID
	static IDType getEnd(IDType _decimal, IDType _binary) noexcept
	{
		// 计算指定位数的最大值
		IDType end = 9;
		for (IDType index = 1; index < _decimal; ++index)
			end = end * 10 + 9;

		// 保证最大ID不超过指定位数
		return end - ~(static_cast<IDType>(-1) << _binary) >> _binary;
	}

public:
	/*
	 * 参数：
	 *     @_decimal	ID位数（十进制）
	 *     @_extra		雪花算法之额外计数
	 *     @_binary		额外计数占用位数（二进制）
	 */
	IDManager(IDType _decimal, IDType _extra, IDType _binary) noexcept
		: _container(getBegin(_decimal, _binary), getEnd(_decimal, _binary)), \
		_extra(_extra), _digit(_binary) {}

	// 检测ID有效性
	bool valid(IDType _id) const noexcept
	{
		return _container.valid(_id >> _digit);
	}

	// 获取ID（需要检测ID有效性）
	IDType get() noexcept
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _container.get() << _digit | _extra;
	}

	// 回收ID
	void put(IDType _id)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_container.put(_id >> _digit);
	}

	// 备份
	void backup(std::vector<IDType>& _ids)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _container.backup(_ids);
	}

	// 还原
	bool recover(const std::vector<IDType>& _ids, IDType _index = 0)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _container.recover(_ids, _index);
	}
};
