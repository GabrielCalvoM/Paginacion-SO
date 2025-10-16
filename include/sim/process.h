#ifndef PROCESS_H
#define PROCESS_H

#include <vector>

class Process {
private:
    static unsigned int mIdCount;
    std::vector<unsigned int> mPointers;

public:
    const unsigned int pId;

    Process();

    // --- Getters ---
    std::vector<unsigned int> getPointers() const;

    // --- Setters ---
    void assignPtr(unsigned int ptr);
    void deletePtr(unsigned int ptr);

};

#endif // PROCESS_H
