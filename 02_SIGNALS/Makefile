all: send rec

send: sender.c
	gcc -O2 -Wall -Wextra -Wpedantic sender.c -o send

rec: receiver.c
	gcc -O2 -Wall -Wextra -Wpedantic receiver.c -o receive

clean:
	-rm receive send

.PHONY: all
