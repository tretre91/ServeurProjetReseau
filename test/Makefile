bin/main: main.c bin/bluetooth_server.o bin/broadcast.o
	gcc main.c bin/bluetooth_server.o bin/broadcast.o -o bin/main

exec: bin/main
	./bin/main

bin/bluetooth_server.o:
	mkdir -p bin
	git clone https://github.com/Juroupi/bluetooth-server.git
	gcc -c bluetooth-server/bluetooth_server.c -o bin/bluetooth_server.o

bin/broadcast.o:
	mkdir -p bin
	git clone https://github.com/Juroupi/broadcast-messages.git
	gcc -c broadcast-messages/broadcast.c -o bin/broadcast.o