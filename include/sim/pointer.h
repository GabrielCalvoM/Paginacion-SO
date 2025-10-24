#ifndef POINTER_H
#define POINTER_H

#include <vector>

#include "sim/page.h"

class Pointer {
private:
    static unsigned int mIdCount;
    unsigned int pidOwner;
    std::vector<Page> mPages;
public:
    const unsigned int id;

    Pointer();

    // --- Getters ---

    // Pages, mut and read only
    std::vector<Page>& getPages();
    const std::vector<Page>& getPages() const;


    // --- Setters ---
    void assignPages(int num);
    // void setOwner(unsigned int num)
    
};

#endif // POINTER_H
