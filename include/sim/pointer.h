#ifndef POINTER_H
#define POINTER_H

#include <vector>

#include "sim/page.h"

class Pointer {
private:
    unsigned int mIdCount;
    unsigned int pidOwner;
    std::vector<Page> mPages;
public:
    const unsigned int id;

    Pointer(unsigned int &idCount);

    // --- Getters ---

    // Pages, mut and read only
    std::vector<Page>& getPages();
    const std::vector<Page>& getPages() const;


    // --- Setters ---
    void assignPages(int num, int size, unsigned int &idCount);
    // void setOwner(unsigned int num)
    
};

#endif // POINTER_H
