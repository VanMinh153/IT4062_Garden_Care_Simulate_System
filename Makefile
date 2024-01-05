CC = gcc
#CFLAGS = -Wall -pthread -I./include -Wno-unused-variable -std=c11
CFLAGS = -Wall -pthread
PROGRAM = sensor wateringmc npkmc lamp client
DEVICE_DIR = Device
CLIENT_DIR = Client
DEPS = simulate.h utility.h tcp_socket.h session.h
OBJ = include/simulate.o include/utility.o include/tcp_socket.o include/session.o

SUPPORT = test multi sync
OTHER = menu_1
#---------------------------------
all: clean $(PROGRAM)

#---------------------------------
sensor: $(DEVICE_DIR)/sensor.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
wateringmc: $(DEVICE_DIR)/wateringmc.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
npkmc: $(DEVICE_DIR)/npkmc.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
lamp: $(DEVICE_DIR)/lamp.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(CLIENT_DIR)/client.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

#---------------------------------
multi: select_server.c
	$(CC) -o $@ $^ $(CFLAGS)

check: checkSessionHeader.c include/session.o
	$(CC) -o $@ $^ $(CFLAGS)

sync: sync_thread.c
	$(CC) -o $@ $^ $(CFLAGS)

test: hw7_tcp_test.c
	$(CC) -o $@ $^ $(CFLAGS)

#---------------------------------
ex1: Example_1.c
	$(CC) -o $@ $^ $(CFLAGS)
ex2: Example_2.c
	$(CC) -o $@ $^ $(CFLAGS)
ex3: Example_3.c
	$(CC) -o $@ $^ $(CFLAGS)
ex4: Example_4.c
	$(CC) -o $@ $^ $(CFLAGS)
ex5: Example_5.c
	$(CC) -o $@ $^ $(CFLAGS)
ex6: Example_6.c
	$(CC) -o $@ $^ $(CFLAGS)
ex7: Example_7.c
	$(CC) -o $@ $^ $(CFLAGS)
ex8: Example_8.c
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

#---------------------------------
clean:
	rm -f *.o ex* Example_? $(DEVICE_DIR)/*.o $(CLIENT_DIR)/*.o include/*.o $(PROGRAM) $(SUPPORT) $(OTHER)
