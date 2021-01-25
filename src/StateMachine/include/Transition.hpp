/*
* �ļ���Transition.hpp
* 
* �汾��v1.0
* ���ߣ����
* ���䣺2592419242@qq.com
* �������ڣ�2021��01��07��
* 
* ժҪ��
* 1.�����ǳ��������������ڼ���Ϣ���ݺ�ָʾ״̬ת��������
* 2.���ɴ�����Ч�����ԡ�������ת�������������͡�
*   ��Чָ״̬ʵ����Ч�����Ա�ʾδ�ﵽת���������������ڴ�����ת��ΪĿ��״̬��ת����Ϊת����Ŀ��״̬������ζ���޺���״̬��Ҳ������ʾ���״̬����������
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
