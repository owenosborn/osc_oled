CXX = g++

CXXFLAGS += -std=c++11

objects =  \
	main.o \
	Timer.o \
	OledScreen.o \
	UdpSocket.o \
    Socket.o \
	OSC/OSCData.o \
	OSC/OSCMatch.o \
	OSC/OSCMessage.o \
	OSC/OSCTiming.o \
	OSC/SimpleWriter.o

default :
	@echo "platform not specified"

osc_oled : CXXFLAGS += -DCM3GPIO_HW 
osc_oled : $(objects) hw_interfaces/CM3GPIO.o
	g++ -l wiringPi -o osc_oled $(objects) hw_interfaces/CM3GPIO.o

.PHONY : clean

clean :
	rm main $(objects)

# Generate with g++ -MM *.c* OSC/*.* 
OledScreen.o: OledScreen.cpp OledScreen.h fonts.h 
Socket.o: Socket.cpp Socket.h
Timer.o: Timer.cpp Timer.h
UdpSocket.o: UdpSocket.cpp UdpSocket.h Socket.h
main.o: main.cpp OSC/OSCMessage.h OSC/OSCData.h OSC/OSCTiming.h \
  OSC/SimpleWriter.h UdpSocket.h Socket.h \
  OledScreen.h Timer.h
OSCData.o: OSC/OSCData.cpp OSC/OSCData.h OSC/OSCTiming.h
OSCMatch.o: OSC/OSCMatch.c OSC/OSCMatch.h
OSCMessage.o: OSC/OSCMessage.cpp OSC/OSCMessage.h OSC/OSCData.h \
  OSC/OSCTiming.h OSC/SimpleWriter.h OSC/OSCMatch.h
OSCTiming.o: OSC/OSCTiming.cpp OSC/OSCTiming.h
SimpleWriter.o: OSC/SimpleWriter.cpp OSC/SimpleWriter.h
