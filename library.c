/*
This is the standard library of functions for cminor, implemented in C.
The print statement in C-minor expects there to exist a function
for each type that can be printed.  So, the following cminor code:

x: int = 10;
b: boolean = true;
x: string = "hello";

print x, b, s;

Is effectively translated to the following C code:

print_integer(x);
print_boolean(b);
print_string(s);

And the following cminor code:

x = a ^ b;

Is effectively this C code:

x = integer_power(a,b);
*/

#include <stdio.h>

void print_integer( int x )
{
    printf("%d",x);
}

void print_string( const char *s )
{
    printf("%s",s);
}

void print_boolean( int b )
{
    printf("%s",b?"true":"false");
}

void print_character( char c )
{
    printf("%c",c);
}

int integer_power( int x, int y )
{
    int result = 1;
    while(y>0) {
        result = result * x;
        y = y -1;
    }
    return result;
}

int string_cmp( const char *s1, const char *s2 )
{
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return 0;
        ++i;
    }
    return (s1[i] == '\0' && s2[i] == '\0');
}
