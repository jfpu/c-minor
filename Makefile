all: cminor

# We're not compiling lex.yy.c into its own object file, as
# there are issues syncing YYSTYPE without Bison present. Currently
# the entire lex.yy.c file is being included in main.c
cminor: main.c lex.yy.c
	$(CC) main.c -o cminor

lex.yy.c: lexer.l token_manifest.h
	flex lexer.l

clean:
	rm -f lex.yy.c cminor

.PHONY: all clean
