#pragma once

#include <utility>
#include <cstddef>
#include <vector>
#include <map>
//#include <unordered_map>
#include <iostream>

#include "State.hpp"

template <typename _IDType, typename _MessageType, \
	typename _SizeType = std::size_t>
class StateMachine : public State<_IDType, _MessageType, _SizeType>
{
public:
	typedef _IDType IDType;
	typedef _MessageType MessageType;
	typedef _SizeType SizeType;

	typedef State<IDType, MessageType, SizeType> StateType;
	typedef std::map<IDType, StateType*> MappingType;
	//typedef std::unordered_map<IDType, StateType*> MappingType;

	typedef typename StateType::NodeType NodeType;
	typedef typename StateType::TransitionType TransitionType;

private:
	bool _valid;
	IDType _id;
	MappingType _mapping;

private:
	/*
	 * 分发状态节点以构造状态树
	 * _distribute是distribute具体实现
	 */
	virtual bool _distribute(const std::vector<NodeType>& _nodes, \
		SizeType& _cursor)
	{
		if (_cursor >= _nodes.size()) return false;

		const NodeType& node = _nodes[_cursor];
		if (_cursor++ + node._size >= _nodes.size()) return false;

		for (SizeType index = 0; index < node._size; ++index)
		{
			const NodeType& node = _nodes[_cursor];
			if (!addState(node._id, node._state)) return false;

			if (!node._state->_distribute(_nodes, _cursor))
				return false;
		}
		return true;
	}

	/*
	 * 收集状态节点
	 *
	 * 参数：
	 *     1.节点向量：序列化状态节点，按照深度优先遍历状态树，初始必须添加一个空节点。
	 *     2.游标：索引当前状态节点，初始必须指向空节点。
	 *
	 * 对于初始化节点向量，可以调用静态函数State::initCollection，无需手动添加空节点。
	 */
	virtual void _collect(std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		NodeType& node = _nodes[_cursor];
		node._size = _mapping.size();

		for (typename MappingType::iterator iterator = _mapping.begin(); \
			iterator != _mapping.end(); ++iterator)
		{
			_nodes.push_back(NodeType(iterator->first, iterator->second));
			iterator->second->_collect(_nodes, ++_cursor);
		}
	}

	/*
	 * 侧向或者向上状态转换
	 * _convert是convert具体实现
	 */
	virtual bool _convert(TransitionType& _transition)
	{
		// 倘若状态机为无效类型，返回无效过渡实例
		if (!_valid) return false;

		// 若过渡实例为无效或者忽略类型，则不作处理而直接返回
		if (_transition.invalid() || _transition.ignore())
			return false;

		// 若过渡实例为创建类型，则添加指定状态
		if (_transition.createable())
			addState(_transition.getID(), _transition.getState());

		// 根据过渡实例转换状态
		return update(_transition);
	}

	/*
	 * 按照序列转换为指定状态
	 * _update是update具体实现
	 */
	virtual int _update(IDType _id, \
		const std::vector<NodeType>& _nodes, SizeType& _cursor)
	{
		const NodeType& node = _nodes[_cursor];
		if (_cursor != 0 && node._id == _id) return 0;

		for (SizeType index = 0, size = node._size; \
			index < size; ++index)
		{
			if (++_cursor >= _nodes.size()) return -1;

			const NodeType& node = _nodes[_cursor];
			typename MappingType::iterator iterator = _mapping.find(node._id);
			if (iterator == _mapping.end()) return -1;

			int result = iterator->second->_update(_id, _nodes, _cursor);
			if (result <= 0) return result;

			if (_valid && node._id == this->_id)
			{
				TransitionType transition(TransitionType::EMPTY);
				if (index + 1 < size)
				{
					SizeType index = _cursor + 1;
					if (index >= _nodes.size()) return -1;

					const NodeType& node = _nodes[index];
					transition.set(TransitionType::CONVERTIBLE, node._id);
				}

				update(transition);
			}
		}
		return 1;
	}

