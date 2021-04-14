/*
* 文件：State.hpp
* 
* 版本：v1.0
* 作者：许聪
* 邮箱：2592419242@qq.com
* 创建日期：2021年01月07日
* 
* 摘要：
* 1.抽象思想：主动胜于被动。自发性组织，无需管理者。
* 2.状态衍生多功能状态，多功能状态派生状态机，而状态机也是状态。
* 3.多功能状态为状态机前提，其虚函数为状态机的递推接口提供支持。
*/

#pragma once

#include <cstddef>
#include <vector>

#include "Transition.hpp"

template <typename _TransitionType, typename _MessageType>
class State
{
public:
	typedef _TransitionType TransitionType;
	typedef _MessageType MessageType;

	virtual ~State() {};

	virtual void enter() = 0;
	virtual void exit() = 0;
	virtual TransitionType handle(MessageType message) = 0;
};

template <typename _IDType, typename _MessageType, typename _SizeType = size_t>
class MultiState : public State<Transition<_IDType, MultiState<_IDType, _MessageType, _SizeType> >, _MessageType>
{
public:
	typedef _IDType IDType;
	typedef MultiState StateType;
	typedef _SizeType SizeType;
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
	MultiState(StateType* _owner = NULL)
		: _owner(_owner) {}

	void setOwner(StateType* _owner)
	{
		this->_owner = _owner;
	}

	virtual void enter() {};
	virtual void exit() {};

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
			_nodes[_cursor]._size = 1;
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
};
