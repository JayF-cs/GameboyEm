#include "ppu.h"
#include "Bus.h"

uint8_t ppu::getTileFromVRAM(int currentX, int pixelNum)
{
    /*////////////////////////////
    Calculate indexes in tile map
    Tile map is 256 x 256 pixels
    Each tile is 8 pixels wide
    For window:
    Tile Map Index Y = (window's LY % 256) / 8
    Tile Map Index X = (Xpos in winodw % 256) / 8
    The Pixel Row of the tile you are on = (window's ly) % 8
    For background:
    Tile Map Index Y = ((SCY + LY) % 256) / 8
    Tile Map Index X = ((SCX + current X position) % 256) / 8
    The Pixel Row of the tile you are on = (SCY + LY) % 8
    /////////////////////////////*/
    int tileMapIndexY;
    int tileMapIndexX;
    int pixelRow;
    int pixelCol;
    uint16_t tileLocation;

    int windowX = currentX + pixelNum - (scx % 8);

    if((lcdc & 0x20) != 0 && window_trigger && (windowX >= (wx - 7)))
    {
        tileMapIndexY = window_ly/8;
        tileMapIndexX = (windowX - (wx - 7))/8;
        pixelRow = window_ly % 8;
        pixelCol = (windowX - (wx - 7)) % 8;
        tileLocation = (lcdc >> 6) & 1 ? 0x9C00:0x9800;
    }
    else
    {
        tileMapIndexY = ((scy + ly) % 256)/8;
        tileMapIndexX = ((scx + currentX) % 256)/8;
        pixelRow = (scy + ly) % 8;
        pixelCol = pixelNum;
        tileLocation = (lcdc >> 3) & 1 ? 0x9C00:0x9800;
    }

    //Get where to get tile from in VRAM
    uint8_t tileIndex = b->read_8(tileLocation + (tileMapIndexY * 32) + tileMapIndexX);

    uint16_t tileAddr;
    if((lcdc >> 4) & 1)
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
    uint8_t low = b->read_8(tileAddr);
    uint8_t high = b->read_8(tileAddr + 1);

    //Bit manipulation to get specified pixel in low to be LSB and specified pixel in high to be bit 1 
    low = (low >> (7 - pixelCol)) & 1;
    high = (high >> (7 - pixelCol)) & 1;
    high = high << 1;
    //Or high and low to get either 0 1 2 or 3 and return it
    return high | low;
}

uint8_t ppu::getObjFromVRAM(object obj, int pCol, int pRow)
{
    uint8_t objHeight = (lcdc & 0x4) >> 2 ? 16:8;

    if(objHeight == 16) obj.tIndex &= 0xFE;

    if((obj.flags & 0x20))
    {
        pCol = 7 - pCol;
    }

    if(obj.flags & 0x40)
    {
        pRow = objHeight - pRow - 1;
    }

    uint16_t spriteRow = 0x8000 +  obj.tIndex * 16 + pRow * 2;

    uint8_t low = b->read_8(spriteRow);
    uint8_t high = b->read_8(spriteRow + 1);

    low = (low >> (7 - pCol)) & 1;
    high = (high >> (7 - pCol)) & 1;
    high = high << 1;
    //Or high and low to get either 0 1 2 or 3 and return it
    return high | low;
}

bool ppu::priotiryChecker(uint8_t BGP_index, uint8_t OBJ_index, uint8_t flags)
{
    if(OBJ_index == 0)
    {
        return true;
    }

    if(!(lcdc & 0x1))
    {
        return false;
    }

    if((flags & 0x80) && (BGP_index != 0))
    {
        return true;
    }

    return false;
}

uint8_t ppu::getColor(uint16_t paletteAddr, uint8_t paletteIndex)
{
    return ((b->read_8(paletteAddr) >> paletteIndex * 2) & 0x3);
}

void ppu::frameColor(GBColors color, uint8_t row, uint8_t col)
{
    switch (color)
    {
        case GBColors::WHITE:
        {
            frame[row][col] = 0xE0F8D0FF;
            break;
        } 
        case GBColors::LIGHT_GRAY: 
        {
            frame[row][col] = 0x88C070FF;
            break;
        }
        case GBColors::DARK_GRAY:
        {
            frame[row][col] = 0x346856FF;
            break;
        } 
        case GBColors::BLACK:
        {
            frame[row][col] = 0x081820FF;
            break;
        } 
        default:
            break;
    }
}

