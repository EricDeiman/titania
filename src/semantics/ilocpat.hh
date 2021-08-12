#ifndef ILOCPAT_HH
#define ILOCPAT_HH

#include <string>
#include <regex>

// the NOLINT comments are to supress a particular warning from clang tidy

std::string immC{ R"((-?\d+|@\w+))" };  // an immediate can be a number or @identifier  // NOLINT(misc-definitions-in-headers)
std::string regC{ R"(r(\d+|tos|arp))" };  // NOLINT(misc-definitions-in-headers)
std::string ccC{ R"(cc(\d+))" };  // NOLINT(misc-definitions-in-headers)
std::string insrC{ R"((\w+) +)" };  // NOLINT(misc-definitions-in-headers)
std::string immOrRegC{ R"((-?\d+|@\w+|r\d+|rarp|rtos))" };  // NOLINT(misc-definitions-in-headers)

std::string arw_{ " *=> *" };  // NOLINT(misc-definitions-in-headers)
std::string srw_{ " *-> *" };  // NOLINT(misc-definitions-in-headers)
std::string cel_{ R"((.*#.*)?)" };  // comment to end of line  // NOLINT(misc-definitions-in-headers)
std::string com_{ " *, *" };  // NOLINT(misc-definitions-in-headers)

std::regex
mkRX( std::vector< std::string > pattern ) {  // NOLINT(misc-definitions-in-headers)
    std::string buff;

    for( auto s : pattern ) {
        buff += s;
    }

    buff += cel_;

    return std::regex{ buff, std::regex::optimize };
}


#endif