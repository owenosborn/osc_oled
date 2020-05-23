#include "CM3GPIO.h"

// for the OLED
#define SSD1306_LCDHEIGHT                 64
#define SSD1306_LCDWIDTH                  128
#define SSD1306_SETCONTRAST               0x81
#define SSD1306_DISPLAYALLON_RESUME       0xA4
#define SSD1306_DISPLAYALLON              0xA5
#define SSD1306_NORMALDISPLAY             0xA6
#define SSD1306_INVERTDISPLAY             0xA7
#define SSD1306_DISPLAYOFF                0xAE
#define SSD1306_DISPLAYON                 0xAF
#define SSD1306_SETDISPLAYOFFSET          0xD3
#define SSD1306_SETCOMPINS                0xDA
#define SSD1306_SETVCOMDETECT             0xDB
#define SSD1306_SETDISPLAYCLOCKDIV        0xD5
#define SSD1306_SETPRECHARGE              0xD9
#define SSD1306_SETMULTIPLEX              0xA8
#define SSD1306_SETLOWCOLUMN              0x00
#define SSD1306_SETHIGHCOLUMN             0x10
#define SSD1306_SETSTARTLINE              0x40
#define SSD1306_SETSTARTPAGE              0xB0
#define SSD1306_MEMORYMODE                0x20
#define SSD1306_COMSCANINC                0xC0
#define SSD1306_COMSCANDEC                0xC8
#define SSD1306_SEGREMAP                  0xA0
#define SSD1306_CHARGEPUMP                0x8D
#define SSD1306_EXTERNALVCC               0x1
#define SSD1306_SWITCHCAPVCC              0x2

// GPIO pin defs
#define OLED_DC 36               // DC pin of OLED
#define OLED_RST 37              // RST pin of OLED

// OLED init bytes
static unsigned char oled_initcode[] = {
	// Initialisation sequence
	SSD1306_DISPLAYOFF,                     // 0xAE
	SSD1306_SETLOWCOLUMN,                   // low col = 0
	SSD1306_SETHIGHCOLUMN,                  // hi col = 0
	SSD1306_SETSTARTLINE,                   // line #0
	SSD1306_SETCONTRAST,                    // 0x81
	0xCF,
	0xa1,                                   // setment remap 95 to 0 (?)
	SSD1306_NORMALDISPLAY,                  // 0xA6
	SSD1306_DISPLAYALLON_RESUME,            // 0xA4
	SSD1306_SETMULTIPLEX,                   // 0xA8
	0x3F,                                   // 0x3F 1/64 duty
	SSD1306_SETDISPLAYOFFSET,               // 0xD3
	0x0,                                    // no offset
	SSD1306_SETDISPLAYCLOCKDIV,             // 0xD5
	0xF0,                                   // the suggested ratio 0x80
	SSD1306_SETPRECHARGE,                   // 0xd9
	0xF1,
	SSD1306_SETCOMPINS,                     // 0xDA
	0x12,                                   // disable COM left/right remap
	SSD1306_SETVCOMDETECT,                  // 0xDB
	0x40,                                   // 0x20 is default?
	SSD1306_MEMORYMODE,                     // 0x20
	0x00,                                   // 0x0 act like ks0108
	SSD1306_SEGREMAP | 0x1,
	SSD1306_COMSCANDEC,
	SSD1306_CHARGEPUMP,                     //0x8D
	0x14,
	// Enabled the OLED panel
	SSD1306_DISPLAYON
};

static unsigned char oled_poscode[] = {
   	SSD1306_SETLOWCOLUMN,                   // low col = 0
	SSD1306_SETHIGHCOLUMN,                  // hi col = 0
	SSD1306_SETSTARTLINE                    // line #0
};

CM3GPIO::CM3GPIO() {
}

void CM3GPIO::init(){
    // setup GPIO, this uses actual BCM pin numbers 
    wiringPiSetupGpio();

    // OLED pins
    pinMode (OLED_DC, OUTPUT) ;
    pinMode (OLED_RST, OUTPUT) ;
    wiringPiSPISetup(0, 4*1000*1000);
    
    // reset OLED
    digitalWrite(OLED_RST,  LOW) ;
    delay(50);
    digitalWrite(OLED_RST,  HIGH) ;
    
    // initialize OLED
    digitalWrite(OLED_DC, LOW);
    wiringPiSPIDataRW(0, oled_initcode, 28);

}


void CM3GPIO::updateOLED(OledScreen &s){
    // spi will overwrite the buffer with input, so we need a tmp
    uint8_t tmp[1024];
    memcpy(tmp, s.pix_buf, 1024);
    
    digitalWrite(OLED_DC, LOW);
    wiringPiSPIDataRW(0, oled_poscode, 3);
    digitalWrite(OLED_DC, HIGH);
    wiringPiSPIDataRW(0, tmp, 1024);
}

