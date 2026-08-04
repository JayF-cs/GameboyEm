#include "Cartridge.h"

Cartridge::Cartridge() {};

int Cartridge::loadVector(const std::string& path)
{
    //Makes file pointer based on the path parameter that is passed in
    std::vector<uint8_t> rom;
    std::ifstream file(path,std::ios::binary | std::ios::ate);

    //Makes sure file pointer is valid
    if(!file)
    {
        std::cerr << "Could not open file!\n";
        return -1;
    }

    std::streamsize size = file.tellg();
    if(size <= 0)
    {
        std::cerr << "Error getting file size\n";
        return -1;
    }
    file.seekg(0, std::ios::beg);
    rom.clear();
    rom.resize(static_cast<size_t>(size));
    //Open ROM file and dump contents into ROM vector
    if(!file.read(reinterpret_cast<char*>(rom.data()), size))
    {
        std::cerr << "Error reading ROM data" << std::endl;
        return -1;
    }
    
    file.close();
    //Get the MBC type for memory stuff
    //Also defined in Cartridge.h
    uint8_t mbcType = rom[0x0147];
    //printf("MBC type: %02X", mbcType);

    setMBC(mbcType, rom);

    return 0;

}

uint8_t Cartridge::read_8(uint16_t address)
{
    if(address >= 0 && address <= 0x7FFF)
    {
        return mbc->readRom(address);
    }
    else if(address >= 0xA000 && address <= 0xBFFF)
    {
        return mbc->readRam(address);
    }

    return 0xFF;
}

void Cartridge::write_8(uint16_t address, uint8_t val)
{
    if(address >= 0 && address <= 0x7FFF)
    {
        mbc->writeRom(address, val);
    }
    else if(address >= 0xA000 && address <= 0xBFFF)
    {
        mbc->writeRam(address, val);
    }
}

void Cartridge::setMBC(uint8_t mbcType, std::vector<uint8_t> rom)
{
    uint8_t romSize = rom[0x0148];
    uint8_t ramSize = rom[0x0149];

    switch(mbcType)
    {
        case 0x00: // ROM ONLY
            mbc = std::make_unique<MBC0>(std::move(rom), romSize, ramSize);
            break;
        case 0x01: // MBC1
        case 0x02: // MBC1+RAM
        case 0x03: // MBC1+RAM+BATTERY
            mbc = std::make_unique<MBC1>(std::move(rom), romSize, ramSize);
            break;
        case 0x0F: // MBC3+TIMER+BATTERY
        case 0x10: // MBC3+TIMER+RAM+BATTERY
        case 0x11: // MBC3
        case 0x12: // MBC3+RAM
        case 0x13: // MBC3+RAM+BATTERY
            mbc = std::make_unique<MBC3>(std::move(rom), romSize, ramSize);
            break;
        case 0x19: // MBC5
        case 0x1A: // MBC5+RAM
        case 0x1B: // MBC5+RAM+BATTERY
        case 0x1C: // MBC5+RUMBLE
        case 0x1D: // MBC5+RUMBLE+RAM
        case 0x1E: // MBC5+RUMBLE+RAM+BATTERY
            mbc = std::make_unique<MBC5>(std::move(rom), romSize, ramSize);
            break;
        default:
            std::cerr << "Unsupported MBC type: " << +mbcType << "\n";
            std::exit(EXIT_FAILURE);
    }
}

bool Cartridge::hasBattery() const
{
    switch(mbc->getMBCType())
    {
        case 0x03: case 0x06: case 0x09: case 0x0D:
        case 0x0F: case 0x10: case 0x13:
        case 0x1B: case 0x1E:
        case 0x22: case 0xFF:
            return true;
        default:
            return false;
    }
}

int Cartridge::saveRam(const std::string& path)
{
    if(!hasBattery()) return 0;

    auto& ram = mbc->getRam();
    std::ofstream file(path, std::ios::binary);
    if(!file)
    {
        std::cerr << "Could not open save file for writing\n";
        return -1;
    }

    if(!ram.empty())
        file.write(reinterpret_cast<const char*>(ram.data()), ram.size());

    if(mbc->hasRTC())
    {
        auto rtcData = mbc->serializeRTC();
        file.write(reinterpret_cast<const char*>(rtcData.data()), rtcData.size());
    }

    return file.good() ? 0 : -1;
}

int Cartridge::loadRam(const std::string& path)
{
    if(!hasBattery()) return 0;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if(!file) return -1;

    std::streamsize size = file.tellg();
    if(size <= 0) return -1;
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(static_cast<size_t>(size));
    if(!file.read(reinterpret_cast<char*>(data.data()), size)) return -1;

    auto& ram = mbc->getRam();
    size_t ramBytes = std::min(ram.size(), data.size());
    std::copy(data.begin(), data.begin() + ramBytes, ram.begin());

    if(mbc->hasRTC() && data.size() > ramBytes)
    {
        std::vector<uint8_t> rtcData(data.begin() + ramBytes, data.end());
        mbc->deserializeRTC(rtcData);
    }

    return 0;
}

