#ifndef IR_HH
#define IR_HH

#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "basicblock.hh"
#include "codebuffer.hh"
#include "mredutil.hh"

class LvnMetaDatum {
public:
    size_t valNumber;
    bool constant;
    int value;
};

class LvnMeta {
public:
    using lvnDict = std::unordered_map< std::string, LvnMetaDatum >;

    LvnMeta();

    bool
    lookUp( std::string, LvnMetaDatum & );

    LvnMetaDatum
    add( std::string, bool = false, int = 0 );

    size_t
    set( std::string, size_t, bool = false, int = 0 );

    std::string
    replaceWithI2i( std::string );

    std::ostream &
    dumpLvn( std::ostream & );

    void
    renameReg( std::string, std::string );

    std::string
    updateRegisters( std::string );

    std::string
    lookUpName( size_t );

private:
    lvnDict table;
    size_t number = 0;
    static constexpr int capFactor = 10;
    std::vector< std::vector< std::string > > numToName{ capFactor };
    std::vector< size_t >renameRegs;

    void
    chkCap( int = -1 );

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

    bool
    isCommumative( std::string );

private:
    std::vector< CodeBuffer >fnBuffers;

    void
    mkBasicBlock( CodeBuffer & );

    bool
    isJumpInsr( std::string );

    static nullstream _nullstream;

    bool
    handleBinOp( LvnMeta &, std::string, std::string, std::string, std::string, std::string & );

    bool
    handleLoadiOp( LvnMeta &, std::string, std::string, std::string & );

    std::vector< std::string >commumativeOp {
        "add", 
        "addi",
        "mult", 
        "multi",
    };
};

#endif