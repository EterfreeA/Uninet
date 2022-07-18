#pragma once

#include <utility>
#include <vector>
#include <set>
#include <map>

// 以点形式存放回收ID
template <typename _IDType>
class IDPointContainer
{
public:
	using IDType = _IDType;
	using VectorType = std::vector<IDType>;
	using SizeType = VectorType::size_type;

private:
	using SetType = std::set<IDType>;

private:
	IDType _begin, _end, _present; // 起始、终止与当前ID
	SetType _recovery; // 回收ID集合

public:
	IDPointContainer(IDType _begin, IDType _end)
		: _begin(_begin), _end(_end), _present(_begin) {}

	// 容量
	auto size() const noexcept
	{
		return _end - _begin;
	}

	// 检测ID有效性
	bool valid(IDType _id) const
	{
		return _id >= _begin && _id < _present \
			&& !_recovery.contains(_id);
	}

	// 获取ID（需要检测ID有效性）
	auto get() noexcept
	{
		// 优先分配回收ID
		if (!_recovery.empty())
		{
			auto iterator = _recovery.cbegin();
			auto id = *iterator;
			_recovery.erase(iterator);
			return id;
		}

		// 分配当前ID
		auto id = _present;
		// 当前ID自增并不超过终止ID
		_present = _present < _end ? \
			_present + 1 : _end;
		return id;
	}

	// 回收ID
	void put(IDType _id)
	{
		// 1.回收ID与当前ID间断
		if (_id + 1 != _present)
		{
			_recovery.insert(_id);
			return;
		}

		// 2.回收ID与当前ID连续
		_present = _id; // 当前ID赋值为回收ID

		// 后向遍历回收ID集合
		decltype(_present) span = 1;
		auto rbegin = _recovery.crend(), rend = rbegin;
		if (auto iterator = _recovery.crbegin(); \
			iterator != rend && *iterator + span == _present)
			for (rbegin = rend = iterator++, ++span; \
				iterator != _recovery.crend() \
				&& *iterator + span == _present; \
				rend = iterator++, ++span);

		// 若存在与当前ID连续的ID段
		if (span != 1)
		{
			auto iterator = --rend.base();
			_present = *iterator; // 当前ID赋值为ID段的最小ID
			_recovery.erase(iterator, rbegin.base()); // 移除ID段
		}
	}

	// 备份
	void backup(VectorType& _vector) const
	{
		// 预留空间
		_vector.reserve(_vector.size() + _recovery.size() + 1);

		// 备份当前ID
		_vector.push_back(_present);

		// 序列化回收ID
		_vector.insert(_vector.cend(), \
			_recovery.cbegin(), _recovery.cend());
	}

	// 还原
	bool recover(const VectorType& _vector, SizeType _index = 0)
	{
		// 验证向量索引合法性
		if (_index >= _vector.size()) return false;

		// 验证当前ID合法性
		auto id = _vector[_index];
		if (id < _begin || id > _end) return false;

		// 备份回收ID集合
		decltype(_recovery) recovery;
		recovery.swap(_recovery);

		// 验证回收ID合法性
		auto offset = _index + 1;
		for (auto index = offset; index < _vector.size(); ++index)
			if (!valid(_vector[index]))
			{
				// 还原回收ID集合
				_recovery = std::move(recovery);
				return false;
			}

		// 还原当前ID
		_present = id;

		// 反序列化回收ID
		_recovery.insert(_vector.cbegin() + offset, _vector.cend());
		return true;
	}
};

// 以线段（区间）形式存放回收ID
template <typename _IDType>
class IDLineContainer
{
public:
	using IDType = _IDType;
	using VectorType = std::vector<IDType>;
	using SizeType = VectorType::size_type;

private:
	using MapType = std::map<IDType, IDType>;

private:
	IDType _begin, _end, _present; // 起始、终止与当前ID
	MapType _recovery; // 回收ID区间容器

private:
	bool contain(IDType _id) const
	{
		auto current = _recovery.upper_bound(_id);
		if (current == _recovery.begin())
			return false;

		--current;
		return _id >= current->first \
			&& _id <= current->second;
	}

public:
	IDLineContainer(IDType _begin, IDType _end)
		: _begin(_begin), _end(_end), _present(_begin) {}

	// 容量
	auto size() const noexcept
	{
		return _end - _begin;
	}

