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

#ifndef BASICBLOCK_HH
#define BASICBLOCK_HH

#include <cstddef>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

// the indecies are inclusive
// rather than try to erase lines out of the vector to remove iloc code,
// replace it with something like "---" and remove them all at once when regenerating
// the iloc
class BasicBlock {
public:
    string name;

    size_t startIdx;
    size_t endIdx;

    vector< string > codeBlock;

    unordered_set< string > namesUsed;
    unordered_set< string > namesDefined;
    unordered_set< string > upExposedNames;

    size_t fanIn = 0;
};


#endif