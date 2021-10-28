all: send rec

send: sender/sender.c
	gcc -O2 -Wall -Wextra sender/sender.c -o send

rec: receiver/receiver.c
	gcc -O2 -Wall -Wextra receiver/receiver.c -o receive

clean:
	-rm receive send

.PHONY: all
