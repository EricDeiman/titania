#ifndef mustache__hh
#define mustache__hh

#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "codebuffer.hh"

using dictionary = std::unordered_map< std::string, std::string >;

class mustache {
public:
    mustache( std::vector< std::string > );

    void
    append( std::string );

    bool
    populate( dictionary, CodeBuffer& );

private:
    std::vector< std::string > data;
    std::vector< std::string > neededKeys;

    std::string basePattern{ R"(\{.+?\})" };
    std::regex pattern{ basePattern, std::regex::ECMAScript | std::regex::optimize };

    void
    findReplacements( std::string );

    std::vector< std::string >
    getKeys( dictionary );

};

#endif