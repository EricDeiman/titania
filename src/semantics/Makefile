#$
#$  The Titania programming language
#$  Copyright 2021 Eric J. Deiman
#$
#$  This file is part of the Titania programming language.
#$  The Titania programming language is free software: you can redistribute it
#$  and/ormodify it under the terms of the GNU General Public License as published by the
#$  Free Software Foundation, either version 3 of the License, or (at your option) any
#$  later version.
#$  
#$  The Titania programming language is distributed in the hope that it will be useful,
#$  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#$  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#$  You should have received a copy of the GNU General Public License along with the
#$  Titania programming language. If not, see <https://www.gnu.org/licenses/>
#$

LIBDIR = ../lib
INCDIR = ../include

CXX = clang++

OBJS = type.o elaboration.o codebuffer.o mustache.o ir.o cfg.o

CXXFLAGS += -std=c++17 -g -I $(INCDIR) -I $(INCDIR)/antlr4-runtime -L $(LIBDIR)

.DEFAULT_GOAL := tc

type.o : type.cc type.hh $(INCDIR)/titaniaBaseVisitor.h $(INCDIR)/titaniaParser.h \
$(INCDIR)/titaniaLexer.h

elaboration.o : elaboration.cc elaboration.hh codebuffer.cc mustache.cc ir.cc $(INCDIR)/titaniaBaseVisitor.h $(INCDIR)/titaniaParser.h \
$(INCDIR)/titaniaLexer.h

codebuffer.o : codebuffer.cc codebuffer.hh

cfg.o : cfg.cc cfg.hh

mustache.o : mustache.cc mustache.hh

testlvn : testlvn.cc codebuffer.cc ir.cc cfg.cc

ir.o : ir.cc ir.hh

tc : $(OBJS) $(LIBDIR)/libtitaniaParser.a
	$(CXX) $(CXXFLAGS) $(OBJS) -l titaniaParser -l antlr4-runtime -o $@

.PHONY : clean scrub

clean :
	rm -f *.o

scrub : clean
	rm -f tc