//////////////////////////////////////////////////////////////
//                      MBC 0                               //
//////////////////////////////////////////////////////////////

uint8_t MBC0::readRom(uint16_t addr)
{
    if(addr >= 0 && addr <= 0x7FFF)
    {
        return rom[addr];
    }

    return 0xFF;
}

void MBC0::writeRom(uint16_t addr,  uint8_t val)
{
    return;
}

uint8_t MBC0::readRam(uint16_t addr)
{
    return 0xFF;
}

void MBC0::writeRam(uint16_t addr,  uint8_t val)
{
    return;
}

//////////////////////////////////////////////////////////////
//                      MBC 1                               //
//////////////////////////////////////////////////////////////

uint8_t MBC1::readRom(uint16_t addr)
{

    if(addr >= 0 && addr <= 0x3FFF)
    {
        return rom[addr];
    }
    else if(addr > 0x3FFF && addr <= 0x7FFF)
    {
        int bank1 = (bankMode == 0) ? ((bank1high << 5) | bank1low) : bank1low;
        int mask = numBanksROM - 1;
        
        return rom[((bank1 & mask) * 0x4000) + (addr - 0x4000)];
    }

    return 0xFF;
}

void MBC1::writeRom(uint16_t addr, uint8_t val)
{
    if(addr >= 0x0000 && addr <= 0x1FFF)
    {
        if((val & 0x0F) == 0x0A)
        {
            ramEnabled = true;
        }
        else
        {
            ramEnabled = false;
        }
    }
    else if(addr >= 0x2000 && addr <= 0x3FFF)
    {
        val = val & 0x1F;
        if(val != 0) bank1low = val;
        if(val == 0) bank1low = 1;
    }
    else if(addr >= 0x4000 && addr <= 0x5FFF)
    {
        bank1high = val & 0x3;
    }
    else if(addr >= 0x6000 && addr <= 0x7FFF)
    {
        bankMode = val & 0x1;
    }
}

uint8_t MBC1::readRam(uint16_t addr)
{
    if(!ramEnabled || numBanksRAM == 0) return 0xFF;

    int ramBank = (bankMode == 0) ? bank1high : 0;
    int mask = numBanksRAM - 1;
    return ram[((ramBank & mask) * 0x2000) + (addr - 0xA000)];
}

void MBC1::writeRam(uint16_t addr, uint8_t val)
{
    if(!ramEnabled || numBanksRAM == 0) return;

    int ramBank = (bankMode == 0) ? bank1high : 0;
    int mask = numBanksRAM - 1;
    ram[((ramBank & mask) * 0x2000) + (addr - 0xA000)] = val;
}

//////////////////////////////////////////////////////////////
//                      MBC 3                               //
//////////////////////////////////////////////////////////////

uint8_t MBC3::readRom(uint16_t addr)
{
    if(addr >= 0 && addr <= 0x3FFF)
    {
        return rom[addr];
    }
    else if(addr >= 0x4000 && addr <= 0x7FFF)
    {
        uint8_t bank = romBank & (numBanksRom - 1);
        return rom[((bank * 0x4000) + (addr - 0x4000))];
    }

    return 0xFF;
}

void MBC3::writeRom(uint16_t addr, uint8_t val)
{
    if(addr >= 0x0000 && addr <= 0x1FFF)
    {
        if((val & 0x0F) == 0x0A)
        {
            ramEnabled = true;
        }
        else
        {
            ramEnabled = false;
        }
    }
    else if(addr >= 0x2000 && addr <= 0x3FFF)
    {
        romBank = val;
        if(romBank == 0) romBank = 1;
    }
    else if(addr >= 0x4000 && addr <= 0x5FFF)
    {
        if(numBanksRam == 4)
        {
            if(val <= 0x3)
            {
                ramBank = val;
            }
            else if(val >= 0x8 && val <= 0xC)
            {
                ramBank = val;
            }
        }
        else if(numBanksRam == 8)
        {
            if(val <= 0x7)
            {
                ramBank = val;
            }
            else if(val >= 0x8 && val <= 0xC)
            {
                ramBank = val;
            }
        }
    }
    else if(addr >= 0x6000 && addr <= 0x7FFF)
    {
        if(val == 0) {rtcZeroWrite = true; return;}
        if(!rtcZeroWrite) return;
        if(val != 1) {rtcZeroWrite = false; return;}

        rtcZeroWrite = false;

        std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::time_t elapsed = current_time - base_time;
        base_time += elapsed;
        
        int carry = (rtc.seconds + elapsed) / 60;
        rtc.seconds = (rtc.seconds + elapsed) % 60;
        carry = (rtc.minutes + carry) / 60;
        rtc.minutes = (rtc.minutes + carry) % 60;
        carry = (rtc.hours + carry) / 24;
        rtc.hours = (rtc.hours + carry) % 24;
        rtc.days += carry;

        if(rtc.days > 0x1FF)
        {
            rtc.dayCarry = true;
            rtc.days &= 0x1FF;
        } 
         
    }
}