	/*
	 * 按照顺序转换为指定状态
	 * _update是update具体实现
	 */
	virtual bool _update(IDType _id)
	{
		for (typename MappingType::iterator iterator = _mapping.begin(); \
			iterator != _mapping.end(); ++iterator)
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

public:
	// 进入事件
	virtual void enter() {}

	// 退出事件
	virtual void exit() {}

	// 输入驱动：发生输入事件，处理输入消息
	virtual TransitionType handle(MessageType _message)
	{
		// 倘若状态机为无效类型，返回无效过渡实例
		if (!_valid) return TransitionType();

		// 倘若有效状态不存在，返回无效过渡实例
		typename MappingType::iterator iterator = _mapping.find(_id);
		if (iterator == _mapping.end())
			return TransitionType();

		// 传递消息至有效状态，获取返回的过渡实例
		TransitionType transition = iterator->second->handle(_message);

		// 若过渡实例为无效或者忽略类型，则不作处理而直接返回
		if (transition.invalid() || transition.ignore())
			return transition;

		// 若过渡实例为创建类型，则添加指定状态
		if (transition.createable())
			addState(transition.getID(), transition.getState());

		// 根据过渡实例转换状态，在转换状态之后，若不能再次转换，则返回过渡实例
		if (!update(transition)) return transition;

		// 若仍然存在转换的可能性，则递归至无法转换为止
		return handle(_message);
	}

	/*
	 * 备份状态树的有效节点路径
	 * 自根节点向最底层节点遍历有效状态编号，直至遇到无效节点或者最底层节点为止。
	 *
	 * 参数：
	 *     1.有效状态编号向量：序列化有效状态编号。
	 */
	virtual void backup(std::vector<IDType>& _ids) const
	{
		if (!_valid) return;

		_ids.push_back(_id);

		typename MappingType::const_iterator iterator = _mapping.find(_id);
		if (iterator != _mapping.end())
			iterator->second->backup(_ids);
	}

	/*
	 * 还原状态树的有效节点路径
	 * 自根节点向最底层节点还原有效状态，直至遍历完节点，或者遇到最底层节点为止。
	 *
	 * 参数：
	 *     1.有效状态编号向量：反序列化有效状态编号。
	 *     2.游标：索引当前状态节点。
	 * 返回值：
	 *     还原失败返回false，还原成功返回true。
	 *
	 * 若未找到指定编号的节点，则还原失败，可能是有效状态编号向量或者游标初始位置有误，也可能是存在而未添加动态节点。
	 */
	virtual bool recover(const std::vector<IDType>& _ids, \
		SizeType _cursor = 0)
	{
		if (_cursor >= _ids.size()) return true;

		IDType id = _ids[_cursor];

		typename MappingType::iterator iterator = _mapping.find(id);
		if (iterator == _mapping.end()) return false;

		this->_id = id;
		_valid = true;
		return iterator->second->recover(_ids, _cursor + 1);
	}

public:
	/*
	 * 向指定状态机添加状态
	 * 自当前状态机向下搜索指定状态机，向其添加状态。
	 *
	 * 参数：
	 *     1.状态编号：用于索引待添加状态。
	 *     2.状态指针：指向状态实例。
	 *     3.状态编号向量：按照自上而下顺序搜索状态机。
	 *     4.游标：索引当前状态机。
	 * 返回值：
	 *     添加失败返回false，添加成功返回true。
	 *
	 * 若游标大于状态编号数量，则添加失败，可能是状态编号向量或者初始游标有误。
	 * 若未找到指定状态机，则添加失败。
	 */
	virtual bool addState(IDType _id, StateType* _state, \
		const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size()) return false;

		if (_cursor == _ids.size()) return addState(_id, _state);

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end()) return false;

