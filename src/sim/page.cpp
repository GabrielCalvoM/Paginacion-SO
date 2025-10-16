#include "sim/page.h"

// Static Attributes
unsigned int Page::mIdCount = 0;

// Constructor
Page::Page() : id(++mIdCount) {}

// --- Getters ---
unsigned int Page::getPhysicalDir() const { return mPhysicalDir; }
bool Page::isInRealMem() const { return mInRealMem; }
bool Page::hasSecondChance() const { return mSecondChance; }

// --- Setters ---
void Page::setPhysicalDir(unsigned int dir) { mPhysicalDir = dir; }
void Page::setInRealMem(bool value) { mInRealMem = value; }
void Page::setSecondChance(bool value) { mSecondChance = value; }
