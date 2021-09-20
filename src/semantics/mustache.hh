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