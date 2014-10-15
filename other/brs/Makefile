CFLAGS = -O2

default: brup brdown

brup: brup.flex
	flex brup.flex
	gcc $(CFLAGS) -o brup lex.yy.c -lfl
	
brdown: brdown.flex
	flex brdown.flex
	gcc $(CFLAGS) -o brdown lex.yy.c -lfl
	
clean:
	rm -f brup brdown lex.yy.c lex.yy.c
