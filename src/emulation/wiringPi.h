#define OUTPUT 0
#define INPUT 0
#define PUD_UP 0
#define HIGH 0
#define LOW 0

inline void wiringPiSetupGpio() { }
inline void pinMode(int, int) { }
inline void digitalWrite(int, int) { }
inline int digitalRead(int) { return 0; }
inline void pullUpDnControl(int, int) { }
