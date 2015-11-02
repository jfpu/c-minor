FLAGS=-Wall -g

all: cminor

cminor: main.o lex.yy.o parser.tab.o decl.o expr.o param_list.o stmt.o type.o utility.o
	$(CC) $(FLAGS) main.o lex.yy.o parser.tab.o decl.o expr.o param_list.o stmt.o type.o utility.o -o cminor

main.o: main.c parser.tab.h lex.yy.h
	$(CC) $(FLAGS) -c main.c -o $@

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

lex lex.yy.c lex.yy.h: lexer.l parser.tab.h
	flex lexer.l

parse parser.tab.c parser.tab.h: parser.y
	bison parser.y --report=state

clean:
	rm -f lex.yy.c lex.yy.h parser.tab.c parser.tab.h parser.output *.o cminor

.PHONY: all clean lex parse
