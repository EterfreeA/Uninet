/*
* �ļ���StateMachine.hpp
* 
* �汾��v1.0
* ���ߣ����
* ���䣺2592419242@qq.com
* �������ڣ�2021��01��07��
* 
* ժҪ��
* 1.ԭ��Ϊ����״̬������������������ϲ��״̬����
* 2.֧�����л��뷴���л�������������ȱ��������л������ռ�����״̬ʵ�����ͷ�ʵ����Դ�������л����ڷַ�״̬�ڵ㣬����״̬����
* 3.֧�ֱ����뻹ԭ״̬���������������µĲ㼶˳�����л��ͷ����л���Ч״̬ID��
* 4.�ṩ��ӡ��Ƴ�������ָ��״̬�ڵ�ָ֮��״̬�ķ�����֧���������Ϸ���״̬����ʵ��ͬ����������״̬ת���������������⣬��֧�ְ��������������ת��״̬��
* 5.��Ϲ���ʵ���ʹ�����Ϣ������֧����ת��״̬֮ǰ����̬���ɲ����Ŀ��״̬��
*   �˷�������������ͬ�ڵ��״̬����ڵ�ʵ�ֿ��������½ڵ������趨ʱ����ԭ���Ͻڵ������Σ��Խ��Ͳ�ͬ�ڵ�֮�������ԡ�
*/

#pragma once

#include <utility>
#include <cstddef>
#include <vector>
#include <map>
//#include <unordered_map>
#include <iostream>

#include "State.hpp"

template <typename _IDType, typename _MessageType, typename _SizeType = size_t>
class StateMachine : public MultiState<_IDType, _MessageType, _SizeType>
{
public:
	typedef _IDType IDType;
	typedef MultiState<_IDType, _MessageType, _SizeType> StateType;
	typedef typename StateType::TransitionType TransitionType;
	typedef _MessageType MessageType;
	typedef _SizeType SizeType;
	typedef typename StateType::NodeType NodeType;
	typedef std::map<IDType, StateType*> MappingType;
	//typedef std::unordered_map<IDType, StateType*> MappingType;

private:
	bool _valid;
	IDType _id;
	MappingType _mapping;

public:
	StateMachine() : _valid(false) {}

	// �Ƿ���Ч
	bool valid()
	{
		return _valid;
	}

	// ��ȡ��ǰ״̬ID
	IDType getID()
	{
		return _id;
	}

	// ����������Ϣ
	virtual TransitionType handle(MessageType _message)
	{
		// ����ǰ״̬�ڵ�Ϊ��Ч״̬��������Ч����ʵ��
		if (!_valid)
			return TransitionType();

		// ������Ϣ����ǰ״̬����ȡ���صĹ���ʵ��
		typename MappingType::iterator iterator = _mapping.find(_id);
		if (iterator == _mapping.end())
			return TransitionType();
		TransitionType transition = iterator->second->handle(_message);

		// ������ʵ��Ϊ��Ч���ߺ������ͣ���������������ϲ�
		if (transition.invalid() || transition.ignore())
			return transition;

		// ������ʵ��Ϊ�������ͣ����ָ��״̬����ǰ״̬�ڵ�
		if (transition.createable())
			addState(transition.getID(), transition.getState());

		/* ���ݹ���ʵ��ת����ǰ״̬ */
		// ��ת��״̬֮�󣬲��ܹ��ٴ�ת�������ع���ʵ��
		if (!update(transition))
			return transition;
		// ��ת��״̬֮����Ȼ����ת���Ŀ����ԣ���ݹ����޷�ת��Ϊֹ
		return handle(_message);
	}

	/*
	 * �ַ�״̬�ڵ㣬���ڹ���״̬����
	 *
	 * ������
	 *     1.�ڵ������������л�״̬�ڵ㣬����������ȹ���״̬����
	 *     2.�α꣺������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     ����ʧ�ܷ���false������ɹ�����true��
	 *
	 * ���α���ڵ��ڽڵ���������������󣬿����ǽڵ���������˳�����
	 * ����ӽڵ�ʧ�ܣ�������ʧ�ܣ�������ͬһ״̬�������ͬID�Ľڵ㡣
	 */
	//bool distribute(const std::vector<NodeType>& _nodes, SizeType _cursor = 0);

	/*
	 * �ռ�״̬�ڵ㣬�����ͷ���Դ��
	 *
	 * ������
	 *     1.�ڵ����������л�״̬�ڵ㣬����������ȱ���״̬����
	 *     2.�α꣺������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     �ռ�ʧ�ܷ���false���ռ��ɹ�����true��
	 *
	 * ���α겻���ڽڵ����������α�λ�������ռ�ʧ�ܡ�
	 */
	//bool collect(std::vector<NodeType>& _nodes, SizeType _cursor = 0);

