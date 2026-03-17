#pragma once
#include <cstdint>

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
