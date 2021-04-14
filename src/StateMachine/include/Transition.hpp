/*
* 文件：Transition.hpp
* 
* 版本：v1.0
* 作者：许聪
* 邮箱：2592419242@qq.com
* 创建日期：2021年01月07日
* 
* 摘要：
* 1.过渡是抽象事物，负责过渡期间信息传递和指示状态转换操作。
* 2.过渡存在无效、忽略、创建、转换、空五种类型。
*   无效指状态实例无效，忽略表示未达到转换条件，创建用于创建并转换为目标状态，转换仅为转换至目标状态，空意味着无后续状态，也可以提示外层状态后续操作。
*/

#pragma once

#include <cstddef>

template <typename _IDType, typename _StateType>
class Transition
{
public:
	typedef _IDType IDType;
	typedef _StateType StateType;
	enum Type { INVALID, IGNORE, CREATEABLE, CONVERTIBLE, EMPTY };

private:
	Type _type;
	IDType _id;
	StateType* _state;

public:
	Transition(Type _type = INVALID, IDType _id = 0, StateType* _state = NULL)
		: _type(_type), _id(_id), _state(_state) {}

	void set(Type _type = INVALID, IDType _id = 0, StateType* _state = NULL)
	{
		this->_type = _type;
		this->_id = _id;
		this->_state = _state;
	}

	void setType(Type _type)
	{
		this->_type = _type;
	}

	Type getType()
	{
		return _type;
	}

	void setID(IDType _id)
	{
		this->_id = _id;
	}

	IDType getID()
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

	bool invalid()
	{
		return _type == INVALID;
	}

	bool ignore()
	{
		return _type == IGNORE;
	}

	bool createable()
	{
		return _type == CREATEABLE;
	}

	bool empty()
	{
		return _type == EMPTY;
	}
};
