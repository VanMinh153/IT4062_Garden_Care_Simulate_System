CC = gcc
#CFLAGS = -Wall -pthread -I./include -Wno-unused-variable -std=c11
CFLAGS = -Wall -pthread
PROGRAM = sensor watering fertilizing lamp client
DEVICE_DIR = Device
CLIENT_DIR = Client
EX_DIR = Temp
DEPS = simulate.h utility.h tcp_socket.h session.h
OBJ = include/simulator.o include/utility.o include/tcp_socket.o include/session.o

SUPPORT = test multi sync
OTHER = menu_1
#---------------------------------
all: clean $(PROGRAM)

#---------------------------------
sensor: $(DEVICE_DIR)/sensor.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
watering: $(DEVICE_DIR)/watering.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
fertilizing: $(DEVICE_DIR)/fertilizing.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
lamp: $(DEVICE_DIR)/lamp.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(CLIENT_DIR)/client.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
#---------------------------------
sensor_test: $(DEVICE_DIR)/sensor_test.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
watering_test: $(DEVICE_DIR)/watering_test.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
fertilizing_test: $(DEVICE_DIR)/fertilizing_test.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
lamp_test: $(DEVICE_DIR)/lamp_test.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

client_test: $(CLIENT_DIR)/client_test.o $(OBJ)
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
ex1: $(EX_DIR)/example_1.c
	$(CC) -o $@ $^ $(CFLAGS)
ex2: $(EX_DIR)/example_2.c
	$(CC) -o $@ $^ $(CFLAGS)
ex3: $(EX_DIR)/example_3.c
	$(CC) -o $@ $^ $(CFLAGS)
ex4: $(EX_DIR)/example_4.c
	$(CC) -o $@ $^ $(CFLAGS)
ex5: $(EX_DIR)/example_5.c
	$(CC) -o $@ $^ $(CFLAGS)
ex6: $(EX_DIR)/example_6.c
	$(CC) -o $@ $^ $(CFLAGS)
ex7: $(EX_DIR)/example_7.c
	$(CC) -o $@ $^ $(CFLAGS)
ex8: $(EX_DIR)/example_8.c
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

#---------------------------------
clean:
	rm -f *.o ex* $(EX_DIR)/example_? $(DEVICE_DIR)/*.o $(CLIENT_DIR)/*.o include/*.o $(PROGRAM) $(SUPPORT) $(OTHER)
