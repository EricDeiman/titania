LIBDIR = ../lib
INCDIR = ../include

OBJS = type.o elaboration.o

CXXFLAGS += -std=c++17 -g -I $(INCDIR) -I $(INCDIR)/antlr4-runtime -L $(LIBDIR)

.DEFAULT_GOAL := doit

type.o : type.cc type.hh $(INCDIR)/titaniaBaseVisitor.h $(INCDIR)/titaniaParser.h \
$(INCDIR)/titaniaLexer.h

elaboration.o : elaboration.cc elaboration.hh $(INCDIR)/titaniaBaseVisitor.h $(INCDIR)/titaniaParser.h \
$(INCDIR)/titaniaLexer.h

doit : type.o elaboration.o $(LIBDIR)/libtitaniaParser.a
	g++ $(CXXFLAGS) $(OBJS) -l titaniaParser -l antlr4-runtime -o $@

.PHONY : clean scrub

clean :
	rm -f *.o

scrub : clean
	rm -f doit
