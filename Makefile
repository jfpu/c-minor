all: cminor

cminor: main.c lex.yy.o parser.tab.o
	$(CC) main.c lex.yy.o parser.tab.o -o cminor

lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c: lexer.l token_manifest.h
	flex lexer.l

parser.tab.o: parser.tab.c
	$(CC) -c parser.tab.c

parser.tab.c: parser.y
	bison parser.y

clean:
	rm -f lex.yy.o lex.yy.c parser.tab.o parser.tab.c cminor

.PHONY: all clean