	/*
	 * ����״̬������Ч״̬
	 * �����������µĲ㼶˳����ջ��Ч״̬��ֱ��������Ч״̬�������²�ڵ�Ϊֹ��
	 *
	 * ������
	 *     1.״̬����ջ�����л���Ч״̬��
	 */
	virtual void backup(std::vector<IDType>& _ids)
	{
		if (!_valid)
			return;

		_ids.push_back(_id);
		typename MappingType::iterator iterator = _mapping.find(_id);
		if (iterator != _mapping.end())
			iterator->second->backup(_ids);
	}

	/*
	 * ��ԭ״̬����ָ��״̬
	 * �����������µĲ㼶˳��ԭ״̬����ֱ������״̬��ɣ������������²�ڵ�Ϊֹ��
	 *
	 * ������
	 *     1.״̬����ջ�������л���Ч״̬��
	 *     2.�㼶��������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     ��ԭʧ�ܷ���false����ԭ�ɹ�����true��
	 *
	 * ��δ�ҵ�ָ��ID�Ľڵ㣬��ԭʧ�ܣ�������״̬����ջ���߳�ʼ�㼶����Ҳ�����Ǵ��ڶ�̬�ڵ㣬��δ��ǰ���ɽڵ㡣
	 */
	virtual bool recover(const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor >= _ids.size())
			return true;

		IDType id = _ids[_cursor];
		typename MappingType::iterator iterator = _mapping.find(id);
		if (iterator == _mapping.end())
			return false;

