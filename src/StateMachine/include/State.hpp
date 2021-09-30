/*
* 文件：State.hpp
* 
* 版本：v1.0.1
* 作者：许聪
* 邮箱：2592419242@qq.com
* 创建日期：2021年01月07日
* 更新日期：2021年09月18日
* 
* 摘要：
* 1.设计思想：状态机也是状态。
* 2.状态衍生状态机，而状态机既能够管理状态，也能够管理状态机。
* 3.状态的虚函数支撑状态机的递推接口。
* 
* 变化：
* v1.0.1
* 1.删除无用抽象层。
*/

#pragma once

#include <cstddef>
#include <vector>

#include "Transition.hpp"

template <typename _IDType, typename _MessageType, typename _SizeType = size_t>
class State
{
public:
	typedef _IDType IDType;
	typedef _MessageType MessageType;
	typedef _SizeType SizeType;
	typedef State StateType;
	typedef Transition<IDType, StateType> TransitionType;

	typedef struct Node
	{
		IDType _id;
		StateType* _state;
		SizeType _size;

		Node(IDType _id = 0, StateType* _state = NULL, SizeType _size = 0)
			: _id(_id), _state(_state), _size(_size) {}
	} NodeType;

private:
	StateType* _owner;

public:
	virtual bool _distribute(const std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		++_cursor;
		return true;
	}

	static void initCollection(std::vector<NodeType>& _nodes)
	{
		_nodes.push_back(NodeType());
	}
	virtual void _collect(std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		if (_cursor < _nodes.size())
			_nodes[_cursor]._size = 0;
	}

	virtual bool _convert(TransitionType& _transition)
	{
		return true;
	}

	virtual int _update(IDType _id, const std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		return _nodes[_cursor]._id == _id ? 0 : 1;
	}
	virtual bool _update(IDType _id)
	{
		return true;
	}

public:
	State(StateType* _owner = NULL)
		: _owner(_owner) {}

	virtual ~State() {};

	void setOwner(StateType* _owner)
	{
		this->_owner = _owner;
	}

	virtual void enter() = 0;
	virtual void exit() = 0;
	virtual TransitionType handle(MessageType message) = 0;

	bool distribute(const std::vector<NodeType>& _nodes, SizeType _cursor = 0)
	{
		return _distribute(_nodes, _cursor);
	}

	bool collect(std::vector<NodeType>& _nodes, SizeType _cursor = 0)
	{
		if (_cursor != _nodes.size())
			return false;

		initCollection(_nodes);
		_collect(_nodes, _cursor);
		return true;
	}

	virtual void backup(std::vector<IDType>& _ids) {}
	virtual bool recover(const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return true;
	}

	bool convert(TransitionType& _transition, SizeType _cursor = 0)
	{
		if (_owner == NULL)
		{
			return false;
		}

		if (_cursor <= 0)
		{
			return _owner->_convert(_transition);
		}
		else
		{
			return _owner->convert(_transition, _cursor - 1);
		}
	}

	bool update(IDType _id, const std::vector<NodeType>& _nodes, SizeType _cursor = 0)
	{
		if (_cursor >= _nodes.size())
		{
			return false;
		}
		return _update(_id, _nodes, _cursor) >= 0;
	}
	void update(IDType _id)
	{
		_update(_id);
	}

	virtual bool addState(IDType _id, StateType* _state, const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return false;
	}
	virtual StateType* removeState(IDType _id, const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return NULL;
	}
	virtual bool setState(IDType _id, const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return false;
	}
	virtual StateType* getState(IDType _id, const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return NULL;
	}
	virtual StateType* getState(const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return NULL;
	}
};
