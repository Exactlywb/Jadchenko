all: send rec

send: sender/sender.c
	gcc sender/sender.c -o send

rec: receiver/receiver.c
	gcc receiver/receiver.c -o receive

clean:
	-rm receive send

.PHONY: all
