#include <iostream>
#include <cstdint>
#include "Bus.h"

struct registers
{
    struct 
    {
        union
        {
            struct 
            {
                uint8_t f; //Low byte
                uint8_t a; //High byte
            };

            uint16_t af;
            
        };
    };
    
    struct 
    {
        union
        {
            struct 
            {
                uint8_t c; //Low byte
                uint8_t b; //High byte
            };

            uint16_t bc;
            
        };
    };

    struct 
    {
        union
        {
            struct 
            {
                uint8_t e; //Low byte
                uint8_t d; //High byte
            };

            uint16_t de;
            
        };
    };

    struct 
    {
        union
        {
            struct 
            {
                uint8_t l; //Low byte
                uint8_t h; //High byte
            };

            uint16_t hl;
            
        };
    };

    uint16_t PC;
    uint16_t SP;
};

class CPU
{
    private:

        //Create pointers to a Bus class object and a registers struct object
        Bus *b;
        registers *r;

        const uint8_t FLAG_Z = 1 << 7;
        const uint8_t FLAG_N = 1 << 6;
        const uint8_t FLAG_H = 1 << 5;
        const uint8_t FLAG_C = 1 << 4;

        bool cpu_stopped = false; 
    
    public:

        //Initialize the CPU object
        CPU(Bus *bus, registers *regPtr) : b(bus), r(regPtr) {}

        //Helper function to read a 16 bit value from memory
        uint16_t fetch_16(){

            uint16_t val = b->read_16(r->PC + 1);
            return val;

        }

        uint8_t fetch_8(){

            uint8_t val = b->read_8(r->PC + 1);
            return val;

        }

        void setFlag(uint8_t flag, bool val){
                
            if(val){

                r->f |= flag;
            }else{

                r->f &= ~flag; 
            }

        }

        void setZ(bool val){

            setFlag(FLAG_Z, val);
        }

        void setN(bool val){

            setFlag(FLAG_N, val);
        }

        void setH(bool val){

            setFlag(FLAG_H, val);
        }

        void setC(bool val){

            setFlag(FLAG_C, val);
        }

        uint8_t inc8(uint8_t val){

            uint8_t old = val;
            val += 1;

            setZ(val == 0);
            setN(false);
            setH((old & 0x0F) == 0x0F);
            return val;
        }

        uint8_t dec8(uint8_t val){
            val -= 1;

            setZ(val == 0);
            setN(true);
            setH((val & 0x0F) == 0x00);

            return val;
        }
        
