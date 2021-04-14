#pragma once

#include <cstddef>
#include <vector>

#include "StateMachine.hpp"

enum Stage
{
    STAGE_A,
    STAGE_A1,
    STAGE_A2,
    STAGE_B,
    STAGE_C,
    STAGE_C1,
    STAGE_C2,
};

typedef size_t IDType;
typedef size_t MessageType;
typedef StateMachine<IDType, MessageType> StateMachineType;
typedef StateMachineType::StateType StateType;
typedef StateMachineType::TransitionType TransitionType;

class Owner
{
public:
    typedef StateMachineType::NodeType NodeType;

private:
    StateMachineType _stateMachine;
    std::vector<NodeType> _nodes;
    Stage _stage;

public:
    Owner() : _nodes(0)
    {
        distributeStateMachine();
    }
    ~Owner()
    {
        collectStateMachine();
    }

    StateMachineType& getStateMachine()
    {
        return _stateMachine;
    }

    const std::vector<NodeType>& getNodes()
    {
        return _nodes;
    }

    void setStage(Stage _stage)
    {
        this->_stage = _stage;
    }

    Stage getStage()
    {
        return _stage;
    }

private:
    void distributeStateMachine();
    void collectStateMachine();
};

class A : public StateMachineType
{
    Owner *const _owner;
public:
    A(Owner* _owner) : _owner(_owner) {}
    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class B : public StateType
{
    Owner* const _owner;
public:
    B(Owner* _owner) : _owner(_owner) {}
    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class C : public StateMachineType
{
    Owner* const _owner;
public:
    C(Owner* _owner) : _owner(_owner) {}
    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class A1 : public StateType
{
    Owner* const _owner;
public:
    A1(Owner* _owner) : _owner(_owner) {}
    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class A2 : public StateType
{
    Owner* const _owner;
public:
    A2(Owner* _owner) : _owner(_owner) {}
    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class C1 : public StateType
{
    Owner* const _owner;
public:
    C1(Owner* _owner) : _owner(_owner) {}
    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class C2 : public StateType
{
    Owner* const _owner;
public:
    C2(Owner* _owner) : _owner(_owner) {}
    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};