		return iterator->second->addState(_id, _state, _ids, _cursor + 1);
	}

	/*
	 * 从指定状态机移除指定状态
	 * 自当前状态机向下搜索指定状态机，并且移除指定状态。
	 *
	 * 参数：
	 *     1.状态编号：索引待移除状态。
	 *     2.状态编号向量：按照由上至下顺序搜索状态。
	 *     3.游标：索引当前状态。
	 * 返回值：
	 *     移除失败返回NULL，移除成功返回状态指针。
	 *
	 * 若游标大于状态编号数量，则移除失败，可能是状态编号向量或者初始游标有误。
	 * 若未找到目标状态，则移除失败。
	 */
	virtual StateType* removeState(IDType _id, \
		const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size()) return NULL;

		if (_cursor == _ids.size()) return removeState(_id);

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end()) return NULL;

		return iterator->second->removeState(_id, _ids, _cursor + 1);
	}

	/*
	 * 设置指定状态机的当前状态
	 * 自当前状态机向下搜索指定状态机，并且设置当前状态为目标状态。
	 *
	 * 参数：
	 *     1.状态编号：索引目标状态。
	 *     2.状态编号向量：按照自上而下顺序搜索状态。
	 *     3.游标：索引当前状态。
	 * 返回值：
	 *     设置失败返回false，设置成功返回true。
	 *
	 * 若游标大于状态编号数量，则设置失败，可能是状态编号向量或者初始游标有误。
	 * 若未找到目标状态，则设置失败。
	 */
	virtual bool setState(IDType _id, \
		const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size()) return false;

		if (_cursor == _ids.size()) return setState(_id);

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end()) return NULL;

		return iterator->second->setState(_id, _ids, _cursor + 1);
	}

	/*
	 * 获取指定状态机的指定状态
	 * 自当前状态机向下搜索指定状态机，并且获取目标状态。
	 *
	 * 参数：
	 *     1.状态编号：索引目标状态。
	 *     2.状态编号向量：按照由上至下顺序搜索状态。
	 *     3.游标：索引当前状态。
	 * 返回值：
	 *     获取失败返回NULL，获取成功返回状态指针。
	 *
	 * 若游标大于状态编号数量，则获取失败，可能是状态编号向量或者初始游标有误。
	 * 若未找到目标状态，则获取失败。
	 */
	virtual StateType* getState(IDType _id, \
		const std::vector<IDType>& _ids, SizeType _cursor = 0)
	{
		if (_cursor > _ids.size()) return NULL;

		if (_cursor == _ids.size()) return getState(_id);

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end()) return NULL;

		return iterator->second->getState(_id, _ids, _cursor + 1);
	}

	/*
	 * 获取指定状态机的当前状态
	 * 自当前状态机向下搜索指定状态机，并且获取当前状态。
	 *
	 * 参数：
	 *     1.状态编号向量：按照由上至下顺序搜索状态。
	 *     2.游标：索引当前状态。
	 * 返回值：
	 *     获取失败返回NULL，获取成功返回状态指针。
	 *
	 * 若游标大于状态编号数量，则获取失败，可能是状态编号向量或者初始游标有误。
	 * 若未找到目标状态，则获取失败。
	 */
	virtual StateType* getState(const std::vector<IDType>& _ids, \
		SizeType _cursor = 0)
	{
		if (_cursor > _ids.size()) return NULL;

		if (_cursor == _ids.size()) return getState();

		typename MappingType::iterator iterator = _mapping.find(_ids[_cursor]);
		if (iterator == _mapping.end()) return NULL;

		return iterator->second->getState(_ids, _cursor + 1);
	}

private:
	/*
	 * 根据过渡实例转换状态
	 *
	 * 参数：
	 *     1.过渡实例：指示转换方式。
	 * 返回值：
	 *     在转换状态之后，倘若不能再次转换，返回false；若仍然存在转换的可能性，则返回true。
	 */
	bool update(TransitionType& _transition)
	{
		// 若存在则退出当前状态
		typename MappingType::iterator previous = _mapping.find(_id);
		if (previous != _mapping.end())
			previous->second->exit();

		// 若过渡实例为空类型，说明当前状态无后续状态，则状态机设为无效类型
		if (_transition.empty())
			return _valid = false;

		// 倘若目标状态不存在，说明状态机出现异常，可能是目标状态错误，或者未提前添加目标状态
		typename MappingType::iterator next = _mapping.find(_transition.getID());
		if (next == _mapping.end())
		{
			std::cerr << "bool update(TransitionType&): switch to an invalid state." << std::endl;

			// 过渡实例设为无效类型
			_transition.setType(TransitionType::INVALID);
			return false;
		}

		// 当前状态改为目标状态
		_id = _transition.getID();

		// 进入目标状态
		next->second->enter();
		return true;
	}

