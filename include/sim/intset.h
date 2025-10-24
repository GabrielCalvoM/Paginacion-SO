#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstddef>

enum IntType {
    newI,
    useI,
    delI,
    killI
};

class Instruction {
public:
    const IntType type;
    const unsigned int param1;
    const size_t param2;

    Instruction(InstructionE type, unsigned int param1) :
        type(type), param1(param1), param2(0) {};
    Instruction(InstructionE type, unsigned int param1, size_t param2) :
        type(type), param1(param1), param2(param2) {};

};

class IntSet {
public:
    static unsigned int maxInstructions;
    std::unordered_map<unsigned int, Instruction> set;
    std::vector<unsigned int> order;
    unsigned int nextId = 1;
    unsigned int numInstructions = 0;


    IntSet() = default;

    unsigned int size() const { return numInstructions; }

    bool loadSet(const std::string &filepath);

    void saveSet() const;

    const std::vector<unsigned int>& getOrder() const 
        { return order; }
    
    const Instruction* getInstruction(unsigned int id) const {
        auto it = set.find(id);
        return it == set.end() ? nullptr : &it->second;
    }

    bool pushInstruction(const Instruction &ins);
};

#endif // INSTRUCTION_H
