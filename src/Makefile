CC = g++
CFLAGS = -g -O2 -Wall -D_GNU_SOURCE
ROBOT = srf08.o robot.o
SRF08_TEST = srf08.o srf08_test.o
LDFLAGS = -lpthread -lwiringPi


all: robot

robot: $(ROBOT)
	${CC} ${CFLAGS} ${ROBOT} ${LDFLAGS} -o $@

srf08_test: $(SRF08_TEST)
	${CC} ${CFLAGS} ${ROBOT} ${LDFLAGS} -o $@

%.o: %.cpp *.h
	${CC} ${CFLAGS} -c $<

clean:
	rm -rf *.o *.so *.a robot srf08_test
