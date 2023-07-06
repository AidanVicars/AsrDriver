#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

class data_stream
{
private:
	std::uint32_t cursor{ 0 };
	std::uint32_t size{ 0 };
	std::byte* data = nullptr;
public:
	data_stream(std::uint32_t size)
		: size{ size }
	{
		data = new std::byte[size];
	}

	void free()
	{
		delete[] data;
	}

	void resize(std::uint32_t new_size)
	{
		//Allocate new data
		std::byte* new_data = new std::byte[new_size];
		//Copy old data
		memcpy(new_data, data, size);
		//Delete old data buffer
		delete[] data;
		//Set cursor to start of new data
		cursor = size;
		//Update total size and data ptr
		size = new_size;
		data = new_data;
	}

	void extend(std::uint32_t more_size)
	{
		resize(size + more_size);
	}

	void seek(std::uint32_t position, bool relative = false)
	{
		if (!relative)
			cursor = position;
		else
			cursor += position;
	}

	std::uint32_t tell()
	{
		return cursor;
	}

	void* start()
	{
		return &data[0];
	}

	template<typename data_t>
	void write(data_t data_in, bool maleable = false)
	{
		std::uint32_t data_size = sizeof data_in;
		if ((data_size + cursor) > size)
		{
			if (maleable)
				resize(data_size + size);
			else
				return;
		}

		memcpy(&data[cursor], &data_in, data_size);
		cursor += data_size;
	}

	void write_buffer(void* data_in, std::uint32_t data_size, bool maleable = false)
	{
		if ((data_size + cursor) > size)
		{
			if (maleable)
				resize(data_size + size);
			else
				return;
		}

		memcpy(&data[cursor], data_in, data_size);
		cursor += data_size;
	}

	void write_string(const char* string)
	{
		std::uint32_t string_len = strlen(string);

		strcpy_s((char*)&data[cursor], string_len, string);
	}

	void set(std::uint8_t value, std::uint32_t data_size)
	{
		if ((cursor + data_size) > size)
			return;

		memset(&data[cursor], value, data_size);
		cursor += data_size;
	}

	template<typename data_t>
	void read(data_t* data_out)
	{
		std::uint32_t data_size = sizeof data;
		if ((cursor + size) > size)
			return;

		memcpy(data_out, &data[cursor], data_size);
	}

	void read(void* data_out, std::uint32_t data_size)
	{
		if ((cursor + data_size) > size)
			return;

		memcpy(&data_out, &data[cursor], data_size);
	}

	std::uintptr_t view(std::uintptr_t offset)
	{
		return reinterpret_cast<std::uintptr_t>(&data[offset]);
	}

	std::uintptr_t current()
	{
		return reinterpret_cast<std::uintptr_t>(&data[cursor]);
	}

	std::uint32_t get_size()
	{
		return size;
	}

};
