# 28 Apr 2002; nss_userdir

library: userdir-pwent.o util.o
	gcc -shared -o libnss_userdir.so.2 -Wl,-soname,libnss_userdir.so.2 userdir-pwent.o util.o

userdir-pwent.o: userdir-pwent.c
	gcc -fPIC -Wall -c userdir-pwent.c

util.o: util.c
	gcc -fPIC -Wall -c util.c
