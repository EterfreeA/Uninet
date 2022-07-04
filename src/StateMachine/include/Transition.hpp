#pragma once

#include <cstddef>

template <typename _IDType, typename _StateType>
class Transition
{
public:
	/*
	* 过渡存在无效、忽略、创建、转换、空五种类型。
	* 1.无效指状态实例无效；
	* 2.忽略表示未达到转换条件；
	* 3.创建用于创建并转换为目标状态；
	* 4.转换仅为转换至目标状态；
	* 5.空意味着无后续状态，也可以提示后续操作。
	*/
	enum Type { INVALID, IGNORE, CREATEABLE, CONVERTIBLE, EMPTY };

public:
	typedef _IDType IDType;
	typedef _StateType StateType;

private:
	Type _type;
	IDType _id;
	StateType* _state;

public:
	Transition(Type _type = INVALID, \
		IDType _id = static_cast<IDType>(0), StateType* _state = NULL)
		: _type(_type), _id(_id), _state(_state) {}

	void set(Type _type = INVALID, \
		IDType _id = static_cast<IDType>(0), \
		StateType* _state = NULL)
	{
		this->_type = _type;
		this->_id = _id;
		this->_state = _state;
	}

	void setType(Type _type)
	{
		this->_type = _type;
	}

	Type getType() const
	{
		return _type;
	}

	void setID(IDType _id)
	{
		this->_id = _id;
	}

	IDType getID() const
	{
		return _id;
	}

	void setState(StateType* _state)
	{
		this->_state = _state;
	}

	StateType* getState()
	{
		return _state;
	}

	bool invalid() const
	{
		return _type == INVALID;
	}

	bool ignore() const
	{
		return _type == IGNORE;
	}

	bool createable() const
	{
		return _type == CREATEABLE;
	}

	bool empty() const
	{
		return _type == EMPTY;
	}
};
