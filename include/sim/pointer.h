#ifndef POINTER_H
#define POINTER_H

#include <vector>
#include <memory>

#include "sim/page.h"

class Pointer {
private:
    unsigned int mIdCount;
    unsigned int pidOwner;
    mutable std::vector<std::unique_ptr<Page>> mPages;
public:
    const unsigned int id;

    Pointer(unsigned int &idCount);
    ~Pointer();
    
    // --- Getters ---

    // Pages, mut and read only
    std::vector<std::unique_ptr<Page>>& getPages()  const;


    // --- Setters ---
    void assignPages(int size, unsigned int &idCount);
    // void setOwner(unsigned int num)
    
};

#endif // POINTER_H
