#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/stat.h>

#include "OSC/OSCMessage.h"
#include "OSC/SimpleWriter.h"
#include "UdpSocket.h"
#include "OledScreen.h"
#include "Timer.h"

// include hardware interface
#include "hw_interfaces/CM3GPIO.h"

// buffer for sending OSC messages 
SimpleWriter oscBuf;

// hardware interface controls
CM3GPIO hw;

// this program receives on 4001
UdpSocket udpSock(4001);

// screens
OledScreen oled[16];

// new style graphics messages
void gClear(OSCMessage &msg);
void gSetPixel(OSCMessage &msg);
void gFillArea(OSCMessage &msg);
void gCircle(OSCMessage &msg);
void gFilledCircle(OSCMessage &msg);
void gLine(OSCMessage &msg);
void gBox(OSCMessage &msg);
void gInvert(OSCMessage &msg);
void gInvertArea(OSCMessage &msg);
void gCharacter(OSCMessage &msg);
void gPrintln(OSCMessage &msg);
void gWaveform(OSCMessage &msg);
void gFrame(OSCMessage &msg);
void gFlip(OSCMessage &msg);

int main(int argc, char* argv[]) {
    printf("build date " __DATE__ "   " __TIME__ "/n");
    uint32_t seconds = 0;
    char udpPacketIn[2048];

    Timer screenFpsTimer;

    screenFpsTimer.reset();

    udpSock.setDestination(4000, "localhost");
    OSCMessage msgIn;

    hw.init();

    for(int i = 0; i < 1024; i++) {
        oled[0].pix_buf[i] = i;
    }
    hw.updateOLED(oled[0]);


    for (;;) {
        // receive udp osc messages
        int len = udpSock.readBuffer(udpPacketIn, 2048, 0);
        if (len > 0) {
            msgIn.empty();
            for (int i = 0; i < len; i++) {
                msgIn.fill(udpPacketIn[i]);
            }
            if (!msgIn.hasError()) {
                //char buf[128];
                //msgIn.getAddress(buf,0,128);
                //printf("osc message received %s %i\n",buf,msgIn.size());
                // or'ing will do lazy eval, i.e. as soon as one succeeds it will stop
                bool processed =
                    msgIn.dispatch("/oled/gClear", gClear, 0)
                    || msgIn.dispatch("/oled/gSetPixel", gSetPixel, 0)
                    || msgIn.dispatch("/oled/gFillArea", gFillArea, 0)
                    || msgIn.dispatch("/oled/gCircle", gCircle, 0)
                    || msgIn.dispatch("/oled/gFilledCircle", gFilledCircle, 0)
                    || msgIn.dispatch("/oled/gLine", gLine, 0)
                    || msgIn.dispatch("/oled/gBox", gBox, 0)
                    || msgIn.dispatch("/oled/gInvert", gInvert, 0)
                    || msgIn.dispatch("/oled/gCharacter", gCharacter, 0)
                    || msgIn.dispatch("/oled/gPrintln", gPrintln, 0)
                    || msgIn.dispatch("/oled/gWaveform", gWaveform, 0)
                    || msgIn.dispatch("/oled/gFrame", gFrame, 0)
                    || msgIn.dispatch("/oled/gInvertArea", gInvertArea, 0)
                    || msgIn.dispatch("/oled/gFlip", gFlip, 0)
                    ;
                if (!processed) {
                    char buf[128];
                    msgIn.getAddress(buf,0,128);
                    fprintf(stderr, "unrecognised osc message received %s %i\n",buf,msgIn.size());
                }
            }
            else {
                fprintf(stderr, "osc message has error \n ");
            }
            msgIn.empty();
        }

        usleep(2000);
    } // for;;
}

// graphics messages
// and require sending gFlip to cause screen update
void gClear(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1)) {
        int screen_num = msg.getInt(0) & 0xF;
        if (msg.getInt(1) == 1) oled[screen_num].clear();
    }
}

void gInvert(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1)) {
        int screen_num = msg.getInt(0) & 0xF;
        if (msg.getInt(1) == 1) oled[screen_num].invert_screen();
    }
}

