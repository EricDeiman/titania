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

#ifndef IR_HH
#define IR_HH

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <ostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

#include "basicblock.hh"
#include "cfg.hh"
#include "codebuffer.hh"
#include "mredutil.hh"

class LvnMetaDatum {
public:
    size_t valNumber;
    bool constant;
    int value;
};

class LvnMeta {
public:
    using lvnDict = unordered_map< string, LvnMetaDatum >;

    LvnMeta();

    bool
    lookUp( string, LvnMetaDatum & );

    LvnMetaDatum
    add( string, bool = false, int = 0 );

    size_t
    set( string, size_t, bool = false, int = 0 );

    string
    replaceWithI2i( string );

    ostream &
    dumpLvn( ostream & );

    bool
    renameReg( string, string );

    string
    updateRegisters( string );

    string
    lookUpName( size_t );

private:
    lvnDict table;
    size_t number = 0;
    static constexpr int capFactor = 10;
    vector< vector< string > > numToName{ capFactor };
    vector< size_t >renameRegs;

    void
    chkCap( int = -1 );

    void
    clearDest( string );
};

class IR {
public:
    IR( vector< CodeBuffer >&& s ) : fnBuffers( s ) {
        for( auto fn : fnBuffers ) {
            Cfg cfg{ fn };
            cfgs.push_back( move( cfg ) );
        }
    }

    LvnMeta
    localValueNumbering( BasicBlock & );

    ostream &
    testLocalValueNumbering( string, string, ostream & );

    ostream&
    dumpBasicBlocks( ostream &os );

    bool
    isCommumative( string );

    int
    operatorPrecedence( string );

    void
    treeHeightBalance( BasicBlock & );

    // ostream &
    // testTreeHeightBalance( string, string, ostream & );

private:
    vector< CodeBuffer >fnBuffers;
    vector< Cfg > cfgs;

    static nullstream _nullstream;

    bool
    handleBinOp( LvnMeta &, string, string, string, string, string & );

    bool
    handleLoadiOp( LvnMeta &, string, string, string & );

    vector< string >commumativeOp {
        "add", 
        "addi",
        "mult", 
        "multi",
    };

    unordered_map< string, vector< size_t > >
    buildUses( BasicBlock & );

    unordered_map< string, int >rank;

    struct thbDatum {
        string temporary;
        string instr;
        int opPriority;
    };

    // void
    // balance( thbDatum & );

    // using thbQueue = priority_queue< thbDatum,
    //         vector< thbDatum >,
    //         greater< thbDatum > >;

    // int
    // flatten( string, thbQueue & );

    // void
    // rebuild( thbQueue &, string );

};

#endif