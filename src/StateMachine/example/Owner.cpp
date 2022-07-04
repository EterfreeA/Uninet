#include "Owner.h"

#include <cstddef>
#include <iostream>

static void printOwner(const Owner& _owner)
{
    std::cout << "Stage: " << _owner.getStage() << std::endl;
}

void Owner::distributeStateMachine()
{
    std::cout << "Owner::distributeStateMachine\n" << std::endl;
    std::time_t now = std::time(NULL);

    // 生成状态节点
    //_nodes.reserve(8);
    //_nodes.push_back(NodeType(STAGE_BEGIN, NULL, 3));
    //_nodes.push_back(NodeType(STAGE_A, new A(*this, now + 2), 2));
    //_nodes.push_back(NodeType(STAGE_A1, new A1(*this, now + 3), 0));
    //_nodes.push_back(NodeType(STAGE_A2, new A2(*this, now + 5), 0));
    //_nodes.push_back(NodeType(STAGE_B, new B(*this, now + 7), 0));
    //_nodes.push_back(NodeType(STAGE_C, new C(*this), 2));
    //_nodes.push_back(NodeType(STAGE_C1, new C1(*this, now + 8, now + 10), 0));
    //_nodes.push_back(NodeType(STAGE_C2, new C2(*this, now + 10), 0));

    _nodes.reserve(5);
    _nodes.push_back(NodeType(STAGE_BEGIN, NULL, 2));
    _nodes.push_back(NodeType(STAGE_A, new A(*this, now + 2), 2));
    _nodes.push_back(NodeType(STAGE_A1, new A1(*this, now + 3), 0));
    _nodes.push_back(NodeType(STAGE_A2, new A2(*this, now + 5), 0));
    _nodes.push_back(NodeType(STAGE_B, new B(*this, now + 7), 0));

    // 分发状态节点，构建状态机
    _stateMachine.distribute(_nodes);

    // 添加指定状态至当前状态机
    StateType* state = new C(*this);
    ++_nodes[0]._size;
    _nodes.push_back(NodeType(STAGE_C, state, 2));
    _stateMachine.addState(STAGE_C, state);

    // 添加指定状态至指定状态机
    state = new C1(*this, now + 8, now + 10);
    _nodes.push_back(NodeType(STAGE_C1, state, 0));
    std::vector<STAGE> ids(1, STAGE_C);
    _stateMachine.addState(STAGE_C1, state, ids);
    _nodes.push_back(NodeType(STAGE_C2, NULL, 0));
    //_stateMachine.addState(STAGE_C2, new C2(*this, now + 10), ids);

    // 设置当前状态
    _stateMachine.setState(STAGE_A);
}

void Owner::collectStateMachine()
{
    std::cout << "\nOwner::collectStateMachine" << std::endl;

    // 收集状态节点
    std::vector<NodeType> nodes;
    _stateMachine.collect(nodes);

    // 释放状态实例
    for (std::vector<NodeType>::size_type index = 0; \
        index < nodes.size(); ++index)
        delete nodes[index]._state;
}

void A::enter()
{
    std::cout << "A::enter" << std::endl;
    _owner.setStage(Owner::STAGE_A);
    printOwner(_owner);
}

void A::exit()
{
    std::cout << "A::exit" << std::endl;
}

A::TransitionType A::handle(MessageType _message)
{
    if (_message < _delayTime)
        return TransitionType(TransitionType::IGNORE);

    if (!valid()) setState(Owner::STAGE_A1);

    TransitionType transition = StateMachineType::handle(_message);

    // 对于内部状态，倘若无后续状态，即过渡实例为空类型，则修改过渡实例为转换类型，并且指定目标状态B
    if (transition.empty())
        transition.set(TransitionType::CONVERTIBLE, Owner::STAGE_B);
    return transition;
}

void B::enter()
{
    std::cout << "B::enter" << std::endl;
    _owner.setStage(Owner::STAGE_B);
    printOwner(_owner);
}

void B::exit()
{
    std::cout << "B::exit" << std::endl;
}

B::TransitionType B::handle(MessageType _message)
{
    // 倘若未达到转换条件，返回忽略类型的过渡实例
    if (_message < _endTime)
        return TransitionType(TransitionType::IGNORE);
    return TransitionType(TransitionType::CONVERTIBLE, Owner::STAGE_C);
}

void C::enter()
{
    std::cout << "C::enter" << std::endl;
    _owner.setStage(Owner::STAGE_C);
    printOwner(_owner);
    setState(Owner::STAGE_C1);
}

void C::exit()
{
    std::cout << "C::exit" << std::endl;
}

void A1::enter()
{
    std::cout << "A1::enter" << std::endl;
    _owner.setStage(Owner::STAGE_A1);
    printOwner(_owner);
}

void A1::exit()
{
    std::cout << "A1::exit" << std::endl;
}

A1::TransitionType A1::handle(MessageType _message)
{
    if (_message < _endTime)
        return TransitionType(TransitionType::IGNORE);
    return TransitionType(TransitionType::CONVERTIBLE, Owner::STAGE_A2);
}

void A2::enter()
{
    std::cout << "A2::enter" << std::endl;
    _owner.setStage(Owner::STAGE_A2);
    printOwner(_owner);
}

void A2::exit()
{
    std::cout << "A2::exit" << std::endl;
}

A2::TransitionType A2::handle(MessageType _message)
{
    if (_message < _endTime)
        return TransitionType(TransitionType::IGNORE);

    // 若达到转换条件，但无后续状态，返回空类型的过渡实例
    return TransitionType(TransitionType::EMPTY);
}

void C1::enter()
{
    std::cout << "C1::enter" << std::endl;
    _owner.setStage(Owner::STAGE_C1);
    printOwner(_owner);
}

void C1::exit()
{
    std::cout << "C1::exit" << std::endl;
}

C1::TransitionType C1::handle(MessageType _message)
{
    // 倘若未达到转换条件，返回忽略类型的过渡实例
    if (_message < _endTime1)
        return TransitionType(TransitionType::IGNORE);

    // 倘若达到转换条件，返回创建类型的过渡实例，创建并转换为后续状态
    return TransitionType(TransitionType::CREATEABLE, \
        Owner::STAGE_C2, new C2(_owner, _endTime2));
}

void C2::enter()
{
    std::cout << "C2::enter" << std::endl;
    _owner.setStage(Owner::STAGE_C2);
    printOwner(_owner);
}

void C2::exit()
{
    std::cout << "C2::exit" << std::endl;
}

C2::TransitionType C2::handle(MessageType _message)
{
    if (_message < _endTime)
        return TransitionType(TransitionType::IGNORE);
    return TransitionType(TransitionType::EMPTY);
}
