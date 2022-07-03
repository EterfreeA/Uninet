#pragma once

#include <cstddef>
#include <vector>

#include "Transition.hpp"

template <typename _IDType, typename _MessageType, \
	typename _SizeType = std::size_t>
class State
{
public:
	typedef _IDType IDType;
	typedef _MessageType MessageType;
	typedef _SizeType SizeType;

	typedef State StateType;
	typedef Transition<IDType, StateType> TransitionType;

public:
	typedef struct Node
	{
		IDType _id;
		StateType* _state;
		SizeType _size;

		Node(IDType _id = static_cast<IDType>(0), \
			StateType* _state = NULL, SizeType _size = 0)
			: _id(_id), _state(_state), _size(_size) {}
	} NodeType;

private:
	StateType* _owner;

public:
	static void initCollection(std::vector<NodeType>& _nodes)
	{
		_nodes.push_back(NodeType());
	}

public:
	virtual bool _distribute(const std::vector<NodeType>& _nodes, \
		SizeType& _cursor)
	{
		++_cursor;
		return true;
	}

	virtual void _collect(std::vector<NodeType>& _nodes, \
		SizeType& _cursor)
	{
		if (_cursor < _nodes.size())
			_nodes[_cursor]._size = 0;
	}

	virtual bool _convert(TransitionType& _transition)
	{
		return true;
	}

	virtual int _update(IDType _id, \
		const std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		return _nodes[_cursor]._id == _id ? 0 : 1;
	}

	virtual bool _update(IDType _id)
	{
		return true;
	}

public:
	virtual void enter() = 0;
	virtual void exit() = 0;
	virtual TransitionType handle(MessageType message) = 0;

	virtual void backup(std::vector<IDType>& _ids) const {}
	virtual bool recover(const std::vector<IDType>& _ids, \
		SizeType _cursor = 0)
	{
		return true;
	}

public:
	virtual bool addState(IDType _id, StateType* _state, \
		const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return false;
	}

	virtual StateType* removeState(IDType _id, \
		const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return NULL;
	}

	virtual bool setState(IDType _id, \
		const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return false;
	}

	virtual StateType* getState(IDType _id, \
		const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		return NULL;
	}

	virtual StateType* getState(const std::vector<IDType>& _ids, \
		SizeType _cursor = 0)
	{
		return NULL;
	}

public:
	State(StateType* _owner = NULL)
		: _owner(_owner) {}

	virtual ~State() {}

	void setOwner(StateType* _owner)
	{
		this->_owner = _owner;
	}

	bool distribute(const std::vector<NodeType>& _nodes, \
		SizeType _cursor = 0)
	{
		return _distribute(_nodes, _cursor);
	}

	bool collect(std::vector<NodeType>& _nodes, \
		SizeType _cursor = 0)
	{
		if (_cursor != _nodes.size())
			return false;

		initCollection(_nodes);
		_collect(_nodes, _cursor);
		return true;
	}

	bool convert(TransitionType& _transition, SizeType _cursor = 0)
	{
		if (_owner == NULL) return false;

		if (_cursor <= 0)
			return _owner->_convert(_transition);
		else
			return _owner->convert(_transition, _cursor - 1);
	}

	bool update(IDType _id, const std::vector<NodeType>& _nodes, \
		SizeType _cursor = 0)
	{
		if (_cursor >= _nodes.size()) return false;
		return _update(_id, _nodes, _cursor) >= 0;
	}

	void update(IDType _id)
	{
		_update(_id);
	}
};
