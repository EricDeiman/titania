
#include "ir.hh"

void
IR::mkBasicBlocks() {
    for( auto &b : fnBuffers ) {
        mkBasicBlock( b );
    }
}

std::ostream&
IR::dumpBasicBlocks( std::ostream &os ) {

    for( auto b : fnBuffers ) {

        os << b.getName() << std::endl;
        for( auto bb : b.getBlocks() ) {
            os << "\tstart: " << bb.startIdx + 1 << ", end: " << bb.endIdx + 1 << std::endl;
        }
    }

    return os;
}

void
IR::mkBasicBlock( CodeBuffer &code ) {
    auto buffer{ code.getBuffer() };
    BasicBlock bb;
    bool blockStarted = false;

    for( auto i = 0; i < buffer->size(); i++ ) {
        if( buffer->at( i ).find_first_of( ':' ) != std::string::npos ) {
            if( blockStarted ) {
                // finish the existing block
                bb.endIdx = i - 1;
                code.getBlocks().push_back( std::move( bb ) );

                // and start a new one
                bb.startIdx = i;
                bb.endIdx = 0;
            }
            else {
                bb.startIdx = i;
                blockStarted = true;
            }
        }
        else if( blockStarted && isJumpInsr( buffer->at( i ) ) ) {
            bb.endIdx = i;
            code.getBlocks().push_back( std::move( bb ) );
            blockStarted = false;
            bb.startIdx = 0;
            bb.endIdx = 0;
        }
    }
}

bool
IR::isJumpInsr( std::string s ) {

    std::vector< std::string > jumpInsrs { { "cbrneq", "cbreq", "jump", "jumpi", "hlt", 
        "call", "ret", "showi", "showb", "shows" } };

    for( auto i : jumpInsrs ) {
        auto x = s.find( i );
        if( x != std::string::npos && x == 0 ) {
            return true;
        }
    }

    return false;
}