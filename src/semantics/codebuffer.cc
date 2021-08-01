#include "codebuffer.hh"

CodeBuffer::CodeBuffer( bool global ) {

    // since zero is false, it will be handy to have in a register
    auto reg = getFreshRegister();

    if( global ) {
        codeBuffer.push_back( "loadi 0 => " + reg );
    }

    valuesScopes.push_back( valuesMap );

    valuesScopes.back()[ "0" ] = reg;
    valuesScopes.back()[ "false" ] = reg;
}

std::string
CodeBuffer::getFreshRegister() {
    return "r" + std::to_string( registerNum++ );
}

std::string
CodeBuffer::getFreshCCRegister() {
    return "cc" + std::to_string( ccNum++ );
}

std::string
CodeBuffer::makeLabel( std::string base ) {
    return base + std::to_string( labelSuffix++ );
}

std::vector< std::string >
CodeBuffer::makeLabel( std::vector< std::string > prefixes ) {
    std::vector< std::string >labels;

    for( auto p : prefixes ) {
        labels.push_back( p + std::to_string( labelSuffix ) );
    }

    labelSuffix++;

    return std::move( labels );
}

void
CodeBuffer::writeCodeBuffer( std::vector< std::string > data ) {

    std::string buffer;
    for( auto s : data ) {
        buffer += s;
    }

    codeBuffer.push_back( buffer );
}

size_t
CodeBuffer::valuesScopesCount( std::string id ) {

    for( auto m : valuesScopes ) {
        if( m.count( id ) > 0 ) {
            return m.count( id );
        }
    }

    return 0;
}

std::string
CodeBuffer::valuesScopesLookup( std::string id ) {

    std::string tmp;

    for( auto m : valuesScopes ) {
        if( m.count( id ) > 0 ) {
            tmp = m[ id ];
        }
    }

    return tmp;
}

std::ostream&
CodeBuffer::dumpCodeBuffer( std::ostream &os ) {
    for( auto s : codeBuffer ) {
        os << s << std::endl;
    }

    os << std::endl;

    return os;
}