	// 检测ID有效性
	bool valid(IDType _id) const
	{
		return _id >= _begin && _id < _present \
			&& !contain(_id);
	}

	// 获取ID（需要检测ID有效性）
	auto get() noexcept
	{
		// 优先分配回收ID
		if (!_recovery.empty())
		{
			auto iterator = _recovery.cbegin();
			auto id = iterator->first;

			// 移除单点
			if (id == iterator->second)
				_recovery.erase(iterator);
			// 前向缩小区间
			else
			{
				//auto second = iterator->second;
				//_recovery.erase(iterator);
				//_recovery.emplace(id + 1, second);

				const_cast<IDType&>(iterator->first) = id + 1;
			}
			return id;
		}

		// 分配当前ID
		auto id = _present;
		// 当前ID自增并不超过终止ID
		_present = _present < _end ? _present + 1 : _end;
		return id;
	}

	// 回收ID
	void put(IDType _id)
	{
		// 1.回收ID与当前ID间断
		if (_id + 1 != _present)
		{
			// a.区间容器为空
			if (_recovery.empty())
			{
				_recovery.emplace(_id, _id);
				return;
			}

			auto current = _recovery.upper_bound(_id);

			// b.前向起始区间位置
			if (current == _recovery.begin())
			{
				// 后向间断
				if (current->first != _id + 1)
					_recovery.emplace(_id, _id);
				// 后向连续
				else
				{
					//auto second = current->second;
					//_recovery.erase(current);
					//_recovery.emplace(_id, second);

					const_cast<IDType&>(current->first) = _id;
				}
				return;
			}

			// c.前向终止区间位置
			if (current == _recovery.end())
			{
				--current;

				// 前向间断
				if (current->second + 1 != _id)
					_recovery.emplace(_id, _id);
				// 前向连续
				else
					current->second = _id;
				return;
			}

			// d.两区间之间
			auto previous = current;
			--previous;
			bool front = previous->second == _id - 1;
			bool back = current->first == _id + 1;

			if (front)
			{
				// 前后连续
				if (back)
				{
					previous->second = current->second;
					_recovery.erase(current);
				}
				// 后向连续
				else
					previous->second = _id;
			}
			else
			{
				// 前向连续
				if (back)
				{
					//auto second = current->second;
					//_recovery.erase(current);
					//_recovery.emplace(_id, second);

					const_cast<IDType&>(current->first) = _id;
				}
				// 前后间断
				else
					_recovery.emplace(_id, _id);
			}
			return;
		}

		// 2.回收ID与当前ID连续
		_present = _id; // 当前ID赋值为回收ID
		if (!_recovery.empty())
			// 若回收ID容器的后向起始区间与当前ID连续
			if (auto iterator = _recovery.crbegin(); \
				iterator->second + 1 == _present)
			{
				_present = iterator->first; // 当前ID赋值为区间左端点
				_recovery.erase(--iterator.base()); // 移除区间
			}
	}

	// 备份
	void backup(VectorType& _vector) const
	{
		// 预留空间
		_vector.reserve(_vector.size() + _recovery.size() * 2 + 1);

		// 备份当前ID
		_vector.push_back(_present);

		// 序列化回收ID
		for (auto iterator = _recovery.cbegin(); \
			iterator != _recovery.cend(); ++iterator)
		{
			_vector.push_back(iterator->first);
			_vector.push_back(iterator->second);
		}
	}

	// 还原
	bool recover(const VectorType& _vector, SizeType _index = 0)
	{
		// 验证向量索引合法性（剩余奇数元素）
		if (_index >= _vector.size() \
			|| (_vector.size() - _index - 1) % 2)
			return false;

		// 验证当前ID合法性
		auto id = _vector[_index];
		if (id < _begin || id > _end) return false;

		// 备份回收ID区间容器
		decltype(_recovery) recovery;
		recovery.swap(_recovery);

		// 验证回收ID合法性
		auto offset = _index + 1;
		for (auto index = offset; index < _vector.size(); ++index)
			if (!valid(_vector[index]))
			{
				// 还原回收ID区间容器
				_recovery = std::move(recovery);
				return false;
			}

		// 还原当前ID
		_present = id;

		// 反序列化回收ID
		for (auto index = offset; index < _vector.size(); index += 2)
			_recovery.emplace(_vector[index], _vector[index + 1]);
		return true;
	}
};
