all:
		gcc -fPIC -w -c ip.c
		gcc -shared -o ip.so ip.o
		gcc main.c -o lab1 -ldl -w
