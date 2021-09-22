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

#include "ilocpat.hh"

string immC{ R"((-?\d+|@\w+))" };  
string regC{ R"(r(\d+|tos|arp))" };  
string ccC{ R"(cc(\d+))" };  
string insrC{ R"((\w+) +)" };  
string immOrRegC{ R"((-?\d+|@\w+|r\d+|rarp|rtos))" };  

string arw_{ " *=> *" };  
string srw_{ " *-> *" };  
string cel_{ R"((.*#.*)?)" };  
string com_{ " *, *" };  

regex
mkRX( vector< string > pattern ) { 
    string buff;

    for( auto s : pattern ) {
        buff += s;
    }

    buff += cel_;

    return regex{ buff, regex::optimize };
}
