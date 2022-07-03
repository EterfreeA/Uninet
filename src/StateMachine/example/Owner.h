#pragma once

#include <ctime>
#include <vector>

#include "StateMachine.hpp"

class Owner
{
public:
    enum STAGE
    {
        STAGE_BEGIN,
        STAGE_A,
        STAGE_A1,
        STAGE_A2,
        STAGE_B,
        STAGE_C,
        STAGE_C1,
        STAGE_C2,
        STAGE_END,
    };

public:
    typedef StateMachine<STAGE, std::time_t> StateMachineType;
    typedef StateMachineType::NodeType NodeType;

    typedef StateMachineType::StateType StateType;
    typedef StateMachineType::TransitionType TransitionType;

private:
    StateMachineType _stateMachine;
    std::vector<NodeType> _nodes;
    STAGE _stage;

private:
    void distributeStateMachine();
    void collectStateMachine();

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

    const std::vector<NodeType>& getNodes() const
    {
        return _nodes;
    }

    void setStage(STAGE _stage)
    {
        this->_stage = _stage;
    }

    STAGE getStage() const
    {
        return _stage;
    }
};

class A : public Owner::StateMachineType
{
    typedef Owner::StateMachineType StateMachineType;

private:
    Owner& _owner;
    MessageType _delayTime;

public:
    A(Owner& _owner, MessageType _delayTime)
        : _owner(_owner), _delayTime(_delayTime) {}

    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class B : public Owner::StateType
{
    Owner& _owner;
    MessageType _endTime;

public:
    B(Owner& _owner, MessageType _endTime)
        : _owner(_owner), _endTime(_endTime) {}

    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class C : public Owner::StateMachineType
{
    typedef Owner::StateMachineType StateMachineType;

private:
    Owner& _owner;

public:
    C(Owner& _owner) : _owner(_owner) {}
    virtual void enter();
    virtual void exit();
};

class A1 : public Owner::StateType
{
    Owner& _owner;
    MessageType _endTime;

public:
    A1(Owner& _owner, MessageType _endTime)
        : _owner(_owner), _endTime(_endTime) {}

    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class A2 : public Owner::StateType
{
    Owner& _owner;
    MessageType _endTime;

public:
    A2(Owner& _owner, MessageType _endTime)
        : _owner(_owner), _endTime(_endTime) {}

    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class C1 : public Owner::StateType
{
    Owner& _owner;
    MessageType _endTime1;
    MessageType _endTime2;

public:
    C1(Owner& _owner, MessageType _endTime1, MessageType _endTime2)
        : _owner(_owner), _endTime1(_endTime1), _endTime2(_endTime2) {}

    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};

class C2 : public Owner::StateType
{
    Owner& _owner;
    MessageType _endTime;

public:
    C2(Owner& _owner, MessageType _endTime)
        : _owner(_owner), _endTime(_endTime) {}

    virtual void enter();
    virtual void exit();
    virtual TransitionType handle(MessageType _message);
};
