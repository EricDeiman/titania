#ifndef IR_HH
#define IR_HH

#include <cstddef>
#include <iostream>
#include <vector>

#include "codebuffer.hh"
#include "basicblock.hh"

class IR {
public:
    IR( std::vector< CodeBuffer >&& s ) : fnBuffers( s ) {}

    void 
    mkBasicBlocks();

    std::ostream&
    dumpBasicBlocks( std::ostream &os );

private:
    std::vector< CodeBuffer >fnBuffers;

    void
    mkBasicBlock( CodeBuffer & );

    bool
    isJumpInsr( std::string );

};

#endif