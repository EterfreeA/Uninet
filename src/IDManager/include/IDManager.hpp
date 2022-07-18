#pragma once

#include <concepts>

#include "IDContainer.hpp"

// 管理器配接容器
template <std::unsigned_integral _IDType, \
	typename _IDContainer = IDLineContainer<_IDType>>
class IDManager
{
public:
	using IDType = _IDType;
	using IDContainer = _IDContainer;

	using VectorType = IDContainer::VectorType;
	using SizeType = IDContainer::SizeType;

private:
	IDContainer _container; // ID容器
	IDType _extra, _digit; // 雪花算法：额外计数与占用位数（二进制）

private:
	// 计算最小ID
	static auto getBegin(IDType _decimal, IDType _binary) noexcept
	{
		// 计算指定位数的最小值
		decltype(_decimal) begin = 1;
		for (decltype(_decimal) index = 1; \
			index < _decimal; ++index, begin *= 10);

		// 保证最小ID不低于指定位数
		auto offset = ~(static_cast<decltype(begin)>(-1) << _binary);
		return begin + offset >> _binary;
	}

	// 计算最大ID
	static auto getEnd(IDType _decimal, IDType _binary) noexcept
	{
		// 计算指定位数的最大值
		decltype(_decimal) end = 9;
		for (decltype(_decimal) index = 1; \
			index < _decimal; ++index, end = end * 10 + 9);

		// 保证最大ID不超过指定位数
		auto offset = ~(static_cast<decltype(end)>(-1) << _binary);
		return (end - offset >> _binary) + 1;
	}

public:
	/*
	 * 参数：
	 *     @_decimal	ID位数（十进制）
	 *     @_extra		雪花算法之额外计数
	 *     @_binary		额外计数占用位数（二进制）
	 */
	IDManager(IDType _decimal, IDType _extra, IDType _binary)
		: _container(getBegin(_decimal, _binary), getEnd(_decimal, _binary)), \
		_extra(_extra), _digit(_binary) {}

	// 容量
	auto size() const noexcept
	{
		return _container.size();
	}

	// 检测ID有效性
	bool valid(IDType _id) const
	{
		return _container.valid(_id >> _digit);
	}

	// 获取ID（需要检测ID有效性）
	auto get() noexcept
	{
		return _container.get() << _digit | _extra;
	}

	// 回收ID
	void put(IDType _id)
	{
		_container.put(_id >> _digit);
	}

	// 备份
	void backup(VectorType& _vector) const
	{
		_container.backup(_vector);
	}

	// 还原
	bool recover(const VectorType& _vector, SizeType _index = 0)
	{
		return _container.recover(_vector, _index);
	}
};
