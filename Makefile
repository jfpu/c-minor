all: cminor

cminor: main.c lex.yy.c parser.tab.c
	$(CC) -Wall main.c lex.yy.c parser.tab.c -o cminor

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

parser.tab.c parser.tab.h: parser.y
	bison parser.y

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h cminor

.PHONY: all clean
