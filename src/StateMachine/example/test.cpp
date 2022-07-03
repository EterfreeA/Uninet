#include "Owner.h"

#ifdef _WIN32
#include <Windows.h>

#elif defined(linux) || defined(__linux) || defined(__linux__)
#define LINUX
#include <unistd.h>

#else
#error "Currently only supports Windows and Linux!"
#endif

#include <ctime>
//#include <chrono>
#include <cstddef>
#include <vector>
#include <iostream>
//#include <thread>

int main()
{
    Owner owner;
    Owner::StateMachineType& stateMachine = owner.getStateMachine();

    // 备份状态树的有效节点路径
    std::cout << "\nStateMachine::backup\n" << std::endl;
    std::vector<Owner::STAGE> ids;
    stateMachine.backup(ids);

    // 输入驱动状态机
    Owner::TransitionType transition;
    do
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
#ifdef _WIN32
        Sleep(100);
#elif defined(LINUX)
        sleep(1);
#endif
        transition = stateMachine.handle(std::time(NULL));
    } while (!transition.empty() && !transition.invalid());

    // 还原状态树的有效节点路径
    std::cout << "\nStateMachine::recover\n" << std::endl;
    stateMachine.recover(ids);

    // 切换有效状态
    ids.assign(1, Owner::STAGE_A);
    stateMachine.setState(Owner::STAGE_A1, ids);

    // 按照顺序转换为指定状态
    //stateMachine.update(Owner::STAGE_B, owner.getNodes());
    stateMachine.update(Owner::STAGE_B);

    // 目标状态不存在，持续转换直至最后状态退出
    stateMachine.update(Owner::STAGE_END);
    return 0;
}
