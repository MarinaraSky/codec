CC=gcc
PROGRAMS=encode decode
CFLAGS = $(CF) -Wall -Wextra -Wpedantic -Wstack-usage=1024 -Wwrite-strings -Wfloat-equal -Waggregate-return -Winline 
DEBUG_FLAGS = -g3

codec: $(PROGRAMS)

encode:
	$(CC) $(CFLAGS) -o encode src/encode.c

decode: src/decode.c
	$(CC) $(CFLAGS) -o decode src/decode.c

clean:
	@rm -f $(PROGRAMS) *.o


