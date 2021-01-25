/*
* 文件：StateMachine.hpp
* 
* 版本：v1.0
* 作者：许聪
* 邮箱：2592419242@qq.com
* 创建日期：2021年01月07日
* 
* 摘要：
* 1.原型为有限状态机，不过可以自由组合层次状态机。
* 2.支持序列化与反序列化，采用深度优先遍历。序列化用于收集所有状态实例，释放实例资源，反序列化用于分发状态节点，构造状态机。
* 3.支持备份与还原状态机，按照由上至下的层级顺序，序列化和反序列化有效状态ID。
* 4.提供添加、移除、查找指定状态节点之指定状态的方法，支持由下向上访问状态机，实现同级或者向上状态转换。除输入驱动外，另支持按照深度优先连续转换状态。
* 5.结合过渡实例和处理消息方法，支持在转换状态之前，动态生成并添加目标状态。
*   此方法仅限于生成同节点的状态，跨节点实现可以由上下节点自行设定时机，原则上节点内自治，以降低不同节点之间的耦合性。
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

	// 是否有效
	bool valid()
	{
		return _valid;
	}

	// 获取当前状态ID
	IDType getID()
	{
		return _id;
	}

	// 处理输入消息
	virtual TransitionType handle(MessageType _message)
	{
		// 若当前状态节点为无效状态，返回无效过渡实例
		if (!_valid)
			return TransitionType();

		// 传递消息至当前状态，获取返回的过渡实例
		typename MappingType::iterator iterator = _mapping.find(_id);
		if (iterator == _mapping.end())
			return TransitionType();
		TransitionType transition = iterator->second->handle(_message);

		// 若过渡实例为无效或者忽略类型，不处理而返回至上层
		if (transition.invalid() || transition.ignore())
			return transition;

		// 若过渡实例为创建类型，添加指定状态至当前状态节点
		if (transition.createable())
			addState(transition.getID(), transition.getState());

		/* 根据过渡实例转换当前状态 */
		// 若转换状态之后，不能够再次转换，返回过渡实例
		if (!update(transition))
			return transition;
		// 若转换状态之后，仍然存在转换的可能性，则递归至无法转换为止
		return handle(_message);
	}

	/*
	 * 分发状态节点，用于构造状态机。
	 *
	 * 参数：
	 *     1.节点向量：反序列化状态节点，按照深度优先构造状态树。
	 *     2.游标：索引当前状态节点。
	 * 返回值：
	 *     构造失败返回false，构造成功返回true。
	 *
	 * 若游标大于等于节点数量，则参数有误，可能是节点数量或者顺序错误。
	 * 若添加节点失败，即构造失败，可能向同一状态机添加相同ID的节点。
	 */
	//bool distribute(const std::vector<NodeType>& _nodes, SizeType _cursor = 0);

	/*
	 * 收集状态节点，用于释放资源。
	 *
	 * 参数：
	 *     1.节点向量：序列化状态节点，按照深度优先遍历状态树。
	 *     2.游标：索引当前状态节点。
	 * 返回值：
	 *     收集失败返回false，收集成功返回true。
	 *
	 * 若游标不等于节点数量，即游标位置有误，收集失败。
	 */
	//bool collect(std::vector<NodeType>& _nodes, SizeType _cursor = 0);

	/*
	 * 备份状态机的有效状态
	 * 按照由上至下的层级顺序入栈有效状态，直至遇到无效状态或者最下层节点为止。
	 *
	 * 参数：
	 *     1.状态向量栈：序列化有效状态。
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
	 * 还原状态机至指定状态
	 * 按照由上至下的层级顺序还原状态机，直至遍历状态完成，或者遇到最下层节点为止。
	 *
	 * 参数：
	 *     1.状态向量栈：反序列化有效状态。
	 *     2.层级：索引当前状态节点。
	 * 返回值：
	 *     还原失败返回false，还原成功返回true。
	 *
	 * 若未找到指定ID的节点，则还原失败，可能是状态向量栈或者初始层级有误，也可能是存在动态节点，而未提前生成节点。
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
	 * 转换指定状态节点的状态
	 * 
	 * 参数：
	 *     1.过渡实例：指示状态节点的转换操作。
	 *     2.层级：向上跨越的状态节点数量。默认为零，即当前状态节点。
	 * 返回值：
	 *     转换失败返回false，转换成功返回true。
	 * 
	 * 若不属于任何状态节点，或者状态节点为无效状态，则转换失败。
	 * 若过渡实例为无效或者忽略类型，则转换失败。
	 */
	//bool convert(TransitionType& _transition, SizeType _cursor = 0);

	/*
	 * 按照序列转换状态至指定状态
	 * 调用需要满足前提条件，即状态在整体上呈线性分布。
	 * 从当前状态起，连续转换状态，直到目标状态为止。若目标状态不存在，则转换至最后状态的下一状态。
	 * 
	 * 参数：
	 *     1.状态ID：索引目标状态。
	 *     2.节点向量：指明状态节点序列，按照深度优先遍历状态树。
	 *     3.游标：索引当前状态节点。
	 * 返回值：
	 *     转换失败返回false，转换成功返回true。
	 * 
	 * 若游标大于等于节点数量，则参数有误，可能是节点数量或者顺序错误。
	 * 若未找到给定状态节点，则转换失败，可能是节点数量或者顺序错误。
	 */
	bool update(IDType _id, const std::vector<NodeType>& _nodes, SizeType _cursor = 0)
	{
		return StateType::update(_id, _nodes, _cursor);
	}

	/*
	 * 按照顺序转换状态至指定状态
	 * 调用需要满足两个前提条件。其一，状态在整体上呈线性分布。其二，相同节点的不同状态按照ID从小到大的顺序排列。
	 * 若目标状态是当前状态或者已经过状态，则不会转换状态。否则连续转换状态直至目标状态。
	 * 若目标状态不存在，则转换至最后状态的下一状态。
	 * 
	 * 参数：
	 *     1.状态ID：索引目标状态。
	 */
	void update(IDType _id)
	{
		StateType::update(_id);
	}

	/*
	 * 添加状态至当前状态节点
	 * 
	 * 参数：
	 *     1.状态ID：索引待添加状态。
	 *     2.状态指针：指向已创建的状态实例。
	 * 返回值：
	 *     添加失败返回false，添加成功返回true。
	 * 
	 * 若状态指针为空，或者存在相同状态ID，则添加失败。
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
	 * 添加状态至给定状态节点
	 * 以当前状态节点为根，向下遍历至给定状态节点，并且插入待添加状态。
	 * 
	 * 参数：
	 *     1.状态ID：索引待添加状态。
	 *     2.状态指针：指向已创建的状态实例。
	 *     3.状态ID向量：按照从上至下的层级顺序搜索状态节点。
	 *     4.层级：索引当前状态节点。
	 * 返回值：
	 *     添加失败返回false，添加成功返回true。
	 * 
	 * 若层级大于状态ID数量，则添加失败，可能是状态ID向量或者初始层级有误。
	 * 若未找到给定状态节点，则添加失败。
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
	 * 移除当前状态节点的指定状态
	 * 
	 * 参数：
	 *     1.状态ID：索引待移除状态。
	 * 返回值：
	 *     移除失败返回NULL，移除成功返回移除的状态指针。
	 * 
	 * 若当前状态节点有效，并且待移除状态是当前状态，或者未找到待移除状态，则移除失败。
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
	 * 移除指定状态节点的指定状态
	 * 以当前状态节点为根，向下遍历至给定状态节点，并且移除目标状态。
	 * 
	 * 参数：
	 *     1.状态ID：索引待移除状态。
	 *     2.状态ID向量：按照从上至下的层级顺序搜索状态节点。
	 *     3.层级：索引当前状态节点。
	 * 返回值：
	 *     移除失败返回NULL，移除成功返回移除的状态指针。
	 * 
	 * 若层级大于状态ID数量，则移除失败，可能是状态ID向量或者初始层级有误。
	 * 若未找到指定状态节点，则移除失败。
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
	 * 设置当前状态节点的当前状态
	 * 
	 * 参数：
	 *     1.状态ID：索引目标状态。
	 * 返回值：
	 *     设置失败返回false，设置成功返回true。
	 * 
	 * 若当前状态节点为有效状态，或者未找到目标状态，则设置失败。
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
	 * 设置指定状态节点的当前状态
	 * 以当前状态节点为根，向下遍历至给定状态节点，并且设置当前状态为目标状态。
	 * 
	 * 参数：
	 *     1.状态ID：索引目标状态。
	 *     2.状态ID向量：按照从上至下的层级顺序搜索状态节点。
	 *     3.层级：索引当前状态节点。
	 * 返回值：
	 *     设置失败返回false，设置成功返回true。
	 * 
	 * 若层级大于状态ID数量，则设置失败，可能是状态ID向量或者初始层级有误。
	 * 若未找到指定状态节点，则设置失败。
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
	 * 获取当前状态节点的指定状态
	 * 
	 * 参数：
	 *     1.状态ID：索引目标状态。
	 * 返回值：
	 *     获取失败返回NULL，获取成功返回目标状态指针。
	 * 
	 * 若当前状态节点为无效状态，或者未找到目标状态，则获取失败。
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
	 * 获取指定状态节点的指定状态
	 * 以当前状态节点为根，向下遍历至给定状态节点，并且获取目标状态。
	 * 
	 * 参数：
	 *     1.状态ID：索引目标状态。
	 *     2.状态ID向量：按照从上至下的层级顺序搜索状态节点。
	 *     3.层级：索引当前状态节点。
	 * 返回值：
	 *     获取失败返回NULL，获取成功返回目标状态指针。
	 * 
	 * 若层级大于状态ID数量，则获取失败，可能是状态ID向量或者初始层级有误。
	 * 若未找到指定状态节点，则获取失败。
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
	 * 获取当前状态节点的当前状态
	 * 
	 * 返回值：
	 *     获取失败返回NULL，获取成功返回目标状态指针。
	 * 
	 * 若当前状态节点为无效状态，则获取失败。
	 */
	StateType* getState()
	{
		return getState(_id);
	}

	/*
	 * 获取指定状态节点的当前状态
	 * 以当前状态节点为根，向下遍历至给定状态节点，并且获取当前状态。
	 * 
	 * 参数：
	 *     1.状态ID向量：按照从上至下的层级顺序搜索状态节点。
	 *     2.层级：索引当前状态节点。
	 * 返回值：
	 *     获取失败返回NULL，获取成功返回目标状态指针。
	 * 
	 * 若层级大于状态ID数量，则获取失败，可能是状态ID向量或者初始层级有误。
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
	 * 根据过渡实例转换状态
	 * 
	 * 参数：
	 *     1.过渡实例：指示状态节点的转换操作。
	 * 返回值：
	 *     若转换状态之后，不能够再次转换，则返回false。
	 *     若转换状态之后，仍然存在转换的可能性，则返回true。
	 */
	bool update(TransitionType& _transition)
	{
		// 若存在则退出当前状态
		typename MappingType::iterator previous = _mapping.find(_id);
		if (previous != _mapping.end())
			previous->second->exit();

		// 若过渡实例为空类型，说明当前状态节点无后续状态，则设为无效状态
		if (_transition.empty())
			return _valid = false;

		// 若目标状态不存在，说明状态节点发生异常，可能是目标状态错误，或者未提前添加目标状态
		typename MappingType::iterator next = _mapping.find(_transition.getID());
		if (next == _mapping.end())
		{
			std::cerr << "bool update(TransitionType&): switch to an invalid state." << std::endl;
			// 过渡实例设为无效类型
			_transition.setType(TransitionType::INVALID);
			return false;
		}

		// 当前状态节点的当前状态设为目标状态
		_id = _transition.getID();
		// 进入目标状态
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
	 * 收集状态节点
	 *
	 * 参数：
	 *     1.节点向量：序列化状态节点，按照深度优先遍历状态树，初始需要添加一个空节点。
	 *     2.游标：索引当前状态节点，初始需要指向添加的空节点。
	 *
	 * 对于节点向量的初始化，可以调用静态函数MultiState::initCollection，无需手动添加空节点。
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
		// 若当前状态节点为无效状态，返回无效过渡实例
		if (!_valid)
			return false;

		// 若过渡实例为无效或者忽略类型，不处理而直接返回
		if (_transition.invalid() || _transition.ignore())
			return false;

		// 若过渡实例为创建类型，添加指定状态至当前状态节点
		if (_transition.createable())
			addState(_transition.getID(), _transition.getState());

		// 根据过渡实例转换当前状态
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
