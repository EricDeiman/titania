// Encapulate the state needed for function and hold the code generated for it.
#ifndef codebuffer_hh
#define codebuffer_hh

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "basicblock.hh"

class CodeBuffer {

public:
    CodeBuffer( std::string, bool = false );
    
    void
    writeCodeBuffer( std::vector< std::string > );

    std::string
    getFreshRegister();

    // CC stands for condition code
    std::string
    getFreshCCRegister();

    std::string
    makeLabel( std::string );

    std::vector< std::string >
    makeLabel( std::vector< std::string > );

    size_t
    valuesScopesCount( std::string );

    std::string
    valuesScopesLookup( std::string );

    std::unordered_map< std::string, std::string > 
    valuesMap;

    std::vector< std::unordered_map< std::string, std::string > >
    valuesScopes;

    std::ostream&
    dumpCodeBuffer( std::ostream &os );

    std::string
    getName();

    std::vector< std::string >*
    getBuffer();

    std::vector< BasicBlock >&
    getBlocks();

private:
    std::vector< std::string > codeBuffer;

    int registerNum = 0;
    int ccNum = 0;
    int labelSuffix = 0;

    std::string name;

    std::vector< BasicBlock >basicBlocks;
};

#endif