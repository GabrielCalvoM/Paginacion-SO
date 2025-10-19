#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstddef>

enum InstructionE {
    newI,
    useI,
    deleteI,
    killI
};

class Instruction {
public:
    const InstructionE type;
    const unsigned int param1;
    const size_t param2;

    Instruction(InstructionE type, unsigned int param1) :
        type(type), param1(param1), param2(0) {};
    Instruction(InstructionE type, unsigned int param1, size_t param2) :
        type(type), param1(param1), param2(param2) {};

};

#endif // INSTRUCTION_H
