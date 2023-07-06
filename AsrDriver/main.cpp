#include <iostream>
#include "asrdrv.hpp"

int main()
{
	asr_drv asr{};
	asr.open();
	std::cout << std::hex << asr.read_cr(3) << '\n';

	asr.write_cr(3, 0);
	
	while (true)
	{
		std::cout << std::hex << asr.read_cr(3) << '\n';
		Sleep(440);
	}

	asr.close();
	system("pause");
	return 0;
}