#ifndef BASICBLOCK_HH
#define BASICBLOCK_HH

#include <cstddef>
#include <string>
#include <vector>

// the indecies are inclusive
// rather than try to erase lines out of the vector to remove iloc code,
// replace it with something like "---" and remove them all at once when regenerating
// the iloc
class BasicBlock {
public:
    std::string name;

    size_t startIdx;
    size_t endIdx;

    std::vector< std::string >
    codeBlock;
};


#endif