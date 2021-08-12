#ifndef IR_HH
#define IR_HH

#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "codebuffer.hh"
#include "basicblock.hh"

class LvnMeta {
public:
    using lvnDict = std::unordered_map< std::string, size_t >;

    int
    lookUp( std::string );

    size_t
    add( std::string );

    size_t
    set( std::string, size_t );

    std::string
    replaceWithI2i( std::string );

    std::ostream &
    dumpLvn( std::ostream & );

private:
    lvnDict table;
    size_t number = 0;
    static constexpr int capFactor = 10;
    std::vector< std::vector< std::string > > numToName{ capFactor };

    void
    chkCap( int = -1 );

    std::string
    lookUpName( size_t );

    void
    clearDest( std::string );
};

class IR {
public:
    IR( std::vector< CodeBuffer >&& s ) : fnBuffers( s ) {}

    void 
    mkBasicBlocks();

    LvnMeta
    localValueNumbering( BasicBlock & );

    std::ostream &
    testLocalValueNumbering( std::string, std::string, std::ostream & );

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