#include <iostream>
#include "cpu.h"
#include "Bus.h"
#include "registers.h"

CPU::CPU() {}

bool CPU::imeGetter()
{
    return ime;
}

bool CPU::cpuStoppedGetter()
{
    return cpu_stopped;
}

void CPU::connectBus(Bus* b) {
    this->b = b;
}

void CPU::connectRegisters(Registers* reg) {
    this->r = reg;
}

//Helper function to read a 16 bit value from memory
uint16_t CPU::fetch_16(){

    uint16_t val = b->read_16(r->PC + 1);
    return val;

}

//Helper function to read a 8 bit value from memory
uint8_t CPU::fetch_8(){

    uint8_t val = b->read_8(r->PC + 1);
    return val;

}

void CPU::setFlag(uint8_t flag, bool val){
                
    if(val){

        r->f |= flag;
    }else{

        r->f &= ~flag; 
    }

}

void CPU::setZ(bool val){

    setFlag(FLAG_Z, val);
}

void CPU::setN(bool val){

    setFlag(FLAG_N, val);
}

void CPU::setH(bool val){

    setFlag(FLAG_H, val);
}

void CPU::setC(bool val){

    setFlag(FLAG_C, val);
}

uint8_t CPU::inc8(uint8_t val){

    uint8_t old = val;
    val += 1;

    setZ(val == 0);
    setN(false);
    setH((old & 0x0F) == 0x0F);
    return val;
}

uint8_t CPU::dec8(uint8_t val){
            
    uint8_t old = val;
    val -= 1;

    setZ(val == 0);
    setN(true);
    setH((old & 0x0F) == 0x00);

    return val;
}
        
void CPU::addHL(uint16_t reg){
    uint16_t val1 = r->hl;
    uint16_t val2 = reg;
    //Type cast the values to 32 bit add together and store in result
    uint32_t result = (uint32_t)val1 + (uint32_t)val2;
    //Type cast the result back to 16 bit
    r->hl = (uint16_t)result;
    setN(false);
    //Check if lower 12 bits of hl and bc added together is greater then 12 bits
    setH(((val1 & 0x0FFF) + (val2 & 0x0FFF)) > 0x0FFF);
    //See if result if greater then 16 bits
    setC(result > 0xFFFF);
}

bool CPU::checkInteruption(){
    uint8_t IF = b->read_8(0xFF0F);
    uint8_t IE = b->read_8(0xFFFF);
    return ((IE & IF & 0x1F) != 0);
}

void CPU::add(uint8_t val){

    uint8_t a = r->a;

    uint16_t result = a + val;

    r->a = (uint8_t)result;

    setZ(r->a == 0); //Set Z flag if register A value is now 0
    setN(false); //Set N flag to 0
    setH((a & 0x0F) + (val & 0x0F) > 0x0F); //Set H flag to 1 if the low 4 bits added together are greater the 4 bits
    setC(result > 0xFF); //Set C flag to 1 if result is greater than 8 bits 
}

void CPU::sub(uint8_t val){

    uint8_t a = r->a;

    uint16_t result = a - val;

    r->a = (uint8_t)result;

    setZ(r->a == 0); //Set Z flag if register A value is now 0
    setN(true); //Set N flag to 1
    setH((a & 0x0F) < (val & 0x0F)); //Set H flag to 1 if the lower bits of val are greater then the lower bit val of register a
    setC(a < val); //Set C flag to 1 if val is larger the register A value because that means it would be less than 0;

}

void CPU::adc(uint8_t val){
    uint8_t a = r->a;
    int c_in = (r->f & FLAG_C) ? 1:0;

    uint16_t result = a + val + c_in;

    r->a = (uint8_t)result;

    setZ(r->a == 0); //Set Z flag if register A value is now 0
    setN(false); //Set N flag to 0
    setH(((a & 0x0F) + (val & 0x0F) + c_in) > 0x0F); //Set H flag to 1 if the low 4 bits added together are greater the 4 bits
    setC(result > 0xFF); //Set C flag to 1 if result is greater than 8 bits 
}

void CPU::sbc(uint8_t val){
    uint8_t a = r->a;
    int c_in = (r->f & FLAG_C) ? 1:0;

    uint16_t result = a - val - c_in;

    r->a = (uint8_t)result;

    setZ(r->a == 0); //Set Z flag if register A value is now 0
    setN(true); //Set N flag to 0
    setH((a & 0x0F) < ((val & 0x0F) + c_in)); //Set H flag to 1 if the low 4 bits added together are greater the 4 bits
    setC(result > 0xFF); //Set C flag to 1 if result is greater than 8 bits 
}

void CPU::and_a(uint8_t val){

    r->a &= val;

    setZ(r->a == 0);
    setN(false);
    setH(true);
    setC(false);
}

void CPU::xor_a(uint8_t val){

    r->a ^= val;

    setZ(r->a == 0);
    setN(false);
    setH(false);
    setC(false);
}

void CPU::or_a(uint8_t val) {
    r->a |= val;

    setZ(r->a == 0);
    setN(false);
    setH(false);
    setC(false);
}

void CPU::cmp(uint8_t val) {
    uint8_t a = r->a;
    uint16_t result = a - val;

    setZ((uint8_t)result == 0); 
    setN(true); 
    setH((a & 0x0F) < (val & 0x0F)); 
    setC(a < val); 
}

uint16_t CPU::pop16(){

    //Read first byte of stack pointer
    uint8_t lowByte = b->read_8(r->SP);
    r->SP += 1;

    //Read second byte of stack pointer
    uint8_t highByte = b->read_8(r->SP);
    r->SP += 1;

    //Set PC register to combonation of High and Low bytes
    return (highByte << 8) | lowByte;

}

void CPU::push(uint16_t val){

    //Write high byte first
    r->SP -= 1;
    b->write_8(r->SP, (val >> 8) & 0xFF);
    

    //Write low byte
    r->SP -= 1;
    b->write_8(r->SP, val & 0xFF);

}

void CPU::interupts(){

    //Set ime to false to prevent immediate interupt call back
    ime  = false;

    //Get the value for IF (If fired) and IE (If enabled)
    uint8_t IF = b->read_8(0xFF0F);
    uint8_t IE = b->read_8(0xFFFF);

    //Check what the interupt was by anding the 2 bytes of data
    uint8_t pendingInterupt = IF & IE;

    //Check if the VRAM bit was set
    if(pendingInterupt & 0x01){
        //Write IF back to memory but remove the bit that is handled
        b->write_8(0xFF0F, IF & ~0x01);
        //Push current value of PC to stack
        push(r->PC);
        //Set PC to VRAM interrupt source
        r->PC = 0x40;
        return;
    }

    //Check if LCD bit is set
    if(pendingInterupt & 0x02){
        //Write IF back to memory but remove the bit that is handled
        b->write_8(0xFF0F, IF & ~0x02);
        //Push current value of PC to stack
        push(r->PC);
        //Set PC to VRAM interrupt source
        r->PC = 0x48;
        return;
    }

    //Check if Timer bit is set
    if(pendingInterupt & 0x04){
        //Write IF back to memory but remove the bit that is handled
        b->write_8(0xFF0F, IF & ~0x04);
        //Push current value of PC to stack
        push(r->PC);
        //Set PC to VRAM interrupt source
        r->PC = 0x50;
        return;
    }

    //Check if Serial bit is set
    if(pendingInterupt & 0x08){
        //Write IF back to memory but remove the bit that is handled
        b->write_8(0xFF0F, IF & ~0x08);
        //Push current value of PC to stack
        push(r->PC);
        //Set PC to VRAM interrupt source
        r->PC = 0x58;
        return;
    }

    //Check if Joypad bit was set
    if(pendingInterupt & 0x10){
        //Write IF back to memory but remove the bit that is handled
        b->write_8(0xFF0F, IF & ~0x10);
        //Push current value of PC to stack
        push(r->PC);
        //Set PC to VRAM interrupt source
        r->PC = 0x60;
        return;
    }

}

uint8_t CPU::swap(uint8_t val){

    uint8_t newLower = val >> 4;
    uint8_t newHigh = val << 4;
    uint8_t newVal = newHigh | newLower;
    setZ(newVal == 0);
    setH(false);
    setC(false);
    setN(false);
    return newVal;
}

uint8_t CPU::rlc(uint8_t val){

    uint8_t bit7 = val >> 7;
    uint8_t newVal = (val << 1) | bit7;
    setZ(newVal == 0);
    setN(false);
    setH(false);
    setC(bit7);
    return newVal;
}

