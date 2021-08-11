
#include "ir.hh"

void
IR::mkBasicBlocks() {
    for( auto &b : fnBuffers ) {
        mkBasicBlock( b );
    }
}

std::ostream&
IR::dumpBasicBlocks( std::ostream &os ) {

    for( auto &b : fnBuffers ) {

        os << b.getName() << std::endl;
        for( auto &bb : b.getBlocks() ) {
            os << "\t" << bb.name << " start: " << bb.startIdx + 1 << ", end: " 
                << bb.endIdx + 1 << std::endl;
            for( auto s : bb.codeBlock ) {
                os << "\t\t" << s << std::endl;
            }
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
                auto label{ buffer->at( i ) };
                auto id{ label.substr( 0, label.find_first_of( ':' ) ) };
                bb.codeBlock.push_back( "jumpi @" + id );
                code.getBlocks().push_back( std::move( bb ) );

                // and start a new one
                bb.codeBlock.reserve( 1 );
                bb.codeBlock.push_back( label );
                bb.name = label;
                bb.startIdx = i;
                bb.endIdx = 0;
            }
            else {
                bb.codeBlock.push_back( buffer->at( i ) );
                bb.name = buffer->at( i );
                bb.startIdx = i;
                blockStarted = true;
            }
        }
        else if( blockStarted && isJumpInsr( buffer->at( i ) ) ) {
            bb.codeBlock.push_back( buffer->at( i ) );
            bb.endIdx = i;
            code.getBlocks().push_back( std::move( bb ) );
            bb.codeBlock.reserve( 1 );
            blockStarted = false;
            bb.startIdx = 0;
            bb.endIdx = 0;
        }
        else if( blockStarted ) {
            bb.codeBlock.push_back( buffer->at( i ) );
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