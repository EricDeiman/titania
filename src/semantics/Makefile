LIBDIR = ../lib
INCDIR = ../include

OBJS = type.o elaboration.o codebuffer.o mustache.o ir.o

CXXFLAGS += -std=c++17 -g -I $(INCDIR) -I $(INCDIR)/antlr4-runtime -L $(LIBDIR)

.DEFAULT_GOAL := tc

type.o : type.cc type.hh $(INCDIR)/titaniaBaseVisitor.h $(INCDIR)/titaniaParser.h \
$(INCDIR)/titaniaLexer.h

elaboration.o : elaboration.cc elaboration.hh $(INCDIR)/titaniaBaseVisitor.h $(INCDIR)/titaniaParser.h \
$(INCDIR)/titaniaLexer.h

codebuffer.o : codebuffer.cc codebuffer.hh

mustache.o : mustache.cc mustache.hh

ir.o : ir.cc ir.hh

tc : $(OBJS) $(LIBDIR)/libtitaniaParser.a
	g++ $(CXXFLAGS) $(OBJS) -l titaniaParser -l antlr4-runtime -o $@

.PHONY : clean scrub

clean :
	rm -f *.o

scrub : clean
	rm -f tc