uint8_t CPU::rl(uint8_t val){

    uint8_t oldCarry = (r->f & FLAG_C) ? 1 : 0;
    uint8_t bit7 = val >> 7;
    uint8_t newVal = (val << 1) | (uint8_t)oldCarry;
    setZ(newVal == 0);
    setH(false);
    setN(false);
    setC(bit7);
    return newVal;

}

uint8_t CPU::rrc(uint8_t val){

    uint8_t bit0 = val & 0x1;
    uint8_t newVal = (val >> 1) | (bit0 << 7);
    setZ(newVal == 0);
    setN(false);
    setH(false);
    setC(bit0);
    return newVal;
}

uint8_t CPU::rr(uint8_t val){

    uint8_t oldCarry = (r->f & FLAG_C) ? 1 : 0;
    uint8_t bit0 = val & 0x1;
    uint8_t newVal = (val >> 1) | ((uint8_t)oldCarry << 7);
    setZ(newVal == 0);
    setH(false);
    setN(false);
    setC(bit0);
    return newVal;

}

uint8_t CPU::sla(uint8_t val){

    uint8_t bit7 = val >> 7;
    uint8_t newVal = (val << 1);
    setZ(newVal == 0);
    setN(false);
    setH(false);
    setC(bit7);
    return newVal;
}

uint8_t CPU::srl(uint8_t val){
    
    uint8_t bit0 = val & 0x1;
    uint8_t newVal = (val >> 1);
    setZ(newVal == 0);
    setN(false);
    setH(false);
    setC(bit0);
    return newVal;
}

uint8_t CPU::sra(uint8_t val){

    uint8_t sign = val & 0x80;
    uint8_t bit0 = val & 0x1;
    uint8_t newVal = (val >> 1) | sign;
    setZ(newVal == 0);
    setN(false);
    setH(false);
    setC(bit0);
    return newVal;

}

void CPU::bit(uint8_t val, uint8_t check){

    uint8_t checkBit = (uint8_t)((val >> check) & 0x1);
    setZ(checkBit == 0);
    setN(false);
    setH(true);

}

uint8_t CPU::res(uint8_t val, uint8_t check){

    uint8_t mask = ~((uint8_t)(1 << check));
    uint8_t newVal = val & mask;
    return newVal;
}

uint8_t CPU::set(uint8_t val, uint8_t check){

    uint8_t mask = (uint8_t)(1 << check);
    uint8_t newVal = val | mask;
    return newVal;
}

int CPU::executeCBOpcode(uint8_t extendedInstruction){
    
    uint8_t opC = extendedInstruction >> 6;
    uint8_t instructType = ((extendedInstruction >> 3) & 0x7);
    uint8_t regType = extendedInstruction & 0x7;
    uint8_t *reg = nullptr;
    bool isHL = (regType == 0x6);

    if(!isHL)
    {
        switch (regType)
        {
        case 0x0:
            reg = &(r->b);
            break;
        case 0x1:
            reg = &(r->c);
            break;
        case 0x02:
            reg = &(r->d);
            break;
        case 0x3:
            reg = &(r->e);
            break;
        case 0x4:
            reg = &(r->h);
            break;
        case 0x5:
            reg = &(r->l);
            break;
        case 0x7:
            reg = &(r->a);
            break;
        default:
            break;
        }
    }

    uint8_t val = isHL ? b->read_8(r->hl): *reg;

    switch (opC)
    {
        case 0x0:
            //If the two MSB equal zero in binary means it is a shift operation
            switch (instructType)
                {
                case 0x0:
                {
                    uint8_t result = rlc(val);
                        if(isHL)
                            b->write_8(r->hl, result);
                        else
                            *reg = result;
                        
                    break;
                }
                
                case 0x1:
                {
                    uint8_t result = rrc(val);
                        if(isHL)
                            b->write_8(r->hl, result);
                        else
                            *reg = result;

                    break;
                }
                
                case 0x2:
                {
                    uint8_t result = rl(val);
                        if(isHL)
                            b->write_8(r->hl, result);
                        else
                            *reg = result;
                    
                    break;
                }

                case 0x3:
                {
                    uint8_t result = rr(val);
                        if(isHL)
                            b->write_8(r->hl, result);
                        else
                            *reg = result;

                    break;
                }

                case 0x4:
                {

                    uint8_t result = sla(val);
                        if(isHL)
                            b->write_8(r->hl, result);
                        else
                            *reg = result;
                
                    break;
                }

                case 0x5:
                {

                    uint8_t result = sra(val);
                        if(isHL)
                            b->write_8(r->hl, result);
                        else
                            *reg = result;

                    break;
                }
                case 0x6:
                {

                    uint8_t result = swap(val);
                        if(isHL)
                            b->write_8(r->hl, result);
                        else
                            *reg = result;

                    break;
                }
                case 0x7:
                {

                    uint8_t result = srl(val);
                        if(isHL)
                            b->write_8(r->hl, result);
                        else
                            *reg = result;

                    break;
                }
                default:
                    break;
                }
            
            break;
        
        case 0x1:
        //If the two MSB equal one in binary means it is a bit operation
        {
            bit(val,instructType);
            break;
        }   


        case 0x2:
            //If the two MSB equal two in binary means it is a RES operation
        {
            uint8_t result = res(val, instructType);
            if(isHL)
                b->write_8(r->hl, result);
            else
                *reg = result;
            break;
        }
        case 0x3:
            //If the two MSB equal three in binary means it is a SET operation
        {
            uint8_t result = set(val, instructType);
            if(isHL)
                b->write_8
                (r->hl, result);
            else
                *reg = result;
            break;
        }    
        default:
            break;
    }

    if(isHL)
        return (opC == 0x1) ? 12 : 16;
    else
        return 8;

}