void gSetPixel(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) ) {
        int screen_num = msg.getInt(0) & 0xF;
        oled[screen_num].put_pixel(msg.getInt(3), msg.getInt(1), msg.getInt(2));
    }
}
void gFillArea(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        int screen_num = msg.getInt(0) & 0xF;
        oled[screen_num].fill_area(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
    }
}

void gInvertArea(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4)) {
        int screen_num = msg.getInt(0) & 0xF;
        oled[screen_num].invert_area(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
    }
}

void gFlip(OSCMessage &msg) {
    if (msg.isInt(0)) {
        int screen_num = msg.getInt(0) & 0xF;
        hw.updateOLED(oled[screen_num]);
    }
}

void gWaveform(OSCMessage &msg) {
    uint8_t tmp[132];
    int len = 0;
    int i;
    if (msg.isInt(0) && msg.isBlob(1)) {
        int screen_num = msg.getInt(0) & 0xF;
        len = msg.getBlob(1, tmp, 132);
        // only if we got 128 values (len and tmp includes the 4 size bytes of blob)
        if (len == 132) {
            // draw 127 connected lines
            for (i = 1; i < 128; i++) {
                oled[screen_num].draw_line(i - 1, tmp[i + 3], i, tmp[i + 4], 1);
            }
        }
    }
}

void gFrame(OSCMessage &msg) {
    uint8_t tmp[1028]; 
    int len = 0;
    int i;
    if (msg.isInt(0) && msg.isBlob(1)) {
        int screen_num = msg.getInt(0) & 0xF;
        len = msg.getBlob(1, tmp, 1028);
        // only if we got 1024 values (len and tmp includes the 4 size bytes of blob)
        if (len == 1028) {
		// copy it right to screen buffer
        	memcpy(oled[screen_num].pix_buf, tmp + 4, 1024);
        }
    }
}

void gCircle(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4)) {
        int screen_num = msg.getInt(0) & 0xF;
        oled[screen_num].draw_circle(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
    }
}

void gFilledCircle(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4)) {
        int screen_num = msg.getInt(0) & 0xF;
        oled[screen_num].draw_filled_circle(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
    }
}

void gLine(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        int screen_num = msg.getInt(0) & 0xF;
        oled[screen_num].draw_line(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
    }
}

void gPrintln(OSCMessage &msg) {

    char str[256];
    char line[256];
    int i;
    int x, y, height, color, screenNum;
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4)) {
        int screen_num = msg.getInt(0) & 0xF;
        i = 0;
        screenNum = msg.getInt(i++);
        x = msg.getInt(i++);
        y = msg.getInt(i++);
        height = msg.getInt(i++);
        color = msg.getInt(i++);
        // since there are no strings in pd, the line message will be made of different types
        // cat the line together, then throw it up on the patch screen
        line[0] = 0;
        while(i < msg.size()) {
            if (msg.isString(i)) {
                msg.getString(i, str, 256);
                strcat(line, str);
                strcat(line, " ");
            }
            else if (msg.isFloat(i)) {
                sprintf(str, "%g ", msg.getFloat(i));
                strcat(line, str);
            }
            else if (msg.isInt(i)) {
                sprintf(str, "%d ", msg.getInt(i));
                strcat(line, str);
            }
            i++;
        }
        oled[screen_num].println(line, x, y, height, color);
    }
}

void gBox(OSCMessage &msg) {
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        int screen_num = msg.getInt(0) & 0xF;
        oled[screen_num].draw_box(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4), msg.getInt(5));
    }
}
void gCharacter(OSCMessage &msg) {
    int size = 8;
    if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3) && msg.isInt(4) && msg.isInt(5)) {
        int screen_num = msg.getInt(0) & 0xF;
        size = msg.getInt(5);
        if (size == 8) oled[screen_num].put_char_small(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        else if (size == 16) oled[screen_num].put_char_arial16(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        else if (size == 24) oled[screen_num].put_char_arial24(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
        else if (size == 32) oled[screen_num].put_char_arial32(msg.getInt(1), msg.getInt(2), msg.getInt(3), msg.getInt(4));
    }
}

