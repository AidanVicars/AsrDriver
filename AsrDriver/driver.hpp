#pragma once

//Windows headers
#include <Windows.h>
#include <winternl.h>

//STL
#include <iostream>
#include <string>

//Wrapper class to provide an interface prettier than the windows api
class driver
{
private:
	HANDLE h_driver = nullptr;
	std::string driver_path;
public:
	driver(std::string_view driver_name)
	{
		driver_path = "\\\\.\\";
		driver_path += driver_name;
	}

	bool open()
	{
		std::cout << "Opening handle to the device " << driver_path << '\n';
		h_driver = CreateFileA(driver_path.c_str(), FILE_READ_ACCESS | FILE_WRITE_ACCESS, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (h_driver == INVALID_HANDLE_VALUE)
			return false;

		std::cout << "\t" << h_driver << '\n';
		return true;
	}

	void close()
	{
		std::cout << "Closing handle to device " << driver_path << '\n';
		CloseHandle(h_driver);
	}

	bool io_control(std::uint32_t ctl_code, void* input, std::uint32_t input_size, void* output, std::uint32_t output_size)
	{
		DWORD bytes_returned = 0;
		NTSTATUS status;

		status = DeviceIoControl(h_driver, ctl_code, input, input_size, output, output_size, &bytes_returned, nullptr);

		if (!NT_SUCCESS(status))
			return false;

		return true;
	}

};