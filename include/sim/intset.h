#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
#include <unordered_map>
#include <vector>

typedef enum IntType {
    newI,
    useI,
    delI,
    killI
} IntTypeE;

const std::unordered_map<IntTypeE, std::string> intTypeString = {
    {newI, "new"},
    {useI, "use"},
    {delI, "delete"},
    {killI, "kill"}
};

const std::unordered_map<std::string, IntTypeE> stringToIntType = {
    {"new", newI},
    {"use", useI},
    {"delete", delI},
    {"kill", killI}
};

class Instruction {
public:
    const IntType type;
    const unsigned int param1;
    const size_t param2;

    Instruction(IntType type, unsigned int param1) :
        type(type), param1(param1), param2(0) {};
    Instruction(IntType type, unsigned int param1, size_t param2) :
        type(type), param1(param1), param2(param2) {};

};

class IntSet {
private:
    unsigned int mNextId = 1;
    unsigned int mNumInstructions = 0;
    std::vector<Instruction> mVec;
    mutable decltype(mVec.begin()) mIt;
    mutable bool mItNull = true;

public:
    static unsigned int maxInstructions;


    IntSet() = default;

    unsigned int size() const { return mNumInstructions; }

    std::string generateInstructions(unsigned int seed, unsigned int nProc, unsigned int nOp);

    std::string loadSet(const std::string filepath);

    void saveSet(const std::string filepath) const;
    
    Instruction* next() const {
        if (mIt == mVec.end()) { mItNull = true; return &*mIt;}
        return &*(mIt++);
    }

    bool pushInstruction(const Instruction ins);
};

#endif // INSTRUCTION_H
