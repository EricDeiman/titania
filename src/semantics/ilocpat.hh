/*
  The Titania programming language
  Copyright 2021 Eric J. Deiman

  This file is part of the Titania programming language.
  The Titania programming language is free software: you can redistribute it
  and/ormodify it under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your option) any
  later version.
  
  The Titania programming language is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along with the
  Titania programming language. If not, see <https://www.gnu.org/licenses/>
*/

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