void ppu::compareLY()
{
    //IF LY == LYC set bit 2 and check if bit 6 oF STAT is set to see if need to write to IF

    if(ly == lyc)
    {
        stat = stat | 0x4;

        if(stat & 0x40)
        {
            ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x2);
        }
    }
    else
    {
        stat = (stat & (~0x4));
    }
}

void ppu::mode0()
{
    if(stat & 0x8)
    {
        ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x2);
    }
}

void ppu::mode1()
{
    //Set VBlank bit in IF
    ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x1);

    //Set LCD bit in IF if Mode bit 1 in STAT is set
    if(stat & 0x10)
    {
        ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x2);
    }
}

void ppu::mode2()
{
    //Check if MODE 2 interrupt bit is set in STAT
    if(stat & 0x20)
    {
        ppu::b->write_8(0xFF0F, ppu::b->read_8(0xFF0F) | 0x2);
    }

    //Start of OAM
    int base = 0xFE00;

    //Keep track of number objects in vector
    int count = 0;
    
    //Loop through all of OAM
    for(int i = 0; i < 0xA0; i += 4)
    {
        //Check if max objects are in vector
        if(count == 10)
        {
            break;
        }

        //Get objects y position
        int objY = ppu::b->read_8(base + i) - 16;
        //Get objects height, either 8 or 16
        uint8_t objHeight = (lcdc & 0x4) >> 2 ? 16:8;

        //Check if the LY is at the start of the object and not past it height
        if(ly >= objY && ly < objY + objHeight)
        {
            //Create object and add it to vector
            uint8_t objX = ppu::b->read_8(base + i + 1);
            uint8_t objInd = ppu::b->read_8(base + i + 2);
            uint8_t objFlags = ppu::b->read_8(base + i + 3);
            object o = {objX, static_cast<int16_t>(objY), objInd, objFlags};
            ppu::objects.push_back(o);
            count++;
        }
    }
}

void ppu::initialEnqueues()
{

    //Enqueue first 8 pixels
    for(int i = 0; i < 8; i++)
    {
        int priorityPixel = -1;
        uint8_t winningColor = 0;

        //Get pixel from VRAM
        uint8_t bgPixel = (lcdc & 0x1) ? getTileFromVRAM(tileCount, i) : 0;
        bgFIFO.push(bgPixel);
        
        int size = objects.size();
        int screenX = tileCount + i - (scx % 8);
        //Check if object enable is on
        if(lcdc & 0x2)
        {
            for(int j = 0; j < size; j++)
            {
                //Check if current pixel is within the objects 8 x 8 bounds
                if(((screenX) - (objects[j].x - 8)  < 8) && (screenX) >= objects[j].x - 8)
                {
                    uint8_t obColor = getObjFromVRAM(objects[j], screenX - (objects[j].x - 8), ly - objects[j].y);

                    if(obColor != 0)
                    {
                        if(priorityPixel == -1 || objects[j].x < objects[priorityPixel].x) 
                        {
                            priorityPixel = j;
                            winningColor = obColor; // Save the non-transparent color
                        }
                    }
                }
            }
        }
        //Go through and check if there are any objects on this pixel
        if(priorityPixel != -1)
        {
            uint16_t obPalette = ((ppu::objects[priorityPixel].flags >> 4) & 0x01) ? 0xFF49 : 0xFF48;
            objPixel p = {winningColor, obPalette, ppu::objects[priorityPixel].flags};
            ppu::obFIFO.push(p);
        }

        //Push padding object pixel if no object pixel
        if(ppu::bgFIFO.size() != ppu::obFIFO.size())
        {
            objPixel p = {0, 0, 0};
            ppu::obFIFO.push(p);
        }
    }
    
    //Increase tile count
    ppu::tileCount += 8;

    //Get rid of pixels before SCX
    for(int i = 0; i < (scx % 8) ; i++)
    {
        ppu::bgFIFO.pop();
        ppu::obFIFO.pop();
    }
}

