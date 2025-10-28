#ifndef PROCESS_H
#define PROCESS_H

#include <vector>

class Process {
private:
    unsigned int mIdCount;
    std::vector<unsigned int> mPointers;

public:
    const unsigned int pId;

    Process(unsigned int &idCount);

    // --- Getters ---
    std::vector<unsigned int> getPointers() const;

    // --- Setters ---
    void assignPtr(unsigned int ptr);
    void deletePtr(unsigned int ptr);

};

#endif // PROCESS_H
