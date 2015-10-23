all: cminor

cminor: main.c lex.yy.o lex.yy.h parser.tab.o parser.tab.h decl.o expr.o param_list.o stmt.o type.o utility.o
	$(CC) -Wall main.c lex.yy.o parser.tab.o decl.o expr.o param_list.o stmt.o type.o utility.o -o cminor

%.o: %.c
	$(CC) -Wall -c $< -o $@

lex.yy.c lex.yy.h: lexer.l parser.tab.h
	flex lexer.l

parser.tab.c parser.tab.h: parser.y
	bison parser.y --report=state

clean:
	rm -f lex.yy.c lex.yy.h parser.tab.c parser.tab.h parser.output *.o cminor

.PHONY: all clean
