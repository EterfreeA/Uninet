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
	IDContainer _container; // ID����
	IDType _extra, _digit; // ѩ���㷨�����������ռ��λ���������ƣ�
	std::mutex _mutex; // ������

private:
	// ������СID
	static IDType getBegin(IDType _decimal, IDType _binary) noexcept
	{
		// ����ָ��λ������Сֵ
		IDType begin = 1;
		for (IDType index = 1; index < _decimal; ++index)
			begin *= 10;

		// ��֤��СID������ָ��λ��
		return begin + ~(static_cast<IDType>(-1) << _binary) >> _binary;
	}

	// �������ID
	static IDType getEnd(IDType _decimal, IDType _binary) noexcept
	{
		// ����ָ��λ�������ֵ
		IDType end = 9;
		for (IDType index = 1; index < _decimal; ++index)
			end = end * 10 + 9;

		// ��֤���ID������ָ��λ��
		return end - ~(static_cast<IDType>(-1) << _binary) >> _binary;
	}

public:
	/*
	 * ������
	 *     @_decimal	IDλ����ʮ���ƣ�
	 *     @_extra		ѩ���㷨֮�������
	 *     @_binary		�������ռ��λ���������ƣ�
	 */
	IDManager(IDType _decimal, IDType _extra, IDType _binary) noexcept
		: _container(getBegin(_decimal, _binary), getEnd(_decimal, _binary)), \
		_extra(_extra), _digit(_binary) {}

	// ���ID��Ч��
	bool valid(IDType _id) const noexcept
	{
		return _container.valid(_id >> _digit);
	}

	// ��ȡID����Ҫ���ID��Ч�ԣ�
	IDType get() noexcept
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _container.get() << _digit | _extra;
	}

	// ����ID
	void put(IDType _id)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_container.put(_id >> _digit);
	}

	// ����
	void backup(std::vector<IDType>& _ids)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _container.backup(_ids);
	}

	// ��ԭ
	bool recover(const std::vector<IDType>& _ids, IDType _index = 0)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _container.recover(_ids, _index);
	}
};