uint8_t MBC3::readRam(uint16_t addr)
{
    if(!ramEnabled || numBanksRam == 0) return 0xFF;

    if(ramBank <= 0x7)
    {
        int mask = numBanksRam - 1;
        return ram[((ramBank & mask) * 0x2000) + (addr - 0xA000)];
    }
    else if(ramBank <= 0xC)
    {
        switch(ramBank)
        {
            case(0x8): return rtc.seconds;
            case(0x9): return rtc.minutes;
            case(0xA): return rtc.hours;
            case(0xB): return static_cast<uint8_t>(rtc.days & 0x00FF);
            case(0xC): return static_cast<uint8_t>((rtc.dayCarry << 7) | (rtc.halt << 6) | ((rtc.days >> 8) & 0x01));
        }
    }

    return 0xFF;
}

void MBC3::writeRam(uint16_t addr, uint8_t val)
{
    if(!ramEnabled || numBanksRam == 0) return;

    if(ramBank <= 0x7)
    {
        int mask = numBanksRam - 1;
        ram[((ramBank & mask) * 0x2000) + (addr - 0xA000)] = val;
    }
    else if(ramBank <= 0xC)
    {
        switch(ramBank)
        {
            case 0x8: rtc.seconds = val; break;
            case 0x9: rtc.minutes = val; break;
            case 0xA: rtc.hours = val; break;
            case 0xB:
                rtc.days = (rtc.days & 0xFF00) | val;
                break;
            case 0xC:
                rtc.days = (rtc.days & 0x00FF) | ((val & 0x01) << 8);
                rtc.halt = (val & 0x40) != 0;
                rtc.dayCarry = (val & 0x80) != 0;
                break;
        }
    }
}

std::vector<uint8_t> MBC3::serializeRTC()
{
    std::vector<uint8_t> data;
    data.push_back(rtc.seconds);
    data.push_back(rtc.minutes);
    data.push_back(rtc.hours);
    data.push_back(static_cast<uint8_t>(rtc.days & 0xFF));
    data.push_back(static_cast<uint8_t>((rtc.days >> 8) & 0xFF));
    data.push_back(rtc.halt ? 1 : 0);
    data.push_back(rtc.dayCarry ? 1 : 0);

    int64_t bt = static_cast<int64_t>(base_time);
    for(int i = 0; i < 8; i++)
        data.push_back(static_cast<uint8_t>((bt >> (i * 8)) & 0xFF));

    return data;
}

void MBC3::deserializeRTC(const std::vector<uint8_t>& data)
{
    if(data.size() < 15) return;

    rtc.seconds  = data[0];
    rtc.minutes  = data[1];
    rtc.hours    = data[2];
    rtc.days     = static_cast<uint16_t>(data[3] | (data[4] << 8));
    rtc.halt     = data[5] != 0;
    rtc.dayCarry = data[6] != 0;

    int64_t bt = 0;
    for(int i = 0; i < 8; i++)
        bt |= static_cast<int64_t>(data[7 + i]) << (i * 8);
    base_time = static_cast<std::time_t>(bt);
}

//////////////////////////////////////////////////////////////
//                      MBC 5                               //
//////////////////////////////////////////////////////////////

uint8_t MBC5::readRom(uint16_t addr)
{
    if(addr >= 0 && addr <= 0x3FFF)
    {
        return rom[addr];
    }
    else if(addr >= 0x4000 && addr <= 0x7FFF)
    {
        uint16_t fullBank = ((romBankHigh << 8) | romBankLow);
        uint16_t bank = (fullBank) & (numBanksRom - 1);
        size_t flatAddr = (static_cast<size_t>(bank) * 0x4000) + (addr - 0x4000);
        return rom[flatAddr];
    }

    return 0xFF;
}

void MBC5::writeRom(uint16_t addr, uint8_t val)
{
    if(addr >= 0x0000 && addr <= 0x1FFF)
    {
        if((val & 0x0F) == 0x0A)
        {
            ramEnabled = true;
        }
        else
        {
            ramEnabled = false;
        }
    }
    else if(addr >= 0x2000 && addr <= 0x2FFF)
    {
        romBankLow = val;
    }
    else if(addr >= 0x3000 && addr <= 0x3FFF)
    {
        romBankHigh = val & 0x1;
    }
    else if(addr >= 0x4000 && addr <= 0x5FFF)
    {
        ramBank = val & 0xF;
    }
}

uint8_t MBC5::readRam(uint16_t addr)
{
    if(!ramEnabled || numBanksRam == 0) return 0xFF;

    int mask = numBanksRam - 1;
    return ram[((ramBank & mask) * 0x2000) + (addr - 0xA000)];
}

void MBC5::writeRam(uint16_t addr, uint8_t val)
{
    if(!ramEnabled || numBanksRam == 0) return;

    int mask = numBanksRam - 1;
    ram[((ramBank & mask) * 0x2000) + (addr - 0xA000)] = val;
}

uint8_t Cartridge::getMBC()
{
    return mbc->getMBCType();
}