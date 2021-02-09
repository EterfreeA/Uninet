#pragma once

#include <utility>
#include <vector>
#include <set>
#include <map>

template <typename _IDType>
class IDPointContainer
{
public:
	using IDType = _IDType;

private:
	IDType _begin, _end, _present; // ��ʼ����ֹ�뵱ǰID
	std::set<IDType> _recovery; // ����ID����

public:
	IDPointContainer(IDType _begin, IDType _end) noexcept
		: _begin(_begin), _end(_end), _present(_begin) {}

	// ���ID��Ч��
	bool valid(IDType _id) const noexcept
	{
		return _id >= _begin && _id < _end;
	}

	// ��ȡID����Ҫ���ID��Ч�ԣ�
	IDType get() noexcept
	{
		// ���ȷ������ID
		if (!_recovery.empty())
		{
			auto iterator = _recovery.cbegin();
			IDType id = *iterator;
			_recovery.erase(iterator);
			return id;
		}

		// ���䵱ǰID
		IDType id = _present;
		_present = _present < _end ? _present + 1 : _end; // ��ǰID��������������ֹID
		return id;
	}

	// ����ID
	void put(IDType _id)
	{
		// 1.����ID�뵱ǰID���
		if (_id + 1 != _present)
		{
			_recovery.insert(_id);
			return;
		}

		// 2.����ID�뵱ǰID����
		_present = _id; // ��ǰID��ֵΪ����ID
		// �����������ID����
		IDType span = 1;
		auto rbegin = _recovery.crend(), rend = rbegin;
		for (auto iterator = _recovery.crbegin(); \
			iterator != _recovery.crend() && *iterator + span == _present; ++iterator, ++span)
		{
			if (span > 1)
				rend = iterator;
			else
			{
				rbegin = iterator;
				rend = iterator;
			}
		}

		// �������뵱ǰID������ID��
		if (span > 1)
		{
			auto iterator = --rend.base();
			_present = *iterator; // ��ǰID��ֵΪID�ε���СID
			_recovery.erase(iterator, rbegin.base()); // �Ƴ�ID��
		}
	}

	// ����
	void backup(std::vector<IDType>& _ids)
	{
		// Ԥ���ռ�
		_ids.reserve(_ids.size() + _recovery.size() + 1);

		// ���ݵ�ǰID
		_ids.push_back(_present);

		// ���л�����ID
		_ids.insert(_ids.cend(), _recovery.cbegin(), _recovery.cend());
	}

	// ��ԭ
	bool recover(const std::vector<IDType>& _ids, IDType _index = 0)
	{
		// ��֤���������Ϸ���
		if (_index >= _ids.size())
			return false;

		// ��֤��ǰID�Ϸ���
		IDType id = _ids[_index];
		if (id < _begin || id > _end)
			return false;

		// ��֤����ID�Ϸ���
		for (decltype(_ids.size()) index = _index + 1; index < _ids.size(); ++index)
			if (!valid(_ids[index]))
				return false;

		// ��ԭ��ǰID
		_present = id;

		// �����л�����ID
		_recovery.clear();
		_recovery.insert(_ids.cbegin() + _index + 1, _ids.cend());
		return true;
	}
};

template <typename _IDType>
class IDLineContainer
{
public:
	using IDType = _IDType;

private:
	IDType _begin, _end, _present; // ��ʼ����ֹ�뵱ǰID
	std::map<IDType, IDType> _recovery; // ����ID��������

public:
	IDLineContainer(IDType _begin, IDType _end) noexcept
		: _begin(_begin), _end(_end), _present(_begin) {}

	// ���ID��Ч��
	bool valid(IDType _id) const noexcept
	{
		return _id >= _begin && _id < _end;
	}

