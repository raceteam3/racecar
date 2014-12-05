CC = g++
CFLAGS = -g -O2 -Wall -D_GNU_SOURCE
ROBOT = srf08.o robot.o Adafruit_PWMServoDriver.o servo.o motor.o ADS1115.o analog_distance_sensor.o GP2Y0A02.o
SRF08_TEST = srf08.o srf08_test.o
PWM_TEST = Adafruit_PWMServoDriver.o pwm_test.o
SERVO_TEST = Adafruit_PWMServoDriver.o servo.o servo_test.o
ADS1115_TEST = ADS1115.o ADS1115_test.o
GP2Y0A02_TEST = ADS1115.o analog_distance_sensor.o GP2Y0A02.o GP2Y0A02_test.o
LDFLAGS = -lpthread -lncursesw

ifdef EMULATE
CFLAGS += -Iemulation
else
LDFLAGS += -lwiringPi
endif


all: robot

robot: $(ROBOT)
	${CC} ${CFLAGS} ${ROBOT} ${LDFLAGS} -o $@

srf08_test: $(SRF08_TEST)
	${CC} ${CFLAGS} ${SRF08_TEST} ${LDFLAGS} -o $@

pwm_test: $(PWM_TEST)
	${CC} ${CFLAGS} ${PWM_TEST} ${LDFLAGS} -o $@

servo_test: $(SERVO_TEST)
	${CC} ${CFLAGS} ${SERVO_TEST} ${LDFLAGS} -o $@

ads1115_test: $(ADS1115_TEST)
	${CC} ${CFLAGS} ${ADS1115_TEST} ${LDFLAGS} -o $@

gpy0a02_test: $(GP2Y0A02_TEST)
	${CC} ${CFLAGS} ${GP2Y0A02_TEST} ${LDFLAGS} -o $@

%.o: %.cpp *.h
	${CC} ${CFLAGS} -c $<

clean:
	rm -rf *.o *.so *.a robot srf08_test pwm_test servo_test ads1115_test gpy0a02_test
