#include "Streams.hpp"

namespace COMPANY_NAME
{
	inline namespace Streams
	{
		Buffer::Buffer() : buffer{ nullptr }, size{ 0 }
		{}

		Buffer::Buffer(Size_Type size) : size{ size }
		{
			if (size <= 16)
				buffer = small_buffer;
			else
				buffer = new char[size];
		}

		Buffer::~Buffer()
		{
			if (buffer && size > 16)
				delete[] buffer;
		}

		Buffer::Buffer(Buffer&& b)
		{
			if (buffer && size > 16)
				delete[] buffer;
			if (b.size <= 16)
			{
				std::memcpy(small_buffer, b.small_buffer, 16);
				size = b.size;
				b.buffer = nullptr;
				b.size = 0;
				buffer = small_buffer;
			}
			else
			{
				buffer = b.buffer;
				size = b.size;
				b.buffer = nullptr;
				b.size = 0;
			}
		}

		Buffer& Buffer::operator=(Buffer&& b)
		{
			if (buffer && size > 16)
				delete[] buffer;
			if (b.size <= 16)
			{
				std::memcpy(small_buffer, b.small_buffer, 16);
				size = b.size;
				b.buffer = nullptr;
				b.size = 0;
				buffer = small_buffer;
			}
			else
			{
				buffer = b.buffer;
				size = b.size;
				b.buffer = nullptr;
				b.size = 0;
			}
			return *this;
		}

		const Buffer::Size_Type& Buffer::Size() const
		{
			return size;
		}

		Buffer::operator char*()
		{
			return buffer + offset;
		}

		void Buffer::Advance(Size_Type amount)
		{
			offset += amount;
		}

		char& Buffer::operator[](Size_Type index)
		{
			return *(buffer + offset + index);
		}

		Buffer::Size_Type Buffer::Offset() const
		{
			return offset;
		}

		void Buffer::Reset_Offset(Size_Type val)
		{
			offset = val;
		}
	}
}