		this->_id = id;
		_valid = true;
		return iterator->second->recover(_ids, _cursor + 1);
	}

	/*
	 * ת��ָ��״̬�ڵ��״̬
	 * 
	 * ������
	 *     1.����ʵ����ָʾ״̬�ڵ��ת��������
	 *     2.�㼶�����Ͽ�Խ��״̬�ڵ�������Ĭ��Ϊ�㣬����ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     ת��ʧ�ܷ���false��ת���ɹ�����true��
	 * 
	 * ���������κ�״̬�ڵ㣬����״̬�ڵ�Ϊ��Ч״̬����ת��ʧ�ܡ�
	 * ������ʵ��Ϊ��Ч���ߺ������ͣ���ת��ʧ�ܡ�
	 */
	//bool convert(TransitionType& _transition, SizeType _cursor = 0);

	/*
	 * ��������ת��״̬��ָ��״̬
	 * ������Ҫ����ǰ����������״̬�������ϳ����Էֲ���
	 * �ӵ�ǰ״̬������ת��״̬��ֱ��Ŀ��״̬Ϊֹ����Ŀ��״̬�����ڣ���ת�������״̬����һ״̬��
	 * 
	 * ������
	 *     1.״̬ID������Ŀ��״̬��
	 *     2.�ڵ�������ָ��״̬�ڵ����У�����������ȱ���״̬����
	 *     3.�α꣺������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     ת��ʧ�ܷ���false��ת���ɹ�����true��
	 * 
	 * ���α���ڵ��ڽڵ���������������󣬿����ǽڵ���������˳�����
	 * ��δ�ҵ�����״̬�ڵ㣬��ת��ʧ�ܣ������ǽڵ���������˳�����
	 */
	bool update(IDType _id, const std::vector<NodeType>& _nodes, SizeType _cursor = 0)
	{
		return StateType::update(_id, _nodes, _cursor);
	}

	/*
	 * ����˳��ת��״̬��ָ��״̬
	 * ������Ҫ��������ǰ����������һ��״̬�������ϳ����Էֲ����������ͬ�ڵ�Ĳ�ͬ״̬����ID��С�����˳�����С�
	 * ��Ŀ��״̬�ǵ�ǰ״̬�����Ѿ���״̬���򲻻�ת��״̬����������ת��״ֱ̬��Ŀ��״̬��
	 * ��Ŀ��״̬�����ڣ���ת�������״̬����һ״̬��
	 * 
	 * ������
	 *     1.״̬ID������Ŀ��״̬��
	 */
	void update(IDType _id)
	{
		StateType::update(_id);
	}

	/*
	 * ���״̬����ǰ״̬�ڵ�
	 * 
	 * ������
	 *     1.״̬ID�����������״̬��
	 *     2.״ָ̬�룺ָ���Ѵ�����״̬ʵ����
	 * ����ֵ��
	 *     ���ʧ�ܷ���false����ӳɹ�����true��
	 * 
	 * ��״ָ̬��Ϊ�գ����ߴ�����ͬ״̬ID�������ʧ�ܡ�
	 */
	bool addState(IDType _id, StateType* _state)
	{
		if (_state == NULL)
			return false;

		if (_mapping.find(_id) != _mapping.end())
			return false;
		if (!_mapping.insert(std::make_pair(_id, _state)).second)
			return false;

		_state->setOwner(this);
		return true;
	}

	/*
	 * ���״̬������״̬�ڵ�
	 * �Ե�ǰ״̬�ڵ�Ϊ�������±���������״̬�ڵ㣬���Ҳ�������״̬��
	 * 
	 * ������
	 *     1.״̬ID�����������״̬��
	 *     2.״ָ̬�룺ָ���Ѵ�����״̬ʵ����
	 *     3.״̬ID���������մ������µĲ㼶˳������״̬�ڵ㡣
	 *     4.�㼶��������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     ���ʧ�ܷ���false����ӳɹ�����true��
	 * 
	 * ���㼶����״̬ID�����������ʧ�ܣ�������״̬ID�������߳�ʼ�㼶����
	 * ��δ�ҵ�����״̬�ڵ㣬�����ʧ�ܡ�
	 */
	virtual bool addState(IDType _id, StateType* _state, const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size())
			return false;
		if (_cursor == _ids.size())
			return addState(_id, _state);

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end())
			return false;
		return iterator->second->addState(_id, _state, _ids, _cursor + 1);
	}

	/*
	 * �Ƴ���ǰ״̬�ڵ��ָ��״̬
	 * 
	 * ������
	 *     1.״̬ID���������Ƴ�״̬��
	 * ����ֵ��
	 *     �Ƴ�ʧ�ܷ���NULL���Ƴ��ɹ������Ƴ���״ָ̬�롣
	 * 
	 * ����ǰ״̬�ڵ���Ч�����Ҵ��Ƴ�״̬�ǵ�ǰ״̬������δ�ҵ����Ƴ�״̬�����Ƴ�ʧ�ܡ�
	 */
	StateType* removeState(IDType _id)
	{
		if (_valid && this->_id == _id)
			return NULL;

		typename MappingType::iterator iterator = _mapping.find(_id);
		if (iterator == _mapping.end())
			return NULL;

		StateType* state = iterator->second;
		_mapping.erase(iterator);
		return state;
	}

	/*
	 * �Ƴ�ָ��״̬�ڵ��ָ��״̬
	 * �Ե�ǰ״̬�ڵ�Ϊ�������±���������״̬�ڵ㣬�����Ƴ�Ŀ��״̬��
	 * 
	 * ������
	 *     1.״̬ID���������Ƴ�״̬��
	 *     2.״̬ID���������մ������µĲ㼶˳������״̬�ڵ㡣
	 *     3.�㼶��������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     �Ƴ�ʧ�ܷ���NULL���Ƴ��ɹ������Ƴ���״ָ̬�롣
	 * 
	 * ���㼶����״̬ID���������Ƴ�ʧ�ܣ�������״̬ID�������߳�ʼ�㼶����
	 * ��δ�ҵ�ָ��״̬�ڵ㣬���Ƴ�ʧ�ܡ�
	 */
	virtual StateType* removeState(IDType _id, const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size())
			return NULL;
		if (_cursor == _ids.size())
			return removeState(_id);

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end())
			return NULL;
		return iterator->second->removeState(_id, _ids, _cursor + 1);
	}

	/*
	 * ���õ�ǰ״̬�ڵ�ĵ�ǰ״̬
	 * 
	 * ������
	 *     1.״̬ID������Ŀ��״̬��
	 * ����ֵ��
	 *     ����ʧ�ܷ���false�����óɹ�����true��
	 * 
	 * ����ǰ״̬�ڵ�Ϊ��Ч״̬������δ�ҵ�Ŀ��״̬��������ʧ�ܡ�
	 */
	bool setState(IDType _id)
	{
		if (_valid)
			return false;

		typename MappingType::iterator next = _mapping.find(_id);
		if (next == _mapping.end())
			return false;

		this->_id = _id;
		_valid = true;
		next->second->enter();
		return true;
	}

	/*
	 * ����ָ��״̬�ڵ�ĵ�ǰ״̬
	 * �Ե�ǰ״̬�ڵ�Ϊ�������±���������״̬�ڵ㣬�������õ�ǰ״̬ΪĿ��״̬��
	 * 
	 * ������
	 *     1.״̬ID������Ŀ��״̬��
	 *     2.״̬ID���������մ������µĲ㼶˳������״̬�ڵ㡣
	 *     3.�㼶��������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     ����ʧ�ܷ���false�����óɹ�����true��
	 * 
	 * ���㼶����״̬ID������������ʧ�ܣ�������״̬ID�������߳�ʼ�㼶����
	 * ��δ�ҵ�ָ��״̬�ڵ㣬������ʧ�ܡ�
	 */
	virtual bool setState(IDType _id, const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size())
			return false;
		if (_cursor == _ids.size())
			return setState(_id);

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end())
			return NULL;
		return iterator->second->setState(_id, _ids, _cursor + 1);
	}

	/*
	 * ��ȡ��ǰ״̬�ڵ��ָ��״̬
	 * 
	 * ������
	 *     1.״̬ID������Ŀ��״̬��
	 * ����ֵ��
	 *     ��ȡʧ�ܷ���NULL����ȡ�ɹ�����Ŀ��״ָ̬�롣
	 * 
	 * ����ǰ״̬�ڵ�Ϊ��Ч״̬������δ�ҵ�Ŀ��״̬�����ȡʧ�ܡ�
	 */
	StateType* getState(IDType _id)
	{
		if (!_valid)
			return NULL;

		typename MappingType::iterator iterator = _mapping.find(_id);
		if (iterator == _mapping.end())
			return NULL;
		return iterator->second;
	}

	/*
	 * ��ȡָ��״̬�ڵ��ָ��״̬
	 * �Ե�ǰ״̬�ڵ�Ϊ�������±���������״̬�ڵ㣬���һ�ȡĿ��״̬��
	 * 
	 * ������
	 *     1.״̬ID������Ŀ��״̬��
	 *     2.״̬ID���������մ������µĲ㼶˳������״̬�ڵ㡣
	 *     3.�㼶��������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     ��ȡʧ�ܷ���NULL����ȡ�ɹ�����Ŀ��״ָ̬�롣
	 * 
	 * ���㼶����״̬ID���������ȡʧ�ܣ�������״̬ID�������߳�ʼ�㼶����
	 * ��δ�ҵ�ָ��״̬�ڵ㣬���ȡʧ�ܡ�
	 */
	virtual StateType* getState(IDType _id, const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size())
			return NULL;
		if (_cursor == _ids.size())
			return getState(_id);

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end())
			return NULL;
		return iterator->second->getState(_id, _ids, _cursor + 1);
	}

	/*
	 * ��ȡ��ǰ״̬�ڵ�ĵ�ǰ״̬
	 * 
	 * ����ֵ��
	 *     ��ȡʧ�ܷ���NULL����ȡ�ɹ�����Ŀ��״ָ̬�롣
	 * 
	 * ����ǰ״̬�ڵ�Ϊ��Ч״̬�����ȡʧ�ܡ�
	 */
	StateType* getState()
	{
		return getState(_id);
	}

	/*
	 * ��ȡָ��״̬�ڵ�ĵ�ǰ״̬
	 * �Ե�ǰ״̬�ڵ�Ϊ�������±���������״̬�ڵ㣬���һ�ȡ��ǰ״̬��
	 * 
	 * ������
	 *     1.״̬ID���������մ������µĲ㼶˳������״̬�ڵ㡣
	 *     2.�㼶��������ǰ״̬�ڵ㡣
	 * ����ֵ��
	 *     ��ȡʧ�ܷ���NULL����ȡ�ɹ�����Ŀ��״ָ̬�롣
	 * 
	 * ���㼶����״̬ID���������ȡʧ�ܣ�������״̬ID�������߳�ʼ�㼶����
	 */
	virtual StateType* getState(const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size())
			return NULL;
		if (_cursor == _ids.size())
			return getState();

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end())
			return NULL;
		return iterator->second->getState(_ids, _cursor + 1);
	}

