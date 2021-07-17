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