void ppu::pixelFetcher()
{
    //Fill up 8 pixels at a time
    for(int i = 0; i < 8; i++)
    {
        //Get pixel from VRAM
        uint8_t bgPixel = (lcdc & 0x1) ? getTileFromVRAM(ppu::tileCount, i) : 0;
        bgFIFO.push(bgPixel);
        int size = ppu::objects.size();

        uint8_t winningColor = 0;
        int priorityPixel = -1; 

        int screenX = tileCount + i - (scx % 8);
        //Check if object enable is on
        if(lcdc & 0x2)
        {
            for(int j = 0; j < size; j++)
            {
                //Check if current pixel is within the objects 8 x 8 bounds
                if(((screenX) - (objects[j].x - 8)  < 8) && (screenX) >= objects[j].x - 8)
                {
                    // 1. Fetch the color right here to check for transparency
                    uint8_t obColor = getObjFromVRAM(objects[j], screenX - (objects[j].x - 8), ly - objects[j].y);
                    
                    // 2. Only consider this sprite if its pixel is NOT transparent!
                    if(obColor != 0)
                    {
                        if(priorityPixel == -1 || objects[j].x < objects[priorityPixel].x) 
                        {
                            priorityPixel = j;
                            winningColor = obColor; // Save the non-transparent color
                        }
                    }
                }
            }
        }
        //Go through and check if there are any objects on this pixel
        if(priorityPixel != -1)
        {
            uint16_t obPalette = ((ppu::objects[priorityPixel].flags >> 4) & 0x01) ? 0xFF49 : 0xFF48;
            objPixel p = {winningColor, obPalette, ppu::objects[priorityPixel].flags};
            ppu::obFIFO.push(p);
        }

        //Push padding object pixel if no object pixel
        if(ppu::bgFIFO.size() != ppu::obFIFO.size())
        {
            objPixel p = {0, 0, 0};
            ppu::obFIFO.push(p);
        }
    }

    ppu::tileCount += 8;
}

void ppu::mode3()
{
    initialEnqueues();

    int numPix = 0;

    while(!(ppu::bgFIFO.empty()))
    {
        uint8_t bgp = bgFIFO.front();
        bgFIFO.pop();
        objPixel objp = obFIFO.front();
        obFIFO.pop();
        
        uint8_t pixel;

        //Check if BGP or OBJP have higher priority
        bool priority = priotiryChecker(bgp, objp.colorIndex, objp.flags);
        //Check
        if(priority)
        {
            pixel = getColor(0xFF47, bgp);
        }
        else
        {
            pixel = getColor(objp.palette, objp.colorIndex);
        }
        
        buffer[numPix++] = pixel;
    }

    while(numPix < 160)
    {
        pixelFetcher();

        for(int i = 0; i < 8; i++)
        {
            if(numPix >= 160) break;

            //Get a pixel from both queues
            uint8_t bgp = bgFIFO.front();
            bgFIFO.pop();
            objPixel objp = obFIFO.front();
            obFIFO.pop();

            uint8_t pixel;

            //Check if BGP or OBJP have higher priority
            bool priority = priotiryChecker(bgp, objp.colorIndex, objp.flags);
            //Check
            if(priority)
            {
                pixel = getColor(0xFF47, bgp);
            }
            else
            {
                pixel = getColor(objp.palette, objp.colorIndex);
            }
            
            buffer[numPix++] = pixel;
        }
    }

    if((lcdc & 0x20) != 0 && window_trigger && (wx - 7 < 160) && wx - 7 >= 0 ) window_ly++;
}

void ppu::setMode(PPU_modes NewMode){mode = NewMode;}

void ppu::setDots(){dots = 0;}

void ppu::setModeSTAT()
{
    stat = (stat & 0xFC)| static_cast<uint8_t>(mode);

    if(startUp) return;

    uint8_t IF = b->read_8(0xFF0F);

    switch (mode)
    {
        case PPU_modes::MODE_0:
            if(stat & (1 << 3)) b->write_8(0xFF0F, IF | (1 << 1)); //Trigger STAT interrupt
            break;

        case PPU_modes::MODE_1:
            b->write_8(0xFF0F, IF | 0x1); //Trigger VBLANK interrupt
            if(stat & (1 << 4)) b->write_8(0xFF0F, IF | (1 << 1)); //Trigger STAT interrupt
            break;
        
        case PPU_modes::MODE_2:
            if(stat & (1 << 5)) b->write_8(0xFF0F, IF | (1 << 1)); //Trigger STAT interrupt
            break;
        
        default:
            break;
    }
}

