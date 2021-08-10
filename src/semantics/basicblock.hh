#ifndef BASICBLOCK_HH
#define BASICBLOCK_HH

#include <cstddef>

// the indecies are inclusive
class BasicBlock {
public:
    size_t startIdx;
    size_t endIdx;
};


#endif