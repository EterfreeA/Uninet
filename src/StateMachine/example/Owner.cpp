#include "Owner.hpp"

#include <iostream>

void printOwner(Owner* _owner)
{
    std::cout << "stage: " << _owner->getStage() << std::endl;
}

void Owner::distributeStateMachine()
{
    _nodes.reserve(8);
    _nodes.push_back(NodeType(0, NULL, 3));
    _nodes.push_back(NodeType(STAGE_A, new A(this), 2));
    _nodes.push_back(NodeType(STAGE_A1, new A1(this), 0));
    _nodes.push_back(NodeType(STAGE_A2, new A2(this), 0));
    _nodes.push_back(NodeType(STAGE_B, new B(this), 0));
    _nodes.push_back(NodeType(STAGE_C, new C(this), 2));
    _nodes.push_back(NodeType(STAGE_C1, new C1(this), 0));
    _nodes.push_back(NodeType(STAGE_C2, new C2(this), 0));

    // ����״̬�ڵ�
    //_nodes.reserve(5);
    //_nodes.push_back(NodeType(0, NULL, 2));
    //_nodes.push_back(NodeType(STAGE_A, new A(this), 2));
    //_nodes.push_back(NodeType(STAGE_A1, new A1(this), 0));
    //_nodes.push_back(NodeType(STAGE_A2, new A2(this), 0));
    //_nodes.push_back(NodeType(STAGE_B, new B(this), 0));

    // �ַ�״̬�ڵ㣬����״̬��
    _stateMachine.distribute(_nodes);
    // ���ָ��״̬����ǰ״̬�ڵ�
    //_stateMachine.addState(STAGE_C, new C(this));
    // ���ָ��״̬��ָ��״̬�ڵ�
    //std::vector<IDType> ids(1, STAGE_C);
    //_stateMachine.addState(STAGE_C1, new C1(this), ids);
    ////_stateMachine.addState(STAGE_C2, new C2(this), ids);

    // ���õ�ǰ״̬
    _stateMachine.setState(STAGE_A);
}

void Owner::collectStateMachine()
{
    // �ռ�״̬�ڵ�
    std::vector<NodeType> nodes;
    _stateMachine.collect(nodes);

    // �ͷ�״̬ʵ��
    for (std::vector<NodeType>::size_type index = 0; index < nodes.size(); ++index)
        delete nodes[index]._state;
}

void A::enter()
{
    std::cout << "void A::enter()" << std::endl;
    _owner->setStage(STAGE_A);
    printOwner(_owner);
    setState(STAGE_A1);

    //TransitionType transition(TransitionType::CONVERTIBLE, STAGE_C);
    //convert(transition);
}

void A::exit()
{
    std::cout << "void A::exit()" << std::endl;
}

TransitionType A::handle(MessageType _message)
{
    TransitionType transition = StateMachineType::handle(_message);
    // �����ڲ�״̬����ת��֮���޿�ת��״̬��������ʵ��Ϊ�����ͣ��޸Ĺ��ɶ���Ϊת�����ͣ�����ָ��Ŀ��״̬B
    if (transition.empty())
        transition.set(TransitionType::CONVERTIBLE, STAGE_B);
    return transition;
}

void B::enter()
{
    std::cout << "void B::enter()" << std::endl;
    _owner->setStage(STAGE_B);
    printOwner(_owner);
}

void B::exit()
{
    std::cout << "void B::exit()" << std::endl;
}

TransitionType B::handle(MessageType _message)
{
    // ��δ�ﵽת�����������غ���״̬�Ĺ���ʵ��
    if (_message < STAGE_B)
        return TransitionType(TransitionType::IGNORE);
    // ���ﵽת���������������޺���״̬�����ؿ�״̬�Ĺ���ʵ��
    return TransitionType(TransitionType::CONVERTIBLE, STAGE_C);
}

void C::enter()
{
    std::cout << "void C::enter()" << std::endl;
    _owner->setStage(STAGE_C);
    printOwner(_owner);
    setState(STAGE_C1);
}

void C::exit()
{
    std::cout << "void C::exit()" << std::endl;
}

TransitionType C::handle(MessageType _message)
{
    return StateMachineType::handle(_message);
}

void A1::enter()
{
    std::cout << "void A1::enter()" << std::endl;
    _owner->setStage(STAGE_A1);
    printOwner(_owner);
}

void A1::exit()
{
    std::cout << "void A1::exit()" << std::endl;
}

TransitionType A1::handle(MessageType _message)
{
    if (_message < STAGE_A1)
        return TransitionType(TransitionType::IGNORE);
    return TransitionType(TransitionType::CONVERTIBLE, STAGE_A2);
}

void A2::enter()
{
    std::cout << "void A2::enter()" << std::endl;
    _owner->setStage(STAGE_A2);
    printOwner(_owner);
}

void A2::exit()
{
    std::cout << "void A2::exit()" << std::endl;
}

TransitionType A2::handle(MessageType _message)
{
    if (_message < STAGE_A2)
        return TransitionType(TransitionType::IGNORE);
    return TransitionType(TransitionType::EMPTY);
}

void C1::enter()
{
    std::cout << "void C1::enter()" << std::endl;
    _owner->setStage(STAGE_C1);
    printOwner(_owner);
}

void C1::exit()
{
    std::cout << "void C1::exit()" << std::endl;
}

TransitionType C1::handle(MessageType _message)
{
    // ��δ�ﵽת�����������غ���״̬�Ĺ���ʵ��
    if (_message < STAGE_C1)
        return TransitionType(TransitionType::IGNORE);
    return TransitionType(TransitionType::CONVERTIBLE, STAGE_C2);
    // ���ﵽת�����������ش���״̬�Ĺ���ʵ����������ָʾ״̬�ڵ�ת��������״̬
    //return TransitionType(TransitionType::CREATEABLE, STAGE_C2, new C2(_owner));
}

void C2::enter()
{
    std::cout << "void C2::enter()" << std::endl;
    _owner->setStage(STAGE_C2);
    printOwner(_owner);
}

void C2::exit()
{
    std::cout << "void C2::exit()" << std::endl;
}

TransitionType C2::handle(MessageType _message)
{
    if (_message < STAGE_C2)
        return TransitionType(TransitionType::IGNORE);
    return TransitionType(TransitionType::EMPTY);
}