void ppu::tick(int cycles)
{
    if((lcdc & 0x80) == 0)
    {
        if(prevPowerState)  // just turned off this tick
        {
            for(int row = 0; row < 144; row++)
                for(int col = 0; col < 160; col++)
                    frameColor(GBColors::WHITE, row, col);

            r->update((uint32_t*)frame);
        }

        ly = 0;  // reset LY to 0 when LCD is off
        dots = 0;
        mode = PPU_modes::MODE_0;
        prevPowerState = 0;
        setModeSTAT();
        return;
    }

    //Check if ppu was just turned on
    if((lcdc & 0x80) && prevPowerState == false)
    {
        compareLY();
        prevPowerState = true;
        startUp = true;
        setModeSTAT();
        return;
    }

    dots += cycles;

    switch(mode)
    {
        case (PPU_modes::MODE_0):
        {
            if(startUp)
            {
                if(dots >= 80)
                {
                    dots -= 80;
                    startUp = false;
                    mode = PPU_modes::MODE_3;
                    tileCount = 0;
                    bgFIFO = {};
                    obFIFO = {};
                    setModeSTAT();
                    mode3();
                }
            }
            else if(dots >= 204)
            {
                dots -= 204;
                ly++;
                //Compare LY check for interrupt
                compareLY();
                //Check if this is last on screen scanline and set to appropriate mode
                mode = (ly == 144) ? PPU_modes::MODE_1:PPU_modes::MODE_2;
                //Change STAT's mode bits
                setModeSTAT();
                //Call mode function
                if(mode == PPU_modes::MODE_1)
                {
                    mode1();
                } 
                else 
                {
                    //Clear the vector previous objects
                    objects.clear();
                    tileCount = 0;
                    bgFIFO = {};
                    obFIFO = {};

                    mode2();
                }
            }
            break;
        }

        case (PPU_modes::MODE_1):
        {
            if(dots >= 456)
            {
                dots -= 456;
                ly++;
                
                if(ly == 154)
                {
                    //If LY is 154 mean completed all 154 scanlines reset LY to 0
                    if(!startUp)
                    {
                        r->update((uint32_t*)frame);
                        startUp = false;

                        window_trigger = false;
                        window_ly = 0;
                    }
                    ly = 0;
                    //Check for interrupt
                    compareLY();
                    mode = PPU_modes::MODE_2;

                    //Change STATS mode bits
                    setModeSTAT();

                    //Clear the vector previous objects
                    objects.clear();
                    tileCount = 0;
                    bgFIFO = {};
                    obFIFO = {};
                    mode2();
                }
                else
                {
                    compareLY();
                }
            }

            break;
        }

        case (PPU_modes::MODE_2):
        {
            if(dots >= 80)
            {
                dots -= 80;
                mode = PPU_modes::MODE_3;
                setModeSTAT();
                if((lcdc & 0x20) != 0 && wy == ly)
                {
                    window_trigger = true;
                }
                mode3();
            };

            break;
        }

        case (PPU_modes::MODE_3):
        {
            if(dots >= 172)
            {
                dots -= 172;
                mode = PPU_modes::MODE_0;
                setModeSTAT();
                lineToRender();
                mode0();
            }

            break;
        }
        default:
            break;
    }
}

void ppu::lineToRender()
{
    if(ly >= 144) return;
    for(int i = 0; i < 160; i++)
    {
        frameColor(static_cast<GBColors>(buffer[i]), ly, i);
    }
}

uint8_t ppu::readReg(uint16_t addr)
{
    switch (addr)
    {
        case 0xFF40: return lcdc;
        case 0xFF41: return stat | 0x80;
        case 0xFF42: return scy;
        case 0xFF43: return scx;
        case 0xFF44: return ly;
        case 0xFF45: return lyc;
        case 0xFF47: return bgp;
        case 0xFF48: return obp0;
        case 0xFF49: return obp1;
        case 0xFF4A: return wy;
        case 0xFF4B: return wx;
        default: return 0xFF;
    }
}

void ppu::writeReg(uint16_t addr, uint8_t val)
{
    switch (addr)
    {
        case 0xFF40: lcdc = val; break;
        case 0xFF41: stat = (val & 0xF8) | (stat & 0x07); break;
        case 0xFF42: scy = val; break;
        case 0xFF43: scx = val; break;
        case 0xFF44: break;
        case 0xFF45: lyc = val; break;
        case 0xFF47: bgp = val; break;
        case 0xFF48: obp0 = val; break;
        case 0xFF49: obp1 = val; break;
        case 0xFF4A: wy = val; break;
        case 0xFF4B: wx = val; break;
    }
}