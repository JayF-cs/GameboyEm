#include "Timer.h"

uint16_t Timer::getTimer()
{
    switch (tControl & 0x3)
    {
        case 0: return 9;
        case 1: return 3;
        case 2: return 5;
        case 3: return 7;
        default: return 9;
    }
}

void Timer::setCounter(uint8_t val)
{
    tCounter = val;
}

void Timer::setModulo(uint8_t val)
{
    tModulo = val;
}

void Timer::setControl(uint8_t val)
{
    bool oldEnable = tControl & 0x04;
    int oldShift = getTimer();
    bool oldBit = oldEnable && ((internal_counter >> oldShift) & 0x1);

    tControl = val;

    bool newEnable = tControl & 0x04;
    int newShift = getTimer();
    bool newBit = newEnable && ((internal_counter >> newShift) & 0x1);

    if (oldBit && !newBit)
    {
        if (tCounter == 0xFF)
        {
            tCounter = tModulo;
            *IF |= 0x04;
        }
        else
        {
            tCounter += 1;
        }
    }

    prevBit = newBit;
}

void Timer::setDiv(uint8_t val)
{
    bool oldBit = (tControl & 0x04) && ((internal_counter >> getTimer()) & 0x1);
    internal_counter = 0;
    if (oldBit)
    {
        if (tCounter == 0xFF) { tCounter = tModulo; *IF |= 0x04; }
        else { tCounter += 1; }
    }

    prevBit = false;
}

void Timer::setBootDiv(uint16_t val)
{
    internal_counter = val;
}

uint8_t Timer::getDiv() {return internal_counter >> 8;}
uint8_t Timer::getCounter() {return tCounter;}
uint8_t Timer::getModulo() {return tModulo;}
uint8_t Timer::getControl() {return tControl;}

void Timer::connectIF(uint8_t *ifMem) {IF = ifMem;}

void Timer::tick(uint8_t cycles, bool cpu_stopped)
{
    for(int i = 0; i < cycles; i++)
    {
        internal_counter++;
        if (cpu_stopped) { internal_counter = 0; }
        bool timerEnable = (tControl & 0x04);
        int shift = getTimer();
        bool curr = timerEnable && ((internal_counter >> shift) & 0x1);

        if(prevBit && !curr)
        {
            if(tCounter == 0xFF)
            {
                tCounter = tModulo;
                *IF |= 0x04;
            }
            else
            {
                tCounter += 1;
            }
        }
        

        prevBit = curr;
    }
}