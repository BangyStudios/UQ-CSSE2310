FLAGS = -g -Wall -pedantic -std=gnu99 -pthread
INCLUDE = -I/local/courses/csse2310/include -L/local/courses/csse2310/lib
LINKCLIENT = -lcsse2310a3 -lcsse2310a4
LINKSERVER = -lcsse2310a4 -ltinyexpr -lm

all: intclient intserver

intclient: intclient.c intcommon.h
		gcc $(FLAGS) $(INCLUDE) $(LINKCLIENT) intclient.c -o intclient
		chmod +x intclient

intserver: intserver.c intcommon.h
		gcc $(FLAGS) $(INCLUDE) $(LINKSERVER) intserver.c -o intserver
		chmod +x intserver

clean:
		rm -f intclient intserver core.* vgcore.*
