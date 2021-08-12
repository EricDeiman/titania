
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "codebuffer.hh"
#include "ir.hh"

int
main( int argc, char **argv ) {

    CodeBuffer cb{ "!" };
    std::ifstream is{ argv[ 1 ] };

    while( !is.eof() ) {
        char buffer[ 1024 ];
        is.getline( buffer, sizeof( buffer ) );
        cb.writeCodeBuffer( { buffer } );
    }

    std::vector< CodeBuffer >vcb;
    vcb.push_back( cb );

    IR ir{ std::move( vcb ) };
    ir.mkBasicBlocks();
    ir.testLocalValueNumbering( "!", "!", std::cout );
    // ir.dumpBasicBlocks( std::cout );


    return 0;
}