        uint8_t step(Bus *b, registers *r){

            //Read the opCode from PC register
            uint8_t opCode = b->read_8(r->PC);
            
            //Based on pointer content preform an operation
            switch(opCode){
                
                case(0x0):
                {
                    //On memory map row 0x column x0 NOP increment PC by 1 byte
                    r->PC += 1;

                    break;
                }

                case(0x01):
                {
                    // LD BC, n16 3 12 ----
                    //On memory map row 0x column x1 fetch 16 bit int and assign it to bc register
                    r->bc = fetch_16();
                    r->PC += 3;
                    break;
                }

                case(0x02):
                {
                    // LD [BC], A 1 8 ----
                    //On memory map row 0x column x2 write contents of A to address stored in bc registry
                    b->write_8(r->bc,r->a);
                    r->PC += 1;

                    break; 
                }
                    
                case(0x03):
                {
                    // INC BC 1 8 ----
                    //On memory map row 0x column x3 increment the BC register by one bit 
                    r->bc += 1;
                    r->PC += 1;
                    break;
                }

                case(0x04):
                {
                    // INC B 1 4 Z 0 H -
                    //On memory map row 0x column x4 increment B register set z n and h flags accordingly
                    r->b = inc8(r->b);
                    r->PC += 1;

                    break;

                }
                    
                case(0x05):
                {
                    // DEC B 1 4 Z 1 H -
                    //On memory map row 0x column x5 decrement B register set z n and h flags accordingly
                    r->b = dec8(r->b);
                    r->PC += 1;

                    break;
                }    
                
                case(0x06):
                {
                    //LD B n8 2 8 ----
                    //On memory map row 0x column x6 fetch 8 bit int and assign to b register;

                    r->b = fetch_8();
                    r->PC += 2;

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

                    break;
                }


                case(0x08):
                {
                    // LD [a16], SP 3 20 ----
                    //On memory map row 0x column x8 write stack pointer to address after opcode
                    b->write_16(r->PC + 1, r->SP);
                    r->PC += 3;
                    break;
                }
                    
                case(0x09):
                {
                    // ADD HL, BC 1 8 - 0 H C
                    //On memory map row 0x column x9 add value in BC register to HL register
                    uint16_t val1 = r->hl;
                    uint16_t val2 = r->bc;
                    //Type cast the values to 32 bit add together and store in result
                    uint32_t result = (uint32_t)val1 + (uint32_t)val2;
                    //Type cast the result back to 16 bit
                    r->hl = (uint16_t)result;

                    setN(false);
                    //Check if lower 12 bits of hl and bc added together is greater then 12 bits
                    setH(((val1 & 0x0FFF) + (val2 & 0x0FFF)) > 0x0FFF);
                    //See if result if greater then 16 bits
                    setC(result > 0xFFFF);
                    break;
                }
                    
                case(0x0A):
                {
                    // LD A, [BC] 1 8 ----
                    //On memory map row 0x column xA load the the value stored in at the memory address in the BC register to register A
                    r->a = b->read_8(r->bc);
                    r->PC += 1;
                    break;
                }

                case(0x0B):
                {
                    // DEC BC 1 8 ----
                    //On memory map row 0x column xB decrement BC register
                    r->bc -= 1;
                    r->PC += 1;
                    break;
                }

                case(0x0C):
                {
                    // INC C 1 4 Z 0 H -
                    //On memory map row 0x column xC increment C register and raise z n and h flags based on result
                    r->c = inc8(r->c);
                    r->PC += 1;

                    break;
                }

                case(0x0D):
                {
                    // DEC C 1 4 Z 1 H -
                    //One memory map row 0x column xD decrement register C and raise z n and h flags based on result
                    r->c = dec8(r->c);
                    r->PC += 1;
                    break;
                }

                case(0x0E):
                {
                    //LD C n8 2 8 ----
                    //On memory map row 0x column xE fetch 8 bit int and assign to C register;
                    r->c = fetch_8();
                    r->PC += 2;
                    break;
                }
                    
                case(0x0F):
                {
                    //RRCA 1 4 0 0 0 C
                    uint16_t bit0 = r->a;
                    r->a = r->a >> 1;
                    bit0 = bit0 << 7;
                    r->a |= bit0;

                    setZ(false);
                    setN(false);
                    setH(false);
                    setC(bit0);

                    r->PC += 1;
                    break;
                }

                case(0x10):
                {
                    // STOP n8 2 4 ----
                    //On memory map row 1x and column x0 stop cpu
                    r->PC += 2;
                    cpu_stopped = true;
                    break;
                }

                case (0x11):
                {
                    // LD DE n16 3 12 ----
                    //On memory map row 1x column x1 fetch 16 bit and assign to DE register
                    r->de = fetch_16();
                    r->PC += 3;
                    break;

                }

                case (0x12):
                {
                    //LD [DE], A 1 8 ----
                    //On memory map row 1x column x2 write value in register A to address stored in DE register
                    b->write_8(r->de, r->a);
                    r->PC += 1;
                    break;
                }

                case(0x13):
                {
                    // INC DE 1 8 ----
                    //On memory map row 1x column x3 increment de register
                    r->de += 1;
                    r->PC += 1;
                    break;

                }

                case (0x14):
                {
                    // INC D 1 4 Z 0 H -
                    //On memory map row 1x column x4 increment d register and call proper flags
                    inc8(r->d);
                    r->PC += 1;
                    break;
                }

                case(0x15):
                {
                    // DEC D 1 4 Z 1 H -
                    //On memory map row 1x column x5 decrement d register and call proper flags
                    dec8(r->d);
                    r->PC += 1;
                    break;
                }

                case (0x16):
                {
                    // LD D n8 2 8 ----
                    //On memory map row 1x column x6 fetch 8 bit and write to D register
                    r->d = fetch_8();
                    r->PC += 2;
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

                    break;

                }
                    
            }

            return 0;
        }



};