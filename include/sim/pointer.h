#ifndef POINTER_H
#define POINTER_H

#include <vector>

#include "sim/page.h"

class Pointer {
private:
    static unsigned int mIdCount;
    std::vector<Page> mPages;
public:
    const unsigned int id;

    Pointer();

    // --- Getters ---
    std::vector<Page> getPages() const;

    // --- Setters ---
    void assignPages(int num);
    
};

#endif // POINTER_H