	// ��ȡID����Ҫ���ID��Ч�ԣ�
	IDType get() noexcept
	{
		// ���ȷ������ID
		if (!_recovery.empty())
		{
			auto iterator = _recovery.cbegin();
			IDType id = iterator->first;
			// �Ƴ�����
			if (id == iterator->second)
				_recovery.erase(iterator);
			// ǰ����С����
			else
			{
				//IDType second = iterator->second;
				//_recovery.erase(iterator);
				//_recovery.insert(std::make_pair(id + 1, second));
				const_cast<IDType&>(iterator->first) = id + 1;
			}
			return id;
		}

		// ���䵱ǰID
		IDType id = _present;
		_present = _present < _end ? _present + 1 : _end; // ��ǰID��������������ֹID
		return id;
	}

	// ����ID
	void put(IDType _id)
	{
		// 1.����ID�뵱ǰID���
		if (_id + 1 != _present)
		{
			// a.��������Ϊ��
			if (_recovery.empty())
			{
				_recovery.insert(std::make_pair(_id, _id));
				return;
			}

			auto current = _recovery.upper_bound(_id);
			// b.ǰ����ʼ����λ��
			if (current == _recovery.begin())
			{
				// ������
				if (current->first != _id + 1)
					_recovery.insert(std::make_pair(_id, _id));
				// ��������
				else
				{
					//IDType second = current->second;
					//_recovery.erase(current);
					//_recovery.insert(std::make_pair(_id, second));
					const_cast<IDType&>(current->first) = _id;
				}
				return;
			}

			// c.ǰ����ֹ����λ��
			if (current == _recovery.end())
			{
				--current;
				// ǰ����
				if (current->second + 1 != _id)
					_recovery.insert(std::make_pair(_id, _id));
				// ǰ������
				else
					current->second = _id;
				return;
			}

			// d.������֮��
			auto previous = current;
			--previous;
			bool front = previous->second == _id - 1, back = current->first == _id + 1;
			// ǰ����
			if (!front && !back)
				_recovery.insert(std::make_pair(_id, _id));
			// ǰ������
			else if (front && back)
			{
				previous->second = current->second;
				_recovery.erase(current);
			}
			// ��������
			else if (front)
				previous->second = _id;
			// ǰ������
			else
			{
				//IDType second = current->second;
				//_recovery.erase(current);
				//_recovery.insert(std::make_pair(_id, second));
				const_cast<IDType&>(current->first) = _id;
			}
			return;
		}

		// 2.����ID�뵱ǰID����
		_present = _id; // ��ǰID��ֵΪ����ID
		if (!_recovery.empty())
		{
			// ������ID�����ĺ�����ʼ�����뵱ǰID����
			auto iterator = _recovery.crbegin();
			if (iterator->second + 1 == _present)
			{
				_present = iterator->first; // ��ǰID��ֵΪ������˵�
				_recovery.erase(--iterator.base()); // �Ƴ�����
			}
		}
	}

	// ����
	void backup(std::vector<IDType>& _ids)
	{
		// Ԥ���ռ�
		_ids.reserve(_ids.size() + (_recovery.size() << 1) + 1);

		// ���ݵ�ǰID
		_ids.push_back(_present);

		// ���л�����ID
		for (auto iterator = _recovery.cbegin(); iterator != _recovery.cend(); ++iterator)
		{
			_ids.push_back(iterator->first);
			_ids.push_back(iterator->second);
		}
	}

	// ��ԭ
	bool recover(const std::vector<IDType>& _ids, IDType _index = 0)
	{
		// ��֤���������Ϸ��ԣ�ʣ������Ԫ�أ�
		if (_index >= _ids.size() || _ids.size() - _index - 1 & 1)
			return false;

		// ��֤��ǰID�Ϸ���
		IDType id = _ids[_index];
		if (id < _begin || id > _end)
			return false;

		// ��֤����ID�Ϸ���
		for (decltype(_ids.size()) index = _index + 1; index < _ids.size(); ++index)
			if (!valid(_ids[index]))
				return false;

		// ��ԭ��ǰID
		_present = id;

		// �����л�����ID
		_recovery.clear();
		for (decltype(_ids.size()) index = _index + 1; index < _ids.size(); index += 2)
			_recovery.insert(std::make_pair(_ids[index], _ids[index + 1]));
		return true;
	}
};