public:
	StateMachine() : _valid(false) {}

	// 是否有效
	bool valid() const { return _valid; }

	// 获取当前状态编号
	IDType getID() const { return _id; }

	/*
	 * 分发状态节点以构造状态树
	 *
	 * 参数：
	 *     1.节点向量：反序列化状态节点，按照深度优先构造状态树。起始位置必须是空节点，且指定子节点数量。
	 *     2.游标：索引当前状态节点，初始必须指向空节点。
	 * 返回值：
	 *     构造失败返回false，构造成功返回true。
	 *
	 * 若游标大于等于节点数量，则参数有误而构造失败，可能是节点数量或者顺序错误。
	 * 若添加节点失败，则构造失败，可能是节点路径与编号重复。
	 */
	//bool distribute(const std::vector<NodeType>& _nodes, SizeType _cursor = 0);

	/*
	 * 收集状态节点以释放资源
	 *
	 * 参数：
	 *     1.节点向量：序列化状态节点，按照深度优先遍历状态树。
	 *     2.游标：索引当前状态节点。
	 * 返回值：
	 *     收集失败返回false，收集成功返回true。
	 *
	 * 若游标不等于节点数量，则游标位置有误而收集失败。
	 */
	//bool collect(std::vector<NodeType>& _nodes, SizeType _cursor = 0);

	/*
	 * 侧向或者向上状态转换
	 * 
	 * 参数：
	 *     1.过渡实例：指示转换方式。
	 *     2.游标：向上跨越的状态机数量。目前仅支持默认值零，即当前状态机。
	 * 返回值：
	 *     转换失败返回false，转换成功返回true。
	 * 
	 * 若状态不属于任何状态机，或者状态机为无效类型，则转换失败。
	 * 若过渡实例为无效或者忽略类型，则转换失败。
	 */
	//bool convert(TransitionType& _transition, SizeType _cursor = 0);

	/*
	 * 按照序列转换为指定状态
	 * 自最底层有效状态起，按照节点向量，依次转换状态，直至目标状态为止。
	 * 若目标状态是当前状态或者已过状态，则不会转换状态。
	 * 若目标状态不存在，则持续转换直至最后状态退出，并且状态机设为无效类型。
	 * 
	 * 前提条件：
	 *     状态在整体上呈线性分布。
	 * 参数：
	 *     1.状态编号：索引目标状态。
	 *     2.节点向量：指定状态节点序列，按照深度优先遍历状态树。
	 *     3.游标：索引当前状态节点。
	 * 返回值：
	 *     转换失败返回false，转换成功返回true。
	 * 
	 * 若游标大于等于节点数量，则参数有误而转换失败，可能是节点数量或者顺序错误。
	 * 若未找到指定状态节点，则转换失败，可能是节点数量或者顺序错误。
	 */
	bool update(IDType _id, const std::vector<NodeType>& _nodes, \
		SizeType _cursor = 0)
	{
		return StateType::update(_id, _nodes, _cursor);
	}

	/*
	 * 按照顺序转换为指定状态
	 * 自最底层有效状态起，按照状态编号，依次转换状态，直至目标状态为止。
	 * 若目标状态是当前状态或者已过状态，则不会转换状态。
	 * 若目标状态不存在，则持续转换直至最后状态退出，并且状态机设为无效类型。
	 * 
	 * 前提条件：
	 *     1.状态在整体上呈线性分布。
	 *     2.相同状态机的不同状态按照编号升序排列。
	 * 参数：
	 *     1.状态编号：索引目标状态。
	 */
	void update(IDType _id)
	{
		StateType::update(_id);
	}

	/*
	 * 向当前状态机添加状态
	 * 
	 * 参数：
	 *     1.状态编号：用于索引待添加状态。
	 *     2.状态指针：指向状态实例。
	 * 返回值：
	 *     添加失败返回false，添加成功返回true。
	 * 
	 * 若状态指针为空，或者状态编号重复，则添加失败。
	 */
	bool addState(IDType _id, StateType* _state)
	{
		if (_state == NULL || _state == this)
			return false;

		if (_mapping.find(_id) != _mapping.end())
			return false;

		if (!_mapping.insert(std::make_pair(_id, _state)).second)
			return false;

		_state->setOwner(this);
		return true;
	}

	/*
	 * 从当前状态机移除指定状态
	 * 
	 * 参数：
	 *     1.状态编号：索引待移除状态。
	 * 返回值：
	 *     移除失败返回NULL，移除成功返回状态指针。
	 * 
	 * 若当前状态机有效，并且待移除状态是有效状态，或者未找到待移除状态，则移除失败。
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
	 * 设置当前状态机的有效状态
	 * 
	 * 参数：
	 *     1.状态编号：索引目标状态。
	 * 返回值：
	 *     设置失败返回false，设置成功返回true。
	 * 
	 * 若未找到目标状态，则设置失败。
	 */
	bool setState(IDType _id)
	{
		if (_valid)
		{
			typename MappingType::iterator previous = _mapping.find(this->_id);
			if (previous != _mapping.end())
				previous->second->exit();
		}

		typename MappingType::iterator next = _mapping.find(_id);
		if (next == _mapping.end())
			return false;

		this->_id = _id;
		_valid = true;
		next->second->enter();
		return true;
	}

	/*
	 * 获取当前状态机的指定状态
	 * 
	 * 参数：
	 *     1.状态编号：索引目标状态。
	 * 返回值：
	 *     获取失败返回NULL，获取成功返回状态指针。
	 * 
	 * 若未找到目标状态，则获取失败。
	 */
	StateType* getState(IDType _id)
	{
		typename MappingType::iterator iterator = _mapping.find(_id);
		return iterator != _mapping.end() ? iterator->second : NULL;
	}

	/*
	 * 获取当前状态机的当前状态
	 * 
	 * 返回值：
	 *     获取失败返回NULL，获取成功返回状态指针。
	 * 
	 * 若未找到目标状态，则获取失败。
	 */
	StateType* getState()
	{
		return getState(_id);
	}
};
