OBJS = jrbase.o jrhandler.o
OUT = jobrunner
FLAGS = -g -c -Wall -pedantic -std=gnu99
LINK = -I/local/courses/csse2310/include -L/local/courses/csse2310/lib -lcsse2310a3

all: $(OBJS)
	gcc -g $(LINK) $(OBJS) -o $(OUT)

jrbase.o: jrbase.c jrcommon.h  
	gcc $(FLAGS) $(LINK) jrbase.c

jrhandler.o: jrhandler.c jrcommon.h
	gcc $(FLAGS) jrhandler.c

clean:
	rm -f $(OBJS) $(OUT)


