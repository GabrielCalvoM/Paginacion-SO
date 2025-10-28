#ifndef PAGE_H
#define PAGE_H

#include "constants.h"

class Page {
private:
    static unsigned int mIdCount;
    unsigned int mPhysicalDir = 0;
    unsigned int mLastAccess = 0;
    
    int space = 0;

    bool mInRealMem = 0;
    bool mSecondChance = 0;

public:
    static const unsigned int pageSize = Consts::PAGE_SIZE; // 4 KB
    const unsigned int id;

    Page(int space);

    // --- Getters ---
    unsigned int getPhysicalDir() const;
    int getSpace() const;
    bool isInRealMem() const;
    bool hasSecondChance() const;

    // --- Setters ---
    void setPhysicalDir(unsigned int);
    void setInRealMem(bool);
    void setAccess(int);
    void setSecondChance(bool);
    void setSpace(int);

};

#endif // PAGE_H
