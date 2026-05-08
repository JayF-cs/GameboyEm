#include "ppu.h"

uint8_t ppu::getTileFromVRAM(int currentX, int pixelNum)
{
    /*////////////////////////////
    Calculate indexes in tile map
    Tile map is 256 x 256 pixels
    Each tile is 8 pixels wide
    Map Index Y = ((SCY + LY) % 256) / 8
    Map Index X = ((SCX + current X position) % 256) / 8
    The Pixel Row you are on = (SCY + LY) % 8
    /////////////////////////////*/
    int tileMapIndexY = ((ppu::b->read_8(0xFF42) + ppu::b->read_8(0xFF44)) % 256)/8;
    int tileMapIndexX = ((ppu::b->read_8(0xFF43) + currentX) % 256)/8;
    int pixelRow = (ppu::b->read_8(0xFF42) + ppu::b->read_8(0xFF44)) % 8;

    //Get where to get tile from in VRAM
    uint16_t tileLocation = (ppu::b->read_8(0xFF40) >> 3) & 1 ? 0x9C00:0x9800;
    uint8_t tileIndex = ppu::b->read_8(tileLocation + (tileMapIndexY * 32) + tileMapIndexX);

    uint16_t tileAddr;
    if((ppu::b->read_8(0xFF40) >> 4) & 1)
    {
        tileAddr = 0x8000 + tileIndex * 16;
    }
    else
    {
        tileAddr = 0x9000 + (int8_t)tileIndex * 16;
    }

    //Since there are 2 one byte per row you must add 2 times pixel value in order to also skip the second row
    tileAddr += pixelRow * 2;
    //Grab both sets of bytes for the row
    uint8_t low = ppu::b->read_8(tileAddr);
    uint8_t high = ppu::b->read_8(tileAddr + 1);

    //Bit manipulation to get specified pixel in low to be LSB and specified pixel in high to be bit 1 
    low = (low >> (7 - pixelNum)) & 1;
    high = (high >> (7 - pixelNum)) & 1;
    high = high << 1;
    //Or high and low to get either 0 1 2 or 3 and return it
    return high | low;
}

uint8_t ppu::getObjFromVRAM(object obj, int pCol, int pRow)
{
    uint8_t objHeight = (ppu::b->read_8(0xFF40) & 0x4) >> 2 ? 16:8;

    if((obj.flags & 0x20))
    {
        pCol = 7 - pCol;
    }

    if(obj.flags & 0x40)
    {
        pRow = objHeight - pRow - 1;
    }

    uint16_t spriteRow = 0x8000 +  obj.tIndex * 16 + pRow * 2;

    uint8_t low = ppu::b->read_8(spriteRow);
    uint8_t high = ppu::b->read_8(spriteRow + 1);

    low = (low >> (7 - pCol)) & 1;
    high = (high >> (7 - pCol)) & 1;
    high = high << 1;
    //Or high and low to get either 0 1 2 or 3 and return it
    return high | low;
}

uint8_t ppu::priotiryChecker(uint8_t BGP_index, uint8_t OBJ_index, uint8_t flags)
{
    if(!(ppu::b->read_8(0xFF40) & 1))
    {
        return OBJ_index;
    }

    if(OBJ_index == 0)
    {
        return BGP_index;
    }

    if(BGP_index == 0)
    {
        return OBJ_index;
    }

    if(flags >> 7)
    {
        return BGP_index;
    }

    return OBJ_index;
}

uint8_t ppu::getColor(uint16_t paletteAddr, uint8_t paletteIndex)
{
    return ((b->read_8(paletteAddr) >> paletteIndex * 2) & 0x3);
}

void ppu::compareLY()
{
    //IF LY == LYC set bit 2 and check if bit 6 oF STAT is set to see if need to write to IF
    if(ppu::b->read_8(0xFF44) == ppu::b->read_8(0xFF45))
    {
        ppu::b->write_8(0xFF45, ppu::b->read_8(0xFF45) | 0x4);

        if(ppu::b->read_8(0xFF41) & 0x40)
        {
            ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x2);
        }

    }
}

void ppu::mode0()
{
    if(ppu::b->read_8(0xFF41) & 0x8)
    {
        ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x2);
    }
}

void ppu::mode1()
{
    //Set VBlank bit in IF
    ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x1);

    //Set LCD bit in IF if Mode bit 1 in STAT is set
    if(ppu::b->read_8(0xFF41) & 0x10)
    {
        ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x2);
    }
}

void ppu::mode2()
{
    //Check if MODE 2 interrupt bit is set in STAT
    if(ppu::b->read_8(0xFF41) & 0x20)
    {
        ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x2);
    }

    //Track dots, but they kind of meaningless here since loop always goes 40 times
    ppu::dots = 0;

    //Clear the vector previous objects
    ppu::objects.clear();

    //Start of OAM
    int base = 0xFE00;

    //Keep track of number objects in vector
    int count = 0;
    
    //Loop through all of OAH
    for(int i = 0; i < 0xA0; i += 4)
    {
        //Check if max objects are in vector
        if(count == 10)
        {
            dots += 2;
            continue;
        }

        //Get objects y position
        uint8_t objY = ppu::b->read_8(base + i) - 16;

        //Get objects height, either 8 or 16
        uint8_t objHeight = (ppu::b->read_8(0xFF40) & 0x4) >> 2 ? 16:8;

        //Check if the LY is at the start of the object and not past it height
        if(ppu::b->read_8(0xFF44) >= objY && ppu::b->read_8(0xFF44) < objY + objHeight)
        {
            //Create object and add it to vector
            uint8_t objX = ppu::b->read_8(base + i + 1);
            uint8_t objInd = ppu::b->read_8(base + i + 2);
            uint8_t objFlags = ppu::b->read_8(base + i + 3);
            object o = {objY, objX, objInd, objFlags};
            ppu::objects.push_back(o);
            count++;
        }

        //Increment dots
        dots += 2;
    }

}

void ppu::initialEnqueues()
{
    uint8_t startingX = ppu::b->read_8(0xFF43);

    for(int i = 0; i < 8; i++)
    {
        ppu::bgFIFO.push(getTileFromVRAM(startingX + i, (startingX + i) % 8));
        int size = ppu::objects.size();
        for(int j = 0; j < size; j++)
        {
            if((ppu::objects[j].x - (startingX + i) < 8) && (startingX + i) < ppu::objects[j].x)
            {
                uint8_t obColor = getObjFromVRAM(ppu::objects[j], ppu::b->read_8(0xFF44) + ppu::objects[j].x - 8 ,ppu::b->read_8(0xFF44) + ppu::objects[j].y);
                uint16_t obPalette = (ppu::objects[j].flags & 0x8) ? 0xFE49:0xFE48;
                objPixel p = {obColor, obPalette, ppu::objects[j].flags};
                ppu::obFIFO.push(p);
            }
        }
    }
}



void ppu::mode3()
{

}