private:
	/*
	 * ���ݹ���ʵ��ת��״̬
	 * 
	 * ������
	 *     1.����ʵ����ָʾ״̬�ڵ��ת��������
	 * ����ֵ��
	 *     ��ת��״̬֮�󣬲��ܹ��ٴ�ת�����򷵻�false��
	 *     ��ת��״̬֮����Ȼ����ת���Ŀ����ԣ��򷵻�true��
	 */
	bool update(TransitionType& _transition)
	{
		// ���������˳���ǰ״̬
		typename MappingType::iterator previous = _mapping.find(_id);
		if (previous != _mapping.end())
			previous->second->exit();

		// ������ʵ��Ϊ�����ͣ�˵����ǰ״̬�ڵ��޺���״̬������Ϊ��Ч״̬
		if (_transition.empty())
			return _valid = false;

		// ��Ŀ��״̬�����ڣ�˵��״̬�ڵ㷢���쳣��������Ŀ��״̬���󣬻���δ��ǰ���Ŀ��״̬
		typename MappingType::iterator next = _mapping.find(_transition.getID());
		if (next == _mapping.end())
		{
			std::cerr << "bool update(TransitionType&): switch to an invalid state." << std::endl;
			// ����ʵ����Ϊ��Ч����
			_transition.setType(TransitionType::INVALID);
			return false;
		}

		// ��ǰ״̬�ڵ�ĵ�ǰ״̬��ΪĿ��״̬
		_id = _transition.getID();
		// ����Ŀ��״̬
		next->second->enter();
		return true;
	}

	virtual bool _distribute(const std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		if (_cursor >= _nodes.size())
			return false;

		const NodeType& node = _nodes[_cursor];
		if (_cursor++ + node._size >= _nodes.size())
			return false;

		for (SizeType index = 0; index < node._size; ++index)
		{
			const NodeType& node = _nodes[_cursor];
			if (!addState(node._id, node._state))
				return false;

			if (!node._state->_distribute(_nodes, _cursor))
				return false;
		}
		return true;
	}

	/*
	 * �ռ�״̬�ڵ�
	 *
	 * ������
	 *     1.�ڵ����������л�״̬�ڵ㣬����������ȱ���״̬������ʼ��Ҫ���һ���սڵ㡣
	 *     2.�α꣺������ǰ״̬�ڵ㣬��ʼ��Ҫָ����ӵĿսڵ㡣
	 *
	 * ���ڽڵ������ĳ�ʼ�������Ե��þ�̬����MultiState::initCollection�������ֶ���ӿսڵ㡣
	 */
	virtual void _collect(std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		NodeType& node = _nodes[_cursor];
		node._size = _mapping.size();
		for (typename MappingType::iterator iterator = _mapping.begin(); iterator != _mapping.end(); ++iterator)
		{
			_nodes.push_back(NodeType(iterator->first, iterator->second));
			iterator->second->_collect(_nodes, ++_cursor);
		}
	}

	virtual bool _convert(TransitionType& _transition)
	{
		// ����ǰ״̬�ڵ�Ϊ��Ч״̬��������Ч����ʵ��
		if (!_valid)
			return false;

		// ������ʵ��Ϊ��Ч���ߺ������ͣ��������ֱ�ӷ���
		if (_transition.invalid() || _transition.ignore())
			return false;

		// ������ʵ��Ϊ�������ͣ����ָ��״̬����ǰ״̬�ڵ�
		if (_transition.createable())
			addState(_transition.getID(), _transition.getState());

		// ���ݹ���ʵ��ת����ǰ״̬
		return update(_transition);
	}

	virtual int _update(IDType _id, const std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		const NodeType& node = _nodes[_cursor];
		if (_cursor != 0 && node._id == _id)
			return 0;

		for (SizeType index = 0, number = node._size; index < number; ++index)
		{
			if (++_cursor >= _nodes.size())
				return -1;

			const NodeType& node = _nodes[_cursor];
			typename MappingType::iterator iterator = _mapping.find(node._id);
			if (iterator == _mapping.end())
				return -1;

			int result = iterator->second->_update(_id, _nodes, _cursor);
			if (result <= 0)
				return result;

			if (_valid && node._id == this->_id)
			{
				TransitionType transition(TransitionType::EMPTY);
				if (index + 1 < number)
				{
					SizeType index = _cursor + 1;
					if (index >= _nodes.size())
						return -1;

					const NodeType& node = _nodes[index];
					transition.set(TransitionType::CONVERTIBLE, node._id);
				}
				update(transition);
			}
		}
		return 1;
	}

	virtual bool _update(IDType _id)
	{
		for (typename MappingType::iterator iterator = _mapping.begin(); iterator != _mapping.end(); ++iterator)
		{
			if (iterator->first == _id)
				return false;
			if (!iterator->second->_update(_id))
				return false;

			if (_valid && iterator->first == this->_id)
			{
				TransitionType transition(TransitionType::EMPTY);
				typename MappingType::iterator next = iterator;
				if (++next != _mapping.end())
					transition.set(TransitionType::CONVERTIBLE, next->first);
				update(transition);
			}
		}
		return true;
	}
};
