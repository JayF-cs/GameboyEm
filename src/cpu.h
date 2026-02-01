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
        uint64_t tCycles = 0;

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
            
            uint8_t old = val;
            val -= 1;

            setZ(val == 0);
            setN(true);
            setH((old & 0x0F) == 0x00);

            return val;
        }
        
        void addHL(uint16_t reg){
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

        uint8_t step(Bus *b, registers *r){

            //Read the opCode from PC register
            uint8_t opCode = b->read_8(r->PC);

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
                    b->write_16(r->PC + 1, r->SP);
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
                    uint16_t bit0 = r->a;
                    r->a = r->a >> 1;
                    bit0 = bit0 << 7;
                    r->a |= bit0;

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
                    inc8(r->d);
                    r->PC += 1;
                    cycle = 4;
                    break;
                }

                case(0x15):
                {
                    // DEC D 1 4 Z 1 H -
                    //On memory map row 1x column x5 decrement d register and call proper flags
                    dec8(r->d);
                    r->PC += 1;
                    cycle = 4;
                    break;
                }

                case (0x16):
                {
                    // LD D n8 2 8 ----
                    //On memory map row 1x column x6 fetch 8 bit and write to D register
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
                    // JP e8 2 12
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
                    // JP NZ e8 2 12/8 (12 or 8) ----
                    //On memory map row 2x column x0 jump if z flag not set else do nothing
                    bool z =  (r->f & FLAG_Z) ? 1:0;
                    if(!z)
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
                    r->hl = inc8(r->hl);
                    r->PC += 1;
                    cycle = 4;
                    break;
                }

                case (0x25):
                {
                    // DEC H 1 4 Z 1 H -
                    //On memory map row 2x column x5 decrement H register
                    r->hl = dec8(r->hl);
                    r->PC += 1;
                    cycle = 4;
                    break;
                }

                case (0x26):
                {
                    // LD H n8 2 8 ----
                    //On memory map row 2x column x6 fetch 8 bit value and assign to H register
                    r->hl = fetch_8();
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
                    // JP Z e8 2 12/8 (12 or 8) ----
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
                    // JP NC e8 2 12/8 (12 or 8) ----
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
                    // JP C e8 2 12/8 (12 or 8) ----
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
            }

            return 0;
        }



};