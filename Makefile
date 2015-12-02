FLAGS=-Wall -g
OBJS=lex.yy.o parser.tab.o decl.o expr.o param_list.o stmt.o type.o utility.o symbol.o scope.o hash_table.o register.o

all: cminor library.o

cminor: main.o $(OBJS)
	$(CC) $(FLAGS) main.o $(OBJS) -o cminor -lm

main.o: main.c parser.tab.h lex.yy.h
	$(CC) $(FLAGS) -c main.c -o $@

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

lex lex.yy.c lex.yy.h: lexer.l parser.tab.h
	flex lexer.l

parse parser.tab.c parser.tab.h: parser.y
	bison parser.y --report=state

clean: wipeass
	rm -f lex.yy.c lex.yy.h parser.tab.c parser.tab.h parser.output *.o *.s *.out cminor

wipeass:
	rm -f ./test_compile/*.s ./test_compile/*.out

.PHONY: all clean lex parse wipeass
