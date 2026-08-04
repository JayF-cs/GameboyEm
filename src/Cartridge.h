#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <chrono>
#include <cassert>
#include <ctime>

struct RTC
{
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t hours = 0;
    uint16_t days = 0;
    bool halt = false;
    bool dayCarry = false;
};

class MBC{

    protected:
        uint8_t ROMsize;
        uint8_t RAMsize;
        std::vector<uint8_t> rom;
        std::vector<uint8_t> ram;
        bool ramEnabled = false;

        static int getNumBanks(uint8_t ramS)
        {
            switch(ramS)
            {
                case 0x02: return 1;
                case 0x03: return 4;
                case 0x04: return 16;
                case 0x05: return 8;
                default: return 0;
            }
        }

        static size_t getRamSize(int ramS)
        {
            return getNumBanks(ramS) * 0x2000;
        }

    public:
        MBC(std::vector<uint8_t> romVec, uint8_t romS, uint8_t ramS) : rom(std::move(romVec)), ROMsize(romS), RAMsize(ramS) {
            ram.resize(getRamSize(ramS));
        };
        std::vector<uint8_t>& getRam() { return ram; }
        virtual bool hasRTC() const { return false; }
        virtual std::vector<uint8_t> serializeRTC() { return {}; }
        virtual void deserializeRTC(const std::vector<uint8_t>&) {}
        virtual uint8_t readRom(uint16_t addr) = 0 ;
        virtual void writeRom(uint16_t addr, uint8_t val) = 0;
        virtual uint8_t readRam(uint16_t addr) = 0;
        virtual void writeRam(uint16_t addr, uint8_t val) = 0;
        virtual ~MBC() = default;
        uint8_t getMBCType() { return rom[0x147]; }
};

class MBC0: public MBC 
{

    public:
        MBC0(std::vector<uint8_t> romVec, uint8_t romS, uint8_t ramS) : MBC(std::move(romVec), romS, ramS) {};
        uint8_t readRom(uint16_t addr);
        void writeRom(uint16_t addr, uint8_t val);
        
        uint8_t readRam(uint16_t addr);
        void writeRam(uint16_t addr, uint8_t val);
};

class MBC1: public MBC {
    private:
        uint8_t bankMode = 0;
        int bank1low = 1;
        int bank1high = 0;
        int numBanksROM = 0;
        int numBanksRAM = 0;

    public:
        MBC1(std::vector<uint8_t> romVec, uint8_t romS, uint8_t ramS) : MBC(std::move(romVec), romS, ramS), numBanksROM(2 << romS), numBanksRAM(getNumBanks(ramS)) {};
        uint8_t readRom(uint16_t addr);
        void writeRom(uint16_t addr, uint8_t val);
        
        uint8_t readRam(uint16_t addr);
        void writeRam(uint16_t addr, uint8_t val);
};

class MBC3 : public MBC
{
    private:
        RTC rtc;
        std::time_t base_time = 0;
        bool rtcZeroWrite = false;
        uint8_t ramBank = 0;
        uint8_t romBank = 0;
        int numBanksRom = 0;
        int numBanksRam = 0;        

    public:
        MBC3(std::vector<uint8_t> romVec, uint8_t romS, uint8_t ramS) : MBC(std::move(romVec), romS, ramS), numBanksRom(2 << romS), numBanksRam(getNumBanks(ramS)), base_time(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {};
        bool hasRTC() const override { return true; }
        std::vector<uint8_t> serializeRTC() override;
        void deserializeRTC(const std::vector<uint8_t>& data) override;
        uint8_t readRom(uint16_t addr);
        void writeRom(uint16_t addr, uint8_t val);
        uint8_t readRam(uint16_t addr);
        void writeRam(uint16_t addr, uint8_t val);
};

class MBC5 : public MBC
 {
    private:
        uint8_t ramBank = 0;
        uint8_t romBankLow = 0;
        bool romBankHigh = 0;
        int numBanksRom = 0;
        int numBanksRam = 0;        

    public:
        MBC5(std::vector<uint8_t> romVec, uint8_t romS, uint8_t ramS) : MBC(std::move(romVec), romS, ramS), numBanksRom(2 << romS), numBanksRam(getNumBanks(ramS)) {};
        uint8_t readRom(uint16_t addr);
        void writeRom(uint16_t addr, uint8_t val);
        
        uint8_t readRam(uint16_t addr);
        void writeRam(uint16_t addr, uint8_t val);
};

class Cartridge
{
    private:
        std::unique_ptr<MBC> mbc;
    public:
        Cartridge();
        bool hasBattery() const;
        int saveRam(const std::string& path);
        int loadRam(const std::string& path);
        int loadVector(const std::string& path);
        void write_8(uint16_t address, uint8_t val);
        uint8_t read_8(uint16_t address);
        void setMBC(uint8_t mbcType, std::vector<uint8_t> rom);
        uint8_t getMBC();
};