uint8_t CPU::step(){

    if(ime && checkInteruption()){
        isHalted = false;
        interupts();
        return 20;
    }
    //Check if the cpu is halted
    if(isHalted){

        if(checkInteruption()){
            isHalted = false;
        }

        return 4;
    }

    //Read the opCode from PC register
    uint8_t opCode = b->read_8(r->PC);
    bool hadHalt = haltBug;
    haltBug = false;

    //Track cycle for step
    uint8_t cycle;

    
    //Based on pointer content preform an operation
    switch(opCode){
        
        case(0x0):
        {
            // NOP 1 4 ----
            //On memory map row 0x column x0 NOP increment PC by 1 byte
            r->PC += 1;
            cycle = 4;

            break;
        }

        case(0x01):
        {
            // LD BC, n16 3 12 ----
            //On memory map row 0x column x1 fetch 16 bit int and assign it to bc register
            r->bc = fetch_16();
            r->PC += 3;
            cycle = 12;
            break;
        }

        case(0x02):
        {
            // LD [BC], A 1 8 ----
            //On memory map row 0x column x2 write contents of A to address stored in bc registry
            b->write_8(r->bc,r->a);
            r->PC += 1;
            cycle = 8;

            break; 
        }
            
        case(0x03):
        {
            // INC BC 1 8 ----
            //On memory map row 0x column x3 increment the BC register by one bit 
            r->bc += 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case(0x04):
        {
            // INC B 1 4 Z 0 H -
            //On memory map row 0x column x4 increment B register set z n and h flags accordingly
            r->b = inc8(r->b);
            r->PC += 1;
            cycle = 4;

            break;

        }
            
        case(0x05):
        {
            // DEC B 1 4 Z 1 H -
            //On memory map row 0x column x5 decrement B register set z n and h flags accordingly
            r->b = dec8(r->b);
            r->PC += 1;
            cycle = 4;

            break;
        }    
        
        case(0x06):
        {
            //LD B n8 2 8 ----
            //On memory map row 0x column x6 fetch 8 bit int and assign to b register;

            r->b = fetch_8();
            r->PC += 2;
            cycle = 8;

            break;
        }
            
        case(0x07):
        {
            // RCLA 1 4 0 0 0 C
            //On memory map row 0x column x7 rotate accumulator bits to left bit 7 wraps around to bit 0

            //Have variable to hold old value of accumulator
            uint8_t bit7 = r->a;
            //Set value of accumulator to bits shifted to left by one
            r->a = r->a << 1;
            //Set value of bit 7 to the seventh bit shifted to bit 0 position
            bit7 = bit7 >> 7;
            //Set accumulator to bits rotated to left with 7 bit wrapping to 0 bit position
            r->a = r->a | bit7;

            setZ(false);
            setN(false);
            setH(false);
            setC(bit7);

            r->PC += 1;
            cycle = 4;

            break;
        }


        case(0x08):
        {
            // LD [a16], SP 3 20 ----
            //On memory map row 0x column x8 write stack pointer to address after opcode
            uint16_t a = b->read_16(r->PC + 1);
            b->write_16(a, r->SP);
            r->PC += 3;
            cycle = 20;
            break;
        }
            
        case(0x09):
        {
            // ADD HL, BC 1 8 - 0 H C
            //On memory map row 0x column x9 add value in BC register to HL register
            addHL(r->bc);
            r->PC += 1;
            cycle = 8;
            break;
        }
            
        case(0x0A):
        {
            // LD A, [BC] 1 8 ----
            //On memory map row 0x column xA load the the value stored in at the memory address in the BC register to register A
            r->a = b->read_8(r->bc);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case(0x0B):
        {
            // DEC BC 1 8 ----
            //On memory map row 0x column xB decrement BC register
            r->bc -= 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case(0x0C):
        {
            // INC C 1 4 Z 0 H -
            //On memory map row 0x column xC increment C register and raise z n and h flags based on result
            r->c = inc8(r->c);
            r->PC += 1;
            cycle = 4;

            break;
        }

        case(0x0D):
        {
            // DEC C 1 4 Z 1 H -
            //One memory map row 0x column xD decrement register C and raise z n and h flags based on result
            r->c = dec8(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case(0x0E):
        {
            //LD C n8 2 8 ----
            //On memory map row 0x column xE fetch 8 bit int and assign to C register;
            r->c = fetch_8();
            r->PC += 2;
            cycle = 8;
            break;
        }
            
        case(0x0F):
        {
            //RRCA 1 4 0 0 0 C
            uint16_t bit0 = r->a & 0x01;
            r->a = r->a >> 1;
            r->a |= bit0 << 7;
            
            setZ(false);
            setN(false);
            setH(false);
            setC(bit0);

            r->PC += 1;
            cycle = 4;
            break;
        }

        case(0x10):
        {
            // STOP n8 2 4 ----
            //On memory map row 1x and column x0 stop cpu
            r->PC += 2;
            cpu_stopped = true;
            cycle = 4;
            break;
        }

        case (0x11):
        {
            // LD DE n16 3 12 ----
            //On memory map row 1x column x1 fetch 16 bit and assign to DE register
            r->de = fetch_16();
            r->PC += 3;
            cycle = 12;
            break;

        }

        case (0x12):
        {
            //LD [DE], A 1 8 ----
            //On memory map row 1x column x2 write value in register A to address stored in DE register
            b->write_8(r->de, r->a);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case(0x13):
        {
            // INC DE 1 8 ----
            //On memory map row 1x column x3 increment de register
            r->de += 1;
            r->PC += 1;
            cycle = 8;
            break;

        }

        case (0x14):
        {
            // INC D 1 4 Z 0 H -
            //On memory map row 1x column x4 increment d register and call proper flags
            r->d = inc8(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case(0x15):
        {
            // DEC D 1 4 Z 1 H -
            //On memory map row 1x column x5 decrement d register and call proper flags
            r->d = dec8(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x16):
        {
            // LD D n8 2 8 ----
            //On memory map row 1x column x6 fetch 8 bit and D register
            r->d = fetch_8();
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0x17):
        {
            // RLA 1 4 0 0 0 C
            //On memory map row 1x column x7 rotate accumulator bit to the left by one and do not wrap bit 7
            uint8_t bit7 = r->a >> 7;
            uint8_t oldC = (r->f & FLAG_C) ? 1:0;
            r->a = (r->a << 1) | oldC;
            
            setZ(false);
            setN(false);
            setH(false);
            setC(bit7);

            r->PC += 1;
            cycle = 4;

            break;

        }

        case (0x18):
        {
            // JR e8 2 12
            //On memory map row 1x column x8 jump PC register ahead by the value of 8bit signed int
            int8_t signJump = (int8_t)b->read_8(r->PC + 1);
            r->PC = r->PC + 2 + signJump;
            cycle = 12;
            break;
        }
        
        case (0x19):
        {
            // ADD HL, DE 1 8 - H L C
            //On memory map row 1x column x9 add value in DE register to HL register
            addHL(r->de);
            r->PC += 1;
            cycle = 8;
            break;

        }

        case (0x1A):
        {
            // LD A [DE] 1 8 ----
            //On memory map row 1x column xA load contents of value at address stored in DE register to A register
            r->a = b->read_8(r->de);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x1B):
        {
            // DEC DE 1 8 ----
            //On memory map row 1x column xB decrement DE register
            r->de -= 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x1C):
        {
            // INC E 1 4 Z 0 H -
            //On memory map row 1x column xC increment e register
            r->e = inc8(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x1D):
        {
            // DEC E 1 4 Z 1 H -
            //On memory map row 1x column xD increment e register
            r->e = dec8(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x01E):
        {
            // LD E n8 2 8 ----
            //On memory map row 1x column xE load 8 bit address to e register
            r->e = fetch_8();
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0x1F):
        {
            // RRA 1 4 0 0 0 C
            //On memory map row 1x column xF rotate accumulator bit to the right by one and do not wrap bit 0
            uint8_t bit0 = (r->a & 0x01);
            uint8_t oldC = (r->f & FLAG_C) ? 1:0;
            r->a = (r->a >> 1) | (oldC << 7);
            
            setZ(false);
            setN(false);
            setH(false);
            setC(bit0);

            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x20):
        {   
            // JR NZ e8 2 12/8 (12 or 8) ----
            //On memory map row 2x column x0 jump if z flag not set else do nothing
            int8_t signJump = (int8_t)b->read_8(r->PC + 1);
            uint16_t target = r->PC + 2 + signJump;
            bool z = (r->f & FLAG_Z) ? 1 : 0;
            if(!z) {
                r->PC = target;
                cycle = 12;
            } else {
                r->PC += 2;
                cycle = 8;
            }

            break;
        }

        case (0x21):
        {
            // LD HL n16 3 12 ----
            //On memory map row 2x column x1 fetch 16 bit value and assign to HL register 
            r->hl = fetch_16();
            r->PC += 3;
            cycle = 12;
            break;
        }

        case (0x22):
        {
            // LD [HL+], A 1 8 ----
            //On memory map row 2x column x2 load the value of A register to HL register and increment after
            b->write_8(r->hl, r->a);
            r->hl++;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x23):
        {
            // INC HL 1 8 ----
            //On memory map row 2x column x3 increment HL register
            r->hl += 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x24):
        {
            // INC H 1 4 Z 0 H -
            //On memory map row 2x column x4 increment H register
            r->h = inc8(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x25):
        {
            // DEC H 1 4 Z 1 H -
            //On memory map row 2x column x5 decrement H register
            r->h = dec8(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x26):
        {
            // LD H n8 2 8 ----
            //On memory map row 2x column x6 fetch 8 bit value and assign to H register
            r->h = fetch_8();
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0x27):
        {
            // DAA 1 4 Z - 0 C
            //On memory map row 2x column x7 to BDC on register A
            uint8_t C = (r->f & FLAG_C) ? 1:0;
            uint8_t H = (r->f & FLAG_H) ? 1:0;
            uint8_t N = (r->f & FLAG_N) ? 1:0;
            uint8_t adjustment = 0;
            bool carry = C;
            
            if(H || (!N && (r->a & 0x0F) > 0x09))
            {
                adjustment |= 0x06;
            }

            if(C || (!N && r->a > 0x99))
            {
                adjustment |= 0x60;
                carry = true;
            }

            if(N)
            {
                r->a -= adjustment;
            }
            else
            {
                r->a += adjustment;
            }

            setZ(r->a == 0);
            setH(false);
            setC(carry);

            r->PC += 1;
            cycle = 4;

            break;
        }

        case (0x28):
        {
            // JR Z e8 2 12/8 (12 or 8) ----
            //On memory map row 2x column x8 jump if z flag set else do nothing
            bool z =  (r->f & FLAG_Z) ? 1:0;
            if(z)
            {
                int8_t signJump = (int8_t)b->read_8(r->PC + 1);
                r->PC += (2 + signJump);
                cycle = 12;
                

            }
            else
            {
                r->PC += 2;
                cycle = 8;
            }

            break;
        }

        case (0x29):
        {   
            // ADD HL, HL 1  8 - 0 H C
            //On memory map row 2x column x9 add HL register to itself
            addHL(r->hl);
            r->PC += 1;
            cycle = 8;
            break;
        } 

        case (0x2A):
        {
            // LD A, [HL+] 1 8 ----
            //On memory map row 2x column xA load value at address stored in HL to register A then increment HL
            r->a = b->read_8(r->hl);
            r->hl += 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x2B):
        {
            // DEC HL 1  8 ----
            //On memory map row 2x column xB decrement value in HL register
            r->hl -= 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x2C):
        {
            // INC L 1  4 Z 0 H -
            //On memory map row 2x column xC increment value in register L
            r->l = inc8(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x2D):
        {
            // DEC L 1  4 Z 1 H -
            //On memory map row 2x column xD decremnt value in register L
            r->l = dec8(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x2E):
        {
            // LD L, n8 2  8
            //On memory map row 2x column xE fetch 8 bit value and load to register L
            r->l = fetch_8();
            r->PC += 2;
            cycle = 8;
            break; 
        }

        case (0x2F):
        {
            // CPL 1  4 - 1 1 -
            //On memory map row 2x column xF invert register A and manually set flag
            r->a = ~r->a;
            setN(true);
            setH(true);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x30):
        {
            // JR NC e8 2 12/8 (12 or 8) ----
            //On memory map row 3x column x0 jump if c flag set else do nothing
            bool c =  (r->f & FLAG_C) ? 1:0;
            if(!c)
            {
                int8_t signJump = (int8_t)b->read_8(r->PC + 1);
                r->PC += (2 + signJump);
                cycle = 12;
                

            }
            else
            {
                r->PC += 2;
                cycle = 8;
            }

            break;
        }

        case (0x31):
        {
            // LD SP, n16 3  12 - - - -
            // On memory map row 3x column x1 fetch and load 16 bit value to SP register
            r->SP = fetch_16();
            r->PC += 3;
            cycle = 12;
            break;
        }

        case (0x32):
        {
            // LD [HL-], A 1 8 ----
            //On memory map row 3x column x2 load the value of A register to HL register and decrement after
            b->write_8(r->hl, r->a);
            r->hl--;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x33):
        {
            // INC SP 1  8 - - - -
            //On memory map row 3x column x3 increment SP register
            r->SP += 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x34):
        {
            // INC [HL] 1 12 Z 0 H -
            //On memory map row 3x column x4 in increment the value stored at the address in HL register

            uint8_t val = b->read_8(r->hl);
            setH((val & 0x0F) == 0x0F);

            val++;

            b->write_8(r->hl, val);

            setZ(val == 0);
            setN(0);

            r->PC += 1;
            cycle = 12;
            break;
        }

        case (0x35):
        {
            // DEC [HL] 1  12 Z 1 H -
            //On memory map row 3x column x5 in decrement the value stored at the address in HL register
            uint8_t val = b->read_8(r->hl);
            setH((val & 0x0F) == 0x00);

            val--;

            b->write_8(r->hl, val);

            setZ(val == 0);
            setN(1);

            r->PC += 1;
            cycle = 12;
            break;
        }

        case (0x36):
        {
            // LD [HL], n8 2  12 - - - -
            //On memory map row 3x column x6 fetch 8 bit val and write to HL register
            b->write_8(r->hl, fetch_8());
            r->PC += 2;
            cycle = 12;
            break;
        }

        case (0x37):
        {
            // SCF 1  4 - 0 0 1
            //On memory map row 3x column x7 set flags h n and c
            setC(true);
            setN(false);
            setH(false);

            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x38):
        {
            // JR C e8 2 12/8 (12 or 8) ----
            //On memory map row 3x column x8 jump if C flag set else do nothing
            bool z =  (r->f & FLAG_C) ? 1:0;
            if(z)
            {
                int8_t signJump = (int8_t)b->read_8(r->PC + 1);
                r->PC += (2 + signJump);
                cycle = 12;
                

            }
            else
            {
                r->PC += 2;
                cycle = 8;
            }

            break;
        }

        case (0x39):
        {
            // ADD HL, SP 1  8 - 0 H C
            //On memory map row 3x column x9 add SP register to HL register
            addHL(r->SP);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x3A):
        {
            // LD A, [HL-] 1  8 - - - -
            //On memory map row 3x column xA load value at address stored in HL to register A then decrement HL
            r->a = b->read_8(r->hl);
            r->hl -= 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x3B):
        {
            // DEC SP 1  8 - - - -
            //On memory map row 3x column xB decremnt SP register
            r->SP -= 1;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x3C):
        {
            // INC A 1  4 Z 0 H -
            //On memory map row 3x column xC increment register A
            r->a = inc8(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x3D):
        {
            // DEC A 1  4 Z 1 H -
            //On memory map row 3x column xD decrement register A
            r->a = dec8(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x3E):
        {
            // LD A, n8 2  8 - - - -
            //On memory map row 3x column xE fetch and load 8 bit value to register A
            r->a = fetch_8();
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0x3F):
        {
            // CCF 1  4 - 0 0 C
            //On memory map row 3x column xF set c flag to opposite of what it is
            bool c =  (r->f & FLAG_C) ? 1:0;
            setC(!c);
            setH(false);
            setN(false); 
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x40):
        {
            // LD B, B 1  4 - - - -
            //On memory map row 4x column x0 load value in register B to register B (Basically NOP)
            r->b = r->b;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x41):
        {
            // LD B, C 1  4 - - - -
            //On memory map row 4x column x1 load value in register C to register B
            r->b = r->c;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x42):
        {
            // LD B, D 1  4 - - - -
            //On memory map row 4x column x2 load value in register D to register B
            r->b = r->d;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x43):
        {
            // LD B, E 1  4 - - - -
            //On memory map row 4x column x3 load value in register E to register B
            r->b = r->e;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x44):
        {
            // LD B, H 1  4 - - - -
            //On memory map row 4x column x4 load value in register H to register B
            r->b = r->h;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x45):
        {
            // LD B, L 1  4 - - - -
            //On memory map row 4x column x5 load value in register L to register B
            r->b = r->l;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x46):
        {
            // LD B, [HL] 1  8 - - - -
            //On memory map row 4x column x6 load value at address in register HL to register B
            r->b = b->read_8(r->hl);
            r->PC += 1;
            cycle = 8;
            break;                   
        }

        case (0x47):
        {
            // LD B, A 1  4 - - - -
            //On memory map row 4x column x7 load value in register A to register B
            r->b = r->a;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x48):
        {
            // LD C, B 1  4 - - - -
            //On memory map row 4x column x8 load value in register B to register C
            r->c = r->b;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x49):
        {
            // LD C, c 1  4 - - - -
            //On memory map row 4x column x9 load value in register C to register C (Basically NOP)
            r->c = r->c;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x4A):
        {
            // LD C, D 1  4 - - - -
            //On memory map row 4x column xA load value in register D to register C
            r->c = r->d;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x4B):
        {
            // LD C, E 1  4 - - - -
            //On memory map row 4x column xB load value in register E to register C
            r->c = r->e;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x4C):
        {
            // LD C, H 1  4 - - - -
            //On memory map row 4x column xC load value in register H to register C
            r->c = r->h;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x4D):
        {
            // LD C, L 1  4 - - - -
            //On memory map row 4x column xD load value in register L to register C
            r->c = r->l;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x4E):
        {
            // LD C, [HL] 1  8 - - - -
            //On memory map row 4x column xE load value at address in register HL to register C
            r->c = b->read_8(r->hl);
            r->PC += 1;
            cycle = 8;
            break;                   
        }

        case (0x4F):
        {
            // LD C, A 1  4 - - - -
            //On memory map row 4x column xF load value in register A to register C
            r->c = r->a;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x50):
        {
            // LD D, B 1  4 - - - -
            //On memory map row 5x column x0 load value in register B to register D
            r->d = r->b;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x51):
        {
            // LD D, C 1  4 - - - -
            //On memory map row 5x column x1 load value in register C to register D
            r->d = r->c;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x52):
        {
            // LD D, D 1  4 - - - -
            //On memory map row 5x column x2 load value in register D to register D (Basically NOP)
            r->d = r->d;
            r->PC += 1;
            cycle = 4;
            break;                   
        }
        

        case (0x53):
        {
            // LD D, E 1  4 - - - -
            //On memory map row 5x column x3 load value in register E to register D
            r->d = r->e;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x54):
        {
            // LD D, H 1  4 - - - -
            //On memory map row 5x column x4 load value in register H to register D
            r->d = r->h;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x55):
        {
            // LD D, L 1  4 - - - -
            //On memory map row 5x column x5 load value in register L to register D
            r->d = r->l;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x56):
        {
            // LD D, [HL] 1  8 - - - -
            //On memory map row 5x column x6 load value to address in register HL to register D
            r->d = b->read_8(r->hl);
            r->PC += 1;
            cycle = 8;
            break;                   
        }

        case (0x57):
        {
            // LD D, A 1  4 - - - -
            //On memory map row 5x column x7 load value in register A to register D
            r->d = r->a;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x58):
        {
            // LD E, B 1  4 - - - -
            //On memory map row 5x column x8 load value in register B to register E
            r->e = r->b;
            r->PC += 1;
            cycle = 4;
            break;                   
        }
        
        case (0x59):
        {
            // LD E, C 1  4 - - - -
            //On memory map row 5x column x9 load value in register C to register E
            r->e = r->c;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x5A):
        {
            // LD E, D 1  4 - - - -
            //On memory map row 5x column xA load value in register D to register E
            r->e = r->d;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x5B):
        {
            // LD E, E 1  4 - - - -
            //On memory map row 5x column xB load value in register E to register E (Basically NOP)
            r->e = r->e;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x5C):
        {
            // LD E, H 1  4 - - - -
            //On memory map row 5x column xC load value in register H to register E
            r->e = r->h;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x5D):
        {
            // LD E, L 1  4 - - - -
            //On memory map row 5x column xD load value in register L to register E
            r->e = r->l;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x5E):
        {
            // LD E, [HL] 1  8 - - - -
            //On memory map row 5x column xE load value to address in register HL to register E
            r->e = b->read_8(r->hl);
            r->PC += 1;
            cycle = 8;
            break;                   
        }

        case (0x5F):
        {
            // LD E, A 1  4 - - - -
            //On memory map row 5x column xF load value in register A to register E
            r->e = r->a;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x60):
        {
            // LD H, B 1  4 - - - -
            //On memory map row 6x column x0 load value in register B to register H
            r->h = r->b;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x61):
        {
            // LD H, C 1  4 - - - -
            //On memory map row 6x column x1 load value in register C to register H
            r->h = r->c;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x62):
        {
            // LD H, D 1  4 - - - -
            //On memory map row 6x column x2 load value in register D to register H
            r->h = r->d;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x63):
        {
            // LD H, E 1  4 - - - -
            //On memory map row 6x column x3 load value in register E to register H
            r->h = r->e;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x64):
        {
            // LD H, H 1  4 - - - -
            //On memory map row 6x column x4 load value in register H to register H (Basically NOP)
            r->h = r->h;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x65):
        {
            // LD H, L 1  4 - - - -
            //On memory map row 6x column x0 load value in register L to register H
            r->h = r->l;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x66):
        {
            // LD H, [HL] 1  8 - - - -
            //On memory map row 6x column x6 load value to address in register HL to register H
            r->h = b->read_8(r->hl);
            r->PC += 1;
            cycle = 8;
            break;                   
        }

        case (0x67):
        {
            // LD H, A 1  4 - - - -
            //On memory map row 6x column x7 load value in register A to register H
            r->h = r->a;
            r->PC += 1;
            cycle = 4;
            break;                   
        }

        case (0x68):
        {
            // LD L, B 1  4 - - - -
            //On memory map row 6x column x8 load value in register B to register L
            r->l = r->b;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x69):
        {
            // LD L, C 1  4 - - - -
            //On memory map row 6x column x9 load value in register C to register L
            r->l = r->c;
            r->PC += 1;
            cycle = 4;
            break;
        }
        
        case (0x6A):
        {
            // LD L, D 1  4 - - - -
            //On memory map row 6x column x8 load value in register D to register L
            r->l = r->d;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x6B):
        {
            // LD L, E 1  4 - - - -
            //On memory map row 6x column xB load value in register E to register L
            r->l = r->e;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x6C):
        {
            // LD L, H  1  4 - - - -
            //On memory map row 6x column xC load value in register H to register L
            r->l = r->h;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x6D):
        {
            // LD L, L 1  4 - - - -
            //On memory map row 6x column x8 load value in register L to register L (Basically NOP)
            r->l = r->l;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x6E):
        {
            // LD L, [HL] 1  8 - - - -
            //On memory map row 6x column xE load value to address in register HL to register L
            r->l = b->read_8(r->hl);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x6F):
        {
            // LD L, A 1  4 - - - -
            //On memory map row 6x column xF load value in register A to register L
            r->l = r->a;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x70):
        {
            // LD [HL], B 1  8 - - - -
            //On memory map row 7x column x0 load value in register B to the address in register HL
            b->write_8(r->hl,r->b);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x71):
        {
            // LD [HL], C 1  8 - - - -
            //On memory map row 7x column x1 load value in register C to the address in register HL
            b->write_8(r->hl,r->c);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x72):
        {
            // LD [HL], D 1  8 - - - -
            //On memory map row 7x column x2 load value in register D to the address in register HL
            b->write_8(r->hl,r->d);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x73):
        {
            // LD [HL], E 1  8 - - - -
            //On memory map row 7x column x3 load value in register E to the address in register HL
            b->write_8(r->hl,r->e);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x74):
        {
            // LD [HL], H 1  8 - - - -
            //On memory map row 7x column x4 load value in register H to the address in register HL
            b->write_8(r->hl,r->h);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x75):
        {
            // LD [HL], L 1  8 - - - -
            //On memory map row 7x column x5 load value in register L to the address in register HL
            b->write_8(r->hl,r->l);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x76):
        {
            // HALT 1  $ - - - -
            //On memory map row 7x column x6 set the isHalted var to true so gameboy halts any further instructions
            if(!ime && checkInteruption())
            {
                haltBug = true;
                r->PC += 1;
            }
            else
            {
                isHalted = true;
                r->PC += 1;
            }
            cycle = 4;
            break;
        }

        case (0x77):
        {
            // LD [HL], A 1  8 - - - -
            //On memory map row 7x column x7 load value in register A to the address in register HL
            b->write_8(r->hl,r->a);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x78):
        {
            // LD A, B 1  4 - - - -
            //On memory map row 7x column x8 load value in register B to register A
            r->a = r->b;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x79):
        {
            // LD A, C 1  4 - - - -
            //On memory map row 7x column x9 load value in register C to register A
            r->a = r->c;
            r->PC += 1;
            cycle = 4;
            break;
        }


        case (0x7A):
        {
            // LD A, D 1  4 - - - -
            //On memory map row 7x column xA load value in register D to register A
            r->a = r->d;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x7B):
        {
            // LD A, E 1  4 - - - -
            //On memory map row 7x column xB load value in register E to register A
            r->a = r->e;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x7C):
        {
            // LD A, H 1  4 - - - -
            //On memory map row 7x column xC load value in register H to register A
            r->a = r->h;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x7D):
        {
            // LD A, L 1  4 - - - -
            //On memory map row 7x column xD load value in register L to register A
            r->a = r->l;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x7E):
        {
            // LD A, [HL] 1  8 - - - -
            //On memory map row 7x column xF load value at address in register HL to register A
            r->a = b->read_8(r->hl);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x7F):
        {
            // LD A, A 1  4 - - - -
            //On memory map row 7x column xF load value in register A to register A (Basically NOP)
            r->a = r->a;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x80):
        {
            // ADD A, B 1  4 Z 0 H C
            //On memory map row 8x column x0 add value in register B to value in register A
            add(r->b);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x81):
        {
            // ADD A, C 1  4 Z 0 H C
            //On memory map row 8x column x1 add value in register C to value in register A
            add(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x82):
        {
            // ADD A, D 1  4 Z 0 H C
            //On memory map row 8x column x2 add value in register D to value in register A
            add(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x83):
        {
            // ADD A, E 1  4 Z 0 H C
            //On memory map row 8x column x3 add value in register E to value in register A
            add(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x84):
        {
            // ADD A, H 1  4 Z 0 H C
            //On memory map row 8x column x4 add value in register H to value in register A
            add(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x85):
        {
            // ADD A, L 1  4 Z 0 H C
            //On memory map row 8x column x5 add value in register L to value in register A
            add(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x86):
        {
            // ADD A, [HL] 1  4 Z 0 H C
            //On memory map row 8x column x6 add value at address stored in register HL to value in register A
            add(b->read_8(r->hl));
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x87):
        {
            // ADD A, A 1  4 Z 0 H C
            //On memory map row 8x column x7 add value in register A to value in register A
            add(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x88):
        {
            // ADC A, B 1  4 Z 0 H C
            //On memory map row 8x column x8 add value in register B to value in register A plus c_in from C flag
            adc(r->b);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x89):
        {
            // ADC A, C 1  4 Z 0 H C
            //On memory map row 8x column x9 add value in register C to value in register A plus c_in from C flag
            adc(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x8A):
        {
            // ADC A, D 1  4 Z 0 H C
            //On memory map row 8x column xA add value in register D to value in register A plus c_in from C flag
            adc(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x8B):
        {
            // ADC A, E 1  4 Z 0 H C
            //On memory map row 8x column xB add value in register E to value in register A plus c_in from C flag
            adc(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x8C):
        {
            // ADC A, H 1  4 Z 0 H C
            //On memory map row 8x column xC add value in register H to value in register A plus c_in from C flag
            adc(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x8D):
        {
            // ADC A, L 1  4 Z 0 H C
            //On memory map row 8x column xD add value in register L to value in register A plus c_in from C flag
            adc(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x8E):
        {
            // ADC A, [HL] 1  8 Z 0 H C
            //On memory map row 8x column xE add value at address in register HL to value in register A plus c_in from C flag
            adc(b->read_8(r->hl));
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x8F):
        {
            // ADC A, A 1  4 Z 0 H C
            //On memory map row 8x column xF add value in register A to value in register A plus c_in from C flag
            adc(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x90):
        {
            // SUB A, B 1  4 Z 1 H C
            //On memory map row 9x column x0 subtract value in register B from value in register A
            sub(r->b);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x91):
        {
            // SUB A, C 1  4 Z 1 H C
            //On memory map row 9x column x1 subtract value in register C from value in register A
            sub(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x92):
        {
            // SUB A, D 1  4 Z 1 H C
            //On memory map row 9x column x2 subtract value in register D from value in register A
            sub(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x93):
        {
            // SUB A, E 1  4 Z 1 H C
            //On memory map row 9x column x3 subtract value in register E from value in register A
            sub(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x94):
        {
            // SUB A, H 1  4 Z 1 H C
            //On memory map row 9x column x4 subtract value in register H from value in register A
            sub(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x95):
        {
            // SUB A, L 1  4 Z 1 H C
            //On memory map row 9x column x5 subtract value in register L from value in register A
            sub(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x96):
        {
            // SUB A, [HL] 1  8 Z 1 H C
            //On memory map row 9x column x6 subtract value at address in register HL from value in register A
            sub(b->read_8(r->hl));
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x97):
        {
            // SUB A, A 1  4 Z 1 H C
            //On memory map row 9x column x7 subtract value in register A from value in register A
            sub(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x98):
        {
            // SBC A, B 1  4 Z 1 H C
            //On memory map row 9x column x8 subtract value in register B and c_in from C flag from value in register A
            sbc(r->b);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x99):
        {
            // SBC A, C 1  4 Z 1 H C
            //On memory map row 9x column x9 subtract value in register C and c_in from C flag from value in register A
            sbc(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x9A):
        {
            // SBC A, D 1  4 Z 1 H C
            //On memory map row 9x column xA subtract value in register D and c_in from C flag from value in register A
            sbc(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x9B):
        {
            // SBC A, E 1  4 Z 1 H C
            //On memory map row 9x column xD subtract value in register E and c_in from C flag from value in register A
            sbc(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x9C):
        {
            // SBC A, H 1  4 Z 1 H C
            //On memory map row 9x column xC subtract value in register H and c_in from C flag from value in register A
            sbc(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x9D):
        {
            // SBC A, L 1  4 Z 1 H C
            //On memory map row 9x column xD subtract value in register L and c_in from C flag from value in register A
            sbc(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0x9E):
        {
            // SBC A, [HL] 1  8 Z 1 H C
            //On memory map row 9x column xE subtract value at address in register HL and c_in from C flag from value in register A
            sbc(b->read_8(r->hl));
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0x9F):
        {
            // SBC A, A 1  4 Z 1 H C
            //On memory map row 9x column xF subtract value in register A and c_in from C flag from value in register A
            sbc(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA0):
        {
            // AND A, B 1  4 Z 0 1 0
            //On memory map row Ax column x0 set register A to be the result of register A & register B
            and_a(r->b);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA1):
        {
            // AND A, C 1  4 Z 0 1 0
            //On memory map row Ax column x1 set register A to be the result of register A & register C
            and_a(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA2):
        {
            // AND A, D 1  4 Z 0 1 0
            //On memory map row Ax column x0 set register A to be the result of register A & register D
            and_a(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA3):
        {
            // AND A, E 1  4 Z 0 1 0
            //On memory map row Ax column x0 set register A to be the result of register A & register E
            and_a(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA4):
        {
            // AND A, H 1  4 Z 0 1 0
            //On memory map row Ax column x4 set register A to be the result of register A & register H
            and_a(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA5):
        {
            // AND A, L 1  4 Z 0 1 0
            //On memory map row Ax column x5 set register A to be the result of register A & register L
            and_a(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA6):
        {
            // AND A, [HL] 1  8 Z 0 1 0
            //On memory map row Ax column x6 set register A to be the result of register A & value at address in register HL
            and_a(b->read_8(r->hl));
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0xA7):
        {
            // AND A, E 1  4 Z 0 1 0
            //On memory map row Ax column x7 set register A to be the result of register A & register A
            and_a(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA8):
        {
            // XOR A, B 1  4 Z 0 0 0
            //On memory map row Ax column x8 set register A to be the result of register A XOR register B
            xor_a(r->b);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xA9):
        {
            // XOR A, C 1  4 Z 0 0 0
            //On memory map row Ax column x9 set register A to be the result of register A XOR register C
            xor_a(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xAA):
        {
            // XOR A, D 1  4 Z 0 0 0
            //On memory map row Ax column xA set register A to be the result of register A XOR register D
            xor_a(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xAB):
        {
            // XOR A, E 1  4 Z 0 0 0
            //On memory map row Ax column xB set register A to be the result of register A XOR register E
            xor_a(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xAC):
        {
            // XOR A, H 1  4 Z 0 0 0
            //On memory map row Ax column xC set register A to be the result of register A XOR register H
            xor_a(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xAD):
        {
            // XOR A, L 1  4 Z 0 0 0
            //On memory map row Ax column xD set register A to be the result of register A XOR register L
            xor_a(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xAE):
        {
            // XOR A, [HL] 1  8 Z 0 0 0
            //On memory map row Ax column xE set register A to be the result of register A XOR value at address in register HL 
            xor_a(b->read_8(r->hl));
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0xAF):
        {
            // XOR A, A 1  4 Z 0 0 0
            //On memory map row Ax column xF set register A to be the result of register A XOR register A
            xor_a(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xB0):
        {
            // OR A, B 1  4 Z 0 0 0
            //On memory map row Bx column x0 set register to be the result of register A or register B
            or_a(r->b);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xB1):
        {
            // OR A, C 1  4 Z 0 0 0
            //On memory map row Bx column x1 set register to be the result of register A or register C
            or_a(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xB2):
        {
            // OR A, D 1  4 Z 0 0 0
            //On memory map row Bx column x2 set register to be the result of register A or register D
            or_a(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xB3):
        {
            // OR A, E 1  4 Z 0 0 0
            //On memory map row Bx column x3 set register to be the result of register A or register E
            or_a(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }
        
        case (0xB4):
        {
            // OR A, H 1  4 Z 0 0 0
            //On memory map row Bx column x4 set register to be the result of register A or register H
            or_a(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xB5):
        {
            // OR A, L 1  4 Z 0 0 0
            //On memory map row Bx column x5 set register to be the result of register A or register L
            or_a(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xB6):
        {
            // OR A, [HL] 1  4 Z 0 0 0
            //On memory map row Bx column x6 set register to be the result of register A or value at address in register HL
            or_a(b->read_8(r->hl));
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0xB7):
        {
            // OR A, A 1  4 Z 0 0 0
            //On memory map row Bx column x7 set register to be the result of register A or register A
            or_a(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xB8):
        {
            // CP A, B 1  4 Z 1 H C
            //On memory map row Bx column x8 subtract register B from register A, but don't save value just use to set flags
            cmp(r->b);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xB9):
        {
            // CP A, C 1  4 Z 1 H C
            //On memory map row Bx column x9 subtract register C from register A, but don't save value just use to set flags
            cmp(r->c);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xBA):
        {
            // CP A, D 1  4 Z 1 H C
            //On memory map row Bx column xA subtract register D from register A, but don't save value just use to set flags
            cmp(r->d);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xBB):
        {
            // CP A, E 1  4 Z 1 H C
            //On memory map row Bx column xB subtract register E from register A, but don't save value just use to set flags
            cmp(r->e);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xBC):
        {
            // CP A, H 1  4 Z 1 H C
            //On memory map row Bx column xC subtract register H from register A, but don't save value just use to set flags
            cmp(r->h);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xBD):
        {
            // CP A, L 1  4 Z 1 H C
            //On memory map row Bx column xD subtract register L from register A, but don't save value just use to set flags
            cmp(r->l);
            r->PC += 1;
            cycle = 4;
            break;
        }
        
        case (0xBE):
        {
            // CP A, [HL] 1  8 Z 1 H C
            //On memory map row Bx column x8 subtract value at address in register HL from register A, but don't save value just use to set flags
            cmp(b->read_8(r->hl));
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0xBF):
        {
            // CP A, A 1  4 Z 1 H C
            //On memory map row Bx column xF subtract register A from register A, but don't save value just use to set flags
            cmp(r->a);
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xC0):
        {
            // RET NZ 1  20/8 
            //On memory map row Cx column x0 set PC register to the first 2 bytes popped of the the stack
            bool z =  (r->f & FLAG_Z) ? 1:0;
            if(!z)
            {
                r->PC = pop16();
                cycle = 20;
            }
            else
            {
                r->PC += 1;
                cycle = 8;
            }
            break;
        }

        case (0xC1):
        {
            // POP BC 1  12 - - - -
            //On memory map row Cx column x1 pop 16 bit value of stack, put that value in BC register
            r->bc = pop16();
            r->PC += 1;
            cycle = 12;
            break;
        }

        case (0xC2):
        {
            // JP NZ, a16 3  16/12 - - - -
            //On memory map row Cx column x2 read address from PC and jump to that set PC to that address if Z flag not set
            bool z =  (r->f & FLAG_Z) ? 1:0;
            if(!z)
            {
                uint16_t address = b->read_16(r->PC + 1);
                r->PC = address;
                cycle = 16;
            }
            else
            {
                r->PC += 3;
                cycle = 12;
            }
            break;
        }

        case 0xC3: // JP a16
        {
            // JP a16 3  16 - - - -
            //On memory map row Cx column x3 read address from PC and jump to that set PC to that address
            r->PC = b->read_16(r->PC + 1);
            cycle = 16;
            break;
        }

        case 0xC4:
        {
            // CALL NZ, a16 3  24/12 - - - -
            //On memory map row Cx column x4 if z flag not set push the next instruction (r->pc + 3) to the stack and set target address to PC
            bool z =  (r->f & FLAG_Z) ? 1:0;
            uint16_t target_address = b->read_16(r->PC + 1);
            if(!z)
            {
                push(r->PC + 3);
                r->PC  = target_address;
                cycle = 24;
            }
            else
            {
                r->PC += 3;
                cycle = 12;
            }
            break;
        }

        case (0xC5):
        {
            // PUSH BC 1  16  - - - -
            //On memory map row Cx column x5 push register BC to stack
            push(r->bc);
            r->PC += 1;
            cycle = 16;
            break;
        }

        case (0xC6):
        {
            // ADD A, n8 2  8 Z 0 H C
            //On memory map row Cx column x6 fetch 8 bit value and add to register A
            uint8_t val = fetch_8();
            add(val);
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0xC7):
        {
            // RST $00 1  16 - - - -
            //On memory map row Cx column x7 push PC + 1 to stack and set PC to 0x00
            push(r->PC + 1);
            r->PC = 0x00;
            cycle = 16;
            break;
        }

        case (0xC8):
        {
            // RET Z 1  20/8 
            //On memory map row Cx column x8 set PC register to the first 2 bytes popped of the the stack
            bool z =  (r->f & FLAG_Z) ? 1:0;
            if(z)
            {
                r->PC = pop16();
                cycle = 20;
            }
            else
            {
                r->PC += 1;
                cycle = 8;
            }
            break;
        }

        case (0xC9):
        {
            // RET 1  16  - - - -
            //On memory map row column x9 set PC register to the first 2 bytes popped off the stack don't check z flag
            r->PC = pop16();
            cycle = 16;
            break;
        }

        case (0xCA):
        {
            // JP Z, a16 3  16/12 - - - -
            //On memory map row Cx column xA read address from PC and jump to that set PC to that address if Z flag set
            bool z =  (r->f & FLAG_Z) ? 1:0;
            if(z)
            {
                uint16_t address = b->read_16(r->PC + 1);
                r->PC = address;
                cycle = 16;
            }
            else
            {
                r->PC += 3;
                cycle = 12;
            }
            break;
        }

        case (0xCB):
        {
            // PREFIX 1  4 - - - -
            //On memory map row Cx column xB have to call CB opcode function for a instruction seperate from this
            uint8_t extendedOp = fetch_8();
            r->PC += 2; 
            cycle = executeCBOpcode(extendedOp);
            break;
        }

        case (0xCC):
        {
            // CALL Z, a16 3  24/12 - - - -
            //On memory map row Cx column xC if z flag set push the next instruction (r->pc + 3) to the stack and set target address to PC
            bool z =  (r->f & FLAG_Z) ? 1:0;
            uint16_t target_address = b->read_16(r->PC + 1);
            if(z)
            {
                push(r->PC + 3);
                r->PC  = target_address;
                cycle = 24;
            }
            else
            {
                r->PC += 3;
                cycle = 12;
            }
            break;
        }

        case (0xCD):
        {
            // CALL a16 3  24/12 - - - -
            //On memory map row Cx column x4 push the next instruction (r->pc + 3) to the stack and set target address to PC don't check Z flag
            uint16_t target_address = b->read_16(r->PC + 1);
            push(r->PC + 3);
            r->PC  = target_address;
            cycle = 24;
            break;
        }

        case (0xCE):
        {
                // ADC A, n8 2  8 Z 0 H C
            //On memory map row Cx column xE fetch 8 bit value and add to register A plus c_in
            uint8_t val = fetch_8();
            adc(val);
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0xCF):
        {
            // RST $08 1  16 - - - -
            //On memory map row Cx column xF push PC + 1 to stack and set PC to 0x08
            push(r->PC + 1);  
            r->PC = 0x08; 
            cycle = 16;
            break;
        }

        case (0xD0):
        {
            // RET NC 1  20/8 
            //On memory map row Dx column x0 set PC register to the first 2 bytes popped of the the stack
            bool c =  (r->f & FLAG_C) ? 1:0;
            if(!c)
            {
                r->PC = pop16();
                cycle = 20;
            }
            else
            {
                r->PC += 1;
                cycle = 8;
            }
            break;
        }

        case (0xD1):
        {
            // POP DE 1  12 - - - -
            //On memory map row Dx column x1 pop 16 bit value of stack, put that value in DE register
            r->de = pop16();
            r->PC += 1;
            cycle = 12;
            break;
        }

        case (0xD2):
        {
            // JP NX, a16 3  16/12 - - - -
            //On memory map row Cx column x2 read address from PC and jump to that set PC to that address if Z flag not set
            bool c =  (r->f & FLAG_C) ? 1:0;
            if(!c)
            {
                uint16_t address = b->read_16(r->PC + 1);
                r->PC = address;
                cycle = 16;
            }
            else
            {
                r->PC += 3;
                cycle = 12;
            }
            break;
        }

        case (0xD4):
        {
            // CALL NC, a16 3  24/12 - - - -
            //On memory map row Dx column x4 if z flag not set push the next instruction (r->pc + 3) to the stack and set target address to PC
            bool c =  (r->f & FLAG_C) ? 1:0;
            uint16_t target_address = b->read_16(r->PC + 1);
            if(!c)
            {
                push(r->PC + 3);
                r->PC  = target_address;
                cycle = 24;
            }
            else
            {
                r->PC += 3;
                cycle = 12;
            }
            break;   
        }

        case (0xD5):
        {
            // PUSH DE 1  16  - - - -
            //On memory map row Dx column x5 push register DE to stack
            push(r->de);
            r->PC += 1;
            cycle = 16;
            break;
        }

        case (0xD6):
        {
            // SUB A, n8 2  8 Z 0 H C
            //On memory map row Dx column x6 fetch 8 bit value and subtract to register A
            uint8_t val = fetch_8();
            sub(val);
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0xD7):
        {
            // RST $10 1  16 - - - -
            //On memory map row Dx column x7 push PC + 1 to stack and set PC to 0x10
            push(r->PC + 1);
            r->PC = 0x10;
            cycle = 16;
            break;
        }

        case (0xD8):
        {
            // RET Z 1  20/8 
            //On memory map row Dx column x8 set PC register to the first 2 bytes popped of the the stack
            bool c =  (r->f & FLAG_C) ? 1:0;
            if(c)
            {
                r->PC = pop16();
                cycle = 20;
            }
            else
            {
                r->PC += 1;
                cycle = 8;
            }
            break;
        }

        case (0xD9):
        {
            // RET 1  16  - - - -
            //On memory map row column x9 set PC register to the first 2 bytes popped off the stack don't check z flag
            r->PC = pop16();
            cycle = 16;
            ime = true;
            break;
        }

        case (0xDA):
        {
            // JP C, a16 3  16/12 - - - -
            //On memory map row Cx column xA read address from PC and jump to that set PC to that address if Z flag set
            bool c =  (r->f & FLAG_C) ? 1:0;
            if(c)
            {
                uint16_t address = b->read_16(r->PC + 1);
                r->PC = address;
                cycle = 16;
            }
            else
            {
                r->PC += 3;
                cycle = 12;
            }
            break;
        }

        case (0xDC):
        {
            // CALL C, a16 3  24/12 - - - -
            //On memory map row Dx column xC if z flag not set push the next instruction (r->pc + 3) to the stack and set target address to PC
            bool c =  (r->f & FLAG_C) ? 1:0;
            uint16_t target_address = b->read_16(r->PC + 1);
            if(c)
            {
                push(r->PC + 3);
                r->PC  = target_address;
                cycle = 24;
            }
            else
            {
                r->PC += 3;
                cycle = 12;
            }
            break; 
        }

        case (0xDE):
        {
            // SBC A, n8 2  8 Z 0 H C
            //On memory map row Dx column x6 fetch 8 bit value and subtract that and c_in from register A
            uint8_t val = fetch_8();
            sbc(val);
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0xDF):
        {
            // RST $18 1  16 - - - -
            //On memory map row Dx column xF push PC + 1 to stack and set PC to 0x18
            push(r->PC + 1);
            r->PC = 0x18;
            cycle = 16;
            break;
        }

        case (0xE0):
        {
            // LDH [a8], A 2  12 - - - -
            //On memory map row Ex column x0 load immediate next 8 bit val after opcodes and write the contents of register A to 0xFF00 + value
            uint8_t a = b->read_8(r->PC + 1);
            b->write_8(0xFF00 + a,r->a);
            r->PC += 2;
            cycle = 12;
            break;
        }
        
        case (0xE1):
        {
            // POP HL 1  12 - - - -
            //On memory map row ex column x1 pop 16 bit value of stack, put that value in HL register
            r->hl = pop16();
            r->PC += 1;
            cycle = 12;
            break;
        }

        case (0xE2):
        {
            // LDH [C], A 1  8 - - - -
            //On memory map row Ex column x2 load immediate next 8 bit val after opcodes and write the contents of register A to 0xFF00 + register C
            b->write_8(0xFF00 + r->c,r->a);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0xE5):
        {
            // PUSH HL 1  16  - - - -
            //On memory map row Ex column x5 push register DE to stack
            push(r->hl);
            r->PC += 1;
            cycle = 16;
            break;
        }
        
        case (0xE6):
        {
            // AND A, n8 2  8 Z 0 1 0
            //On memory map row Ex column x6 fetch 8 bit value and bitwise and it with register A
            uint8_t val = fetch_8();
            and_a(val);
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0xE7):
        {
            // RST $20 1  16 - - - -
            //On memory map row Ex column x7 push PC + 1 to stack and set PC to 0x20
            push(r->PC + 1);
            r->PC = 0x20;
            cycle = 16;
            break;
        }

        case (0xE8):
        {
            // ADD SP, e8 2  16 0 0 H C
            //On memory map row Ex column x8 add signed 8-bit value at the byte after PC register to Stack Pointer
            uint16_t sp = r->SP;
            int8_t e8 = (int8_t)b->read_8(r->PC + 1);

            uint16_t result = sp + e8;

            r->SP = (uint16_t)result;

            setZ(false); //Set Z flag if register A value is now 0
            setN(false); //Set N flag to 0
            setH((sp & 0xF) + ((uint8_t)e8 & 0xF) > 0xF); //Set H flag to 1 if the low 4 bits added together are greater the 4 bits
            setC(((sp & 0xFF) + ((uint8_t)e8 & 0xFF)) > 0xFF); //Set C flag to 1 if result is greater than 8 bits 

            r->PC += 2;
            cycle = 16;
            break;

        }

        case (0xE9):
        {
            // JP HL 1  4 - - - -
            //On memory map row Ex column x9 jump PC register to HL
            r->PC = r->hl;
            cycle = 4;
            break;
        }

        case (0xEA):
        {
            // LDH [a16], A 3  16 - - - -
            //On memory map row Ex column xA load immediate next 16 bit val after opcodes and write the contents of register A to 0xFF00 + value
            uint16_t a = b->read_16(r->PC + 1);
            b->write_8( a,r->a);
            r->PC += 3;
            cycle = 16;
            break;
        }

        case (0xEE):
        {
            // XOR A, n8 2  8 Z 0 1 0
            //On memory map row Ex column xE fetch 8 bit value and XOR it with register A
            uint8_t val = fetch_8();
            xor_a(val);
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0xEF):
        {
            // RST $28 1  16 - - - -
            //On memory map row Ex column xF push PC + 1 to stack and set PC to 0x28
            push(r->PC + 1);
            r->PC = 0x28;
            cycle = 16;
            break;
        }

        case (0xF0):
        {
            // LDH A, [a8] 2  12 - - - -
            //On memory map row Fx column x0 load immediate next 8 bit val after opcodes and write the contents of 0xFF00 + value to 
            uint8_t a = b->read_8(r->PC + 1);
            r->a = b->read_8(0xFF00 + a);
            r->PC += 2;
            cycle = 12;
            break;

        }

        case (0xF1):
        {
            // POP AF 1  12 Z N H C
            ////On memory map row Fx column x1 pop of stack set af register to that value, set flags based on bit 4 - 7 in the register
            r->af = pop16() & 0xFFF0;
            uint8_t flags = r->af & 0xF0;

            setZ(flags & FLAG_Z ? true:false);
            setN(flags & FLAG_N ? true:false);
            setH(flags & FLAG_H ? true:false);
            setC(flags & FLAG_C ? true:false);

            r->PC += 1;
            cycle = 12;
            break;
        }

        case (0xF2):
        {
                // LDH A, [C] 1, 8 - - - -
            //On memory map row Fx column x2 read value from address 0xFF00 + register c and set value to register A
            r->a = b->read_8(0xFF00 + r->c);
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0xF3):
        {
            // DI 1  4 - - - -
            //On memory map row Fx column x3 disable ime
            ime = false;
            ime_delay = 0;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xF5):
        {
            // PUSH AF 1  16 - - - -
            //On memory map row Fx column x5 push AF to stack
            push(r->af);
            r->PC += 1;
            cycle = 16;
            break;
        }

        case (0xF6):
        {
            // OR A, n8 2  8 Z 0 0 0
            //On memory map fetch 8 bit value and or it with register A and set register a to result
            uint8_t val = fetch_8();
            or_a(val);
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0xF7):
        {
            // RST $30 1  16 - - - -
            //On memory map row Fx column x7 push PC + 1 to stack and set PC to 0x30
            push(r->PC + 1);
            r->PC = 0x30;
            cycle = 16;
            break;
        }

        case (0xF8):
        {
            // LD HL, SP + e8 2  12 0 0 H C
            //On memory map row Fx column x8 add signed 8-bit value at the byte after PC register to Stack Pointer set HL register to result
            uint16_t sp = r->SP;
            int8_t e8 = (int8_t)b->read_8(r->PC + 1);
            uint16_t result = sp + e8;
            r->hl = result;

            setZ(false); //Set Z flag if register A value is now 0
            setN(false); //Set N flag to 0
            setH((sp & 0xF) + ((uint8_t)e8 & 0xF) > 0xF); //Set H flag to 1 if the low 4 bits added together are greater the 4 bits
            setC(((sp & 0xFF) + ((uint8_t)e8 & 0xFF)) > 0xFF); //Set C flag to 1 if result is greater than 8 bits 

            r->PC += 2;
            cycle = 12;
            break;
        }

        case (0xF9):
        {
            // LD SP, HL 1  8 - - - -
            //On memory map row Fx column x9 load register HL to stack pointer
            r->SP = r->hl;
            r->PC += 1;
            cycle = 8;
            break;
        }

        case (0xFA):
        {
            // LD A, [a16] 3  16 - - - -
            //On memory map row Fx column xA load immediate next 16 bit val after opcodes and write the contents of value to register A
            uint16_t a = b->read_16(r->PC + 1);
            r->a = b->read_8(a);
            r->PC += 3;
            cycle = 16;
            break;
        }

        case (0xFB):
        {
            // EI 1  4 - - - -
            //On memory map row Fx column xB enable ime
            ime_delay = 2;
            r->PC += 1;
            cycle = 4;
            break;
        }

        case (0xFE):
        {
            // CP A, n8 2  8 Z 1 H C
            //On memory map row Ex column xE fetch 8 bit value and compare it with register A
            uint8_t val = fetch_8();
            cmp(val);
            r->PC += 2;
            cycle = 8;
            break;
        }

        case (0xFF):
        {
            // RST $38 1  16 - - - -
            //On memory map row Ex column xF push PC + 1 to stack and set PC to 0x38
            push(r->PC + 1);
            r->PC = 0x38;
            cycle = 16;
            break;
        }


    }

    if(ime_delay > 0)
    {
        ime_delay--;
        if(ime_delay == 0) ime = true;
    }

    if(hadHalt)
    {
        r->PC -= 1;
    }

    return cycle;
}
