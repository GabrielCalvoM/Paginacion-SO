#include "sim/page.h"

// Static Attributes
unsigned int Page::mIdCount = 0;

// Constructor
Page::Page(int space) : id(++mIdCount), space(space) { }

// --- Getters ---
unsigned int Page::getPhysicalDir() const { return mPhysicalDir; }
bool Page::isInRealMem() const { return mInRealMem; }
bool Page::hasSecondChance() const { return mSecondChance; }
//bool Page::getLastAccess() const { return mLastAccess; }

// --- Setters ---
void Page::setPhysicalDir(unsigned int dir) { mPhysicalDir = dir; }
void Page::setInRealMem(bool value) { mInRealMem = value; }
void Page::setSecondChance(bool value) { mSecondChance = value; }
void Page::setSpace(int value) { space = value; }

//void Page::incLastAccess() { mLastAccess += 1; }
//void Page::resetLastAccess() { mLastAccess = 0; }
