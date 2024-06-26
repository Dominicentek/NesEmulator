#pragma once
#include <cstdint>
#include <array>
#include "Cpu.h"
#include "Ppu.h"
#include "Cartrige.h"
#include "Gui.h"

class Gui;

class Bus
{
public:
	Bus();
	~Bus();

public: 
	Cpu cpu;
	Ppu ppu;
	std::shared_ptr<Cartrige> cartrige;
	std::array<uint8_t, 2 * 1024> ram;

public: //funkce
	void CpuWrite(uint16_t address, uint8_t data);
	uint8_t CpuRead(uint16_t address, bool readOnly = false);

	uint8_t controller[2];

public: //system
	void insertCartrige(const std::shared_ptr<Cartrige>& cartrige);
	void reset();
	void clock(Gui *gui = nullptr);

private:
	uint32_t clockCount;
	uint32_t nSystemClockCounter = 0;

	uint8_t controller_state[2];

	uint8_t dma_page = 0x00;
	uint8_t dma_address = 0x00;
	uint8_t dma_data = 0x00;

	bool dma_transfer = false;
	bool dma_dummy = true;
};

