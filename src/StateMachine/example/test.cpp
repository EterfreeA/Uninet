#include "Owner.hpp"

#include <iostream>

int main()
{
    std::cout << "begin" << std::endl;
    Owner owner;
    StateMachineType& stateMachine = owner.getStateMachine();
    //for (Stage index = STAGE_A, condition = STAGE_C; index <= condition; ++index)
    //    stateMachine.handle(index);

    stateMachine.handle(STAGE_A);
    std::cout << "\nbackup\n" << std::endl;
    std::vector<IDType> ids;
    stateMachine.backup(ids);
    stateMachine.update(STAGE_C, owner.getNodes());
    //stateMachine.update(STAGE_C);
    stateMachine.handle(STAGE_C2);
    std::cout << "\nrecover\n" << std::endl;
    stateMachine.recover(ids);
    stateMachine.handle(STAGE_C2);
    std::cout << "end" << std::endl;
    return 0;
}
