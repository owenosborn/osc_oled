#ifndef CM3GPIO_H
#define CM3GPIO_H

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringShift.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "../OledScreen.h"

class CM3GPIO
{
    public:
        CM3GPIO();
        void init();
        void updateOLED(OledScreen &s);

    private:        
};


#endif
