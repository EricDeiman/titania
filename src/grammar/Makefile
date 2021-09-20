#
#  The Titania programming language
#  Copyright 2021 Eric J. Deiman
#
#  This file is part of the Titania programming language.
#  The Titania programming language is free software: you can redistribute it
#  and/ormodify it under the terms of the GNU General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your option) any
#  later version.
#  
#  The Titania programming language is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License along with the
#  Titania programming language. If not, see <https://www.gnu.org/licenses/>
#

ANTLR = java -Xmx500M org.antlr.v4.Tool
ANTLRFLAGS = -Dlanguage=Cpp -no-listener -visitor
CXXFLAGS = -std=c++17 -g -I ../include/antlr4-runtime/ -L ../lib -L .
P = titania
CP = cp

.DEFAULT_GOAL := lib$(P)Parser.a

$(P)BaseVisitor.cpp $(P)BaseVisitor.h $(P)Visitor.cpp $(P)Visitor.h $(P)Parser.cpp \
$(P)Parser.h $(P)Lexer.cpp $(P)Lexer.h : $(P).g4
	$(ANTLR) $(ANTLRFLAGS) $?

$(P)BaseVisitor.o : $(P)BaseVisitor.cpp $(P)BaseVisitor.h $(P)Visitor.h $(P)Parser.h
$(P)Visitor.o : $(P)Visitor.cpp $(P)Visitor.h $(P)Parser.h
$(P)Lexer.o : $(P)Lexer.cpp $(P)Lexer.h
$(P)Parser.o : $(P)Parser.cpp $(P)Parser.h

lib$(P)Parser.a : $(P)BaseVisitor.o $(P)Visitor.o $(P)Lexer.o $(P)Parser.o
	$(AR) $(ARFLAGS) $@ $?
	$(CP) $@ ../lib
	$(CP) *.h ../include

.PHONY : clean scrub

clean : 
	rm -f *.o *.cpp *.h *.tokens

scrub : clean
	rm -f *.a