FLAGS = -std=c99 -Wall -Wextra -g

all: send recv

link_emulator/lib.o:
	$(MAKE) -C link_emulator

send: send.o link_emulator/lib.o utils.o queue.o
	gcc $(FLAGS) send.o utils.o queue.o link_emulator/lib.o -o send

recv: recv.o link_emulator/lib.o utils.o queue.o
	gcc $(FLAGS) recv.o utils.o queue.o link_emulator/lib.o -o recv

.c.o:
	gcc -Wall -g -c $?

clean:
	$(MAKE) -C link_emulator clean
	-rm -f *.o send recv
