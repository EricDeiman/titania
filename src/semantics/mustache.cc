
#include <algorithm>

#include <iostream>

#include "mustache.hh"

mustache::mustache( std::vector< std::string > _data ) : data( _data ) {
    for( auto s : data ) {
        findReplacements( s ) ;
    }
}

void
mustache::append( std::string next ) {
    data.push_back( next );
    findReplacements( next );
}

bool
mustache::populate( dictionary replace, CodeBuffer &destination ) {

    // sort the keys of the dictionary
    std::vector< std::string >keys{ getKeys( replace ) };
    std::sort( keys.begin(), keys.end() );

    // make sure (at least) the needed keys are in the dictonary
    for( auto k : neededKeys ) {
        if( !std::binary_search( keys.begin(), keys.end(), k ) ) {
            // k is needed, but not in the replacement set
            destination.writeCodeBuffer( { "#!!! replace set missing key ", k } );
            return false;
        }
    }

    for( auto s : data ) {
        std::string temp{ s };
        if( std::regex_search( temp, pattern ) ) {  // at least one replacement is needed

            while( temp.find_first_of( '{' ) != std::string::npos ) {
                auto first{  temp.find_first_of( '{' ) };
                auto second{ temp.find_first_of( '}' ) };
                auto key{ temp.substr( first + 1, second - first - 1 ) };
    
                temp.replace( first, second - first + 1, replace[ key ] );
            }
        }

        destination.writeCodeBuffer( { temp } );
    }

    return true;
}

std::vector< std::string >
mustache::getKeys( dictionary map ) {
    std::vector< std::string >temp{ map.size() };

    std::transform( map.begin(), map.end(), temp.begin(), []( auto p ){ return p.first; } );

    return temp;
}

void
mustache::findReplacements( std::string target ) {

    // at least one replacement neede for the target
    if( std::regex_search( target, pattern ) ) {
        std::sregex_token_iterator end{};
        std::sregex_token_iterator p{ target.begin(), target.end(), pattern };
        for( ; p != end; p++ ) {
            std::string s{ *p };
            auto key{ s.substr( 1, s.size() - 2 ) };
            if( !binary_search( neededKeys.begin(), neededKeys.end(), key ) ) {
                neededKeys.push_back( key );
                std::sort( neededKeys.begin(), neededKeys.end() );
            }
        }
    }
}