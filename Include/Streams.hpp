#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <Reflection/Reflection.hpp>
#include <cstring>

namespace COMPANY_NAME
{
	namespace Imp::Streams
	{
		class Is_Stream
		{
		public:
			template <typename Type, typename Obj>
			static auto Test(Type&& stream, Obj&& obj) -> decltype(stream.Write(obj), stream.Read(obj));
		};

		class Is_Serializable
		{
		public:
			template <typename Type>
			static auto Test(Type&& obj) -> typename Type::Serializable_Type;
		};
	}

	inline namespace Streams
	{
		/*
			Summary:
				Checks if a type is a stream for the type Obj_Type
		*/
		template <typename Type, typename Obj_Type>
		constexpr bool Is_Stream = Satisfies<Imp::Streams::Is_Stream, Type, Obj_Type>;

		/*
			Summary:
				Checks if a type is serializable
		*/
		template <typename Type>
		constexpr bool Is_Serializable = Satisfies<Imp::Streams::Is_Serializable, Type>;

		/*
			Notes:
				Makes dynamic allocations
				Has small buffer optimization
			Summary:
				Represents the buffer to serialize data into
		*/
		class Buffer
		{
		public:
			using Size_Type = size_t;
		private:
			char* buffer;
			Size_Type size;
			char small_buffer[16];
			Size_Type offset;
		public:
			/*
				Summary:
					Creates an empty buffer
				Complexity:
					O(1)
			*/
			Buffer();

			/*
				Notes:
					If size is less than or equal to 16 it does not make a dynamic allocation
				Args:
					size: size of the buffer to create in bytes
				Summary:
					Creates a buffer of size, size
				Complexity:
					O(1)
			*/
			Buffer(Size_Type size);

			/*
				Summary:
					Deletes the buffer
				Complexity:
					O(1)
			*/
			~Buffer();

			/*
				Notes:
					Deleted
			*/
			Buffer(const Buffer&) = delete;

			/*
				Notes:
					Deleted
			*/
			Buffer& operator=(const Buffer&) = delete;

			/*
				Args:
					b: the buffer to steal from
				Summary:
					Steals the data from b, b is left with a nullptr
				Complexity:
					O(1)
			*/
			Buffer(Buffer&& b);

			/*
				Args:
					b: the buffer to steal from
				Return:
					The object being assigned to
				Summary:
					Steals the data from b, b is left with a nullptr
				Complexity:
					O(1)
			*/
			Buffer& operator=(Buffer&& b);

			/*
				Return:
					The size of the buffer
				Summary:
					Gives the current size of the buffer
				Complexity:
					O(1)
			*/
			const Size_Type& Size() const;

			/*
				Return:
					The start of the buffer plus its current offset
				Summary:
					Converts the buffer to a char pointer
				Complexity:
					O(1)
			*/
			operator char*();

			/*
				Args:
					amount: the amount of bytes to change the offset by
				Summary:
					Adds amount, amount of bytes to the offset
				Complexity:
					O(1)
			*/
			void Advance(Size_Type amount);

			/*
				Notes:
					No bounds checking
				Args:
					index: the position of the byte to get
				Return:
					Returns the index th plus current offset byte in the buffer
				Summary:
					Gets the index th plus offset byte out of the buffer
				Complexity:
					O(1)
			*/
			char& operator[](Size_Type index);

			/*
				Args:
					val: the value to set the offset to
				Summary:
					Sets the offset to val
				Complexity:
					O(1)
			*/
			void Reset_Offset(Size_Type val = 0);
			
			/*
				Return:
					Returns the current offset
				Summary:
					Gets the current offset of the buffer
				Complexity:
					O(1)
			*/
			Size_Type Offset() const;
		};

		/*
			Summary:
				Adds the binary format functions to a stream type
		*/
		template <typename Type>
		class Binaryable
		{
			using Size_Type = size_t;

			//GENERAL

			template <typename Object_Type>
			class Size
			{
			public:
				Size_Type operator()(const Object_Type& obj)
				{
					return sizeof(Object_Type);
				}
			};

			template <typename Object_Type>
			class To_Format
			{
			public:
				Buffer operator()(const Object_Type& obj)
				{
					Buffer buffer{ Size<Object_Type>{}(obj) };
					std::memcpy(buffer, reinterpret_cast<const char*>(&obj), buffer.Size());
					return buffer;
				}

				Size_Type In_Place(const Object_Type& obj, Buffer& buffer)
				{
					Size_Type size = Size<Object_Type>{}(obj);
					std::memcpy(buffer, reinterpret_cast<const char*>(&obj), size);
					return size;
				}
			};

			template <typename Object_Type>
			class From_Format
			{
			public:
				void operator()(Object_Type& obj, Buffer& buffer)
				{
					std::memcpy(reinterpret_cast<char*>(&obj), buffer, buffer.Size());
				}

				Size_Type In_Place(Object_Type& obj, Buffer& buffer)
				{
					Size_Type size = Size<Object_Type>{}(obj);
					std::memcpy(reinterpret_cast<char*>(&obj), buffer, size);
					return size;
				}
			};

			//STRING

			template <>
			class Size<std::string>
			{
			public:
				Size_Type operator()(const std::string& obj)
				{
					return obj.size() + 1 + Size<Size_Type>{}(obj.size());
				}
			};

			template <>
			class To_Format<std::string>
			{
			public:
				Buffer operator()(const std::string& obj)
				{
					Buffer buffer{ Size<std::string>{}(obj) };
					Size_Type size = obj.size() + 1;
					buffer.Advance(To_Format<Size_Type>{}.In_Place(size, buffer));
					std::memcpy(buffer, reinterpret_cast<const char*>(obj.data()), size);
					buffer.Reset_Offset();
					return buffer;
				}

				Size_Type In_Place(const std::string& obj, Buffer& buffer)
				{
					Size_Type size = obj.size() + 1, org_off = buffer.Offset();
					buffer.Advance(To_Format<Size_Type>{}.In_Place(size, buffer));
					std::memcpy(buffer, reinterpret_cast<const char*>(obj.data()), size);
					size += buffer.Offset() - org_off;
					buffer.Reset_Offset(org_off);
					return size;
				}
			};

			template <>
			class From_Format<std::string>
			{
			public:
				void operator()(std::string& obj, Buffer& buffer)
				{
					Size_Type size = 0;
					buffer.Advance(From_Format<Size_Type>{}.In_Place(size, buffer));
					obj.clear();
					obj.reserve(size);
					char* c_str = new char[size];
					std::memcpy(c_str, buffer, size);
					obj = c_str;
					delete[] c_str;
					buffer.Reset_Offset();
				}

				Size_Type In_Place(std::string& obj, Buffer& buffer)
				{
					Size_Type size = 0, org_off = buffer.Offset();
					buffer.Advance(From_Format<Size_Type>{}.In_Place(size, buffer));
					obj.clear();
					obj.reserve(size);
					char* c_str = new char[size];
					std::memcpy(c_str, buffer, size);
					obj = c_str;
					delete[] c_str;
					size += buffer.Offset() - org_off;
					buffer.Reset_Offset(org_off);
					return size;
				}
			};

			//VECTOR

			template <typename _Type>
			class Size<std::vector<_Type>>
			{
			public:
				Size_Type operator()(const std::vector<_Type>& obj)
				{
					Size_Type sum = Size<Size_Type>{}(obj.size());
					for (auto& x : obj)
						sum += Size<_Type>{}(x);
					return sum;
				}
			};

			template <typename _Type>
			class To_Format<std::vector<_Type>>
			{
			public:
				Buffer operator()(const std::vector<_Type>& obj)
				{
					Buffer buffer{ Size<std::vector<_Type>>{}(obj) };
					buffer.Advance(To_Format<Size_Type>{}.In_Place(obj.size(), buffer));
					for (auto& x : obj)
						buffer.Advance(To_Format<_Type>{}.In_Place(x, buffer));
					buffer.Reset_Offset();
					return buffer;
				}

				Size_Type In_Place(const std::vector<_Type>& obj, Buffer& buffer)
				{
					Size_Type size = Size<std::vector<_Type>>{}(obj), org_off = buffer.Offset();
					buffer.Advance(To_Format<Size_Type>{}.In_Place(obj.size(), buffer));
					for (auto& x : obj)
						buffer.Advance(To_Format<_Type>{}.In_Place(x, buffer));
					buffer.Reset_Offset(org_off);
					return size;
				}
			};

			template <typename _Type>
			class From_Format<std::vector<_Type>>
			{
			public:
				void operator()(std::vector<_Type>& obj, Buffer& buffer)
				{
					Size_Type size = 0;
					buffer.Advance(From_Format<Size_Type>{}.In_Place(size, buffer));
					obj.clear();
					obj.resize(size);
					for (size_t i = 0; i < size; i++)
						buffer.Advance(From_Format<_Type>{}.In_Place(obj[i], buffer));
					buffer.Reset_Offset();
				}

				Size_Type In_Place(std::vector<_Type>& obj, Buffer& buffer)
				{
					Size_Type size = 0, org_off = buffer.Offset();
					buffer.Advance(From_Format<Size_Type>{}.In_Place(size, buffer));
					obj.clear();
					obj.resize(size);
					for (size_t i = 0; i < size; i++)
						buffer.Advance(From_Format<_Type>{}.In_Place(obj[i], buffer));
					size = buffer.Offset() - org_off;
					buffer.Reset_Offset(org_off);
					return size;
				}
			};

			//TUPLE

			template <typename ... Types>
			class Size<std::tuple<Types...>>
			{
				using Object_Type = std::tuple<Types...>;
			public:
				Size_Type operator()(const Object_Type& obj)
				{
					Size_Type sum = 0;
					([&]()
					{
						sum += Size<Types>{}(std::get<Types>(obj));
					}
					(), ...);
					return sum;
				}
			};

			template <typename ... Types>
			class To_Format<std::tuple<Types...>>
			{
				using Object_Type = std::tuple<Types...>;
			public:
				Buffer operator()(const Object_Type& obj)
				{
					Buffer buffer{ Size<Object_Type>{}(obj) };
					(buffer.Advance(To_Format<Types>{}.In_Place(std::get<Types>(obj), buffer)), ...);
					buffer.Reset_Offset();
					return buffer;
				}

				Size_Type In_Place(const Object_Type& obj, Buffer& buffer)
				{
					Size_Type size = Size<Object_Type>{}(obj), org_off = buffer.Offset();
					(buffer.Advance(To_Format<Types>{}.In_Place(std::get<Types>(obj), buffer)), ...);
					buffer.Reset_Offset(org_off);
					return size;
				}
			};

			template <typename ... Types>
			class From_Format<std::tuple<Types...>>
			{
				using Object_Type = std::tuple<Types...>;
			public:
				void operator()(Object_Type& obj, Buffer& buffer)
				{
					(buffer.Advance(From_Format<Types>{}.In_Place(std::get<Types>(obj), buffer)), ...);
					buffer.Reset_Offset();
				}

				Size_Type In_Place(Object_Type& obj, Buffer& buffer)
				{
					Size_Type org_off = buffer.Offset();
					(buffer.Advance(From_Format<Types>{}.In_Place(std::get<Types>(obj), buffer)), ...);
					Size_Type size = buffer.Offset() - org_off;
					buffer.Reset_Offset(org_off);
					return size;
				}
			};

			//UNORDERED_MAP

			template <typename Key_Type, typename Value_Type>
			class Size<std::unordered_map<Key_Type, Value_Type>>
			{
				using Object_Type = std::unordered_map<Key_Type, Value_Type>;
			public:
				Size_Type operator()(const Object_Type& obj)
				{
					Size_Type sum = Size<Size_Type>{}(obj.size());
					for (auto&[k, v] : obj)
					{
						sum += Size<Key_Type>{}(k);
						sum += Size<Value_Type>{}(v);
					}
					return sum;
				}
			};

			template <typename Key_Type, typename Value_Type>
			class To_Format<std::unordered_map<Key_Type, Value_Type>>
			{
				using Object_Type = std::unordered_map<Key_Type, Value_Type>;
			public:
				Buffer operator()(const Object_Type& obj)
				{
					Buffer buffer{ Size<Object_Type>{}(obj) };
					buffer.Advance(To_Format<Size_Type>{}.In_Place(obj.size(), buffer));
					for (auto&[k, v] : obj)
					{
						buffer.Advance(To_Format<Key_Type>{}.In_Place(k, buffer));
						buffer.Advance(To_Format<Value_Type>{}.In_Place(v, buffer));
					}
					buffer.Reset_Offset();
					return buffer;
				}

				Size_Type In_Place(const Object_Type& obj, Buffer& buffer)
				{
					Size_Type size = Size<Object_Type>{}(obj), org_off = buffer.Offset();
					buffer.Advance(To_Format<Size_Type>{}.In_Place(obj.size(), buffer));
					for (auto&[k, v] : obj)
					{
						buffer.Advance(To_Format<Key_Type>{}.In_Place(k, buffer));
						buffer.Advance(To_Format<Value_Type>{}.In_Place(v, buffer));
					}
					buffer.Reset_Offset(org_off);
					return size;
				}
			};

			template <typename Key_Type, typename Value_Type>
			class From_Format<std::unordered_map<Key_Type, Value_Type>>
			{
				using Object_Type = std::unordered_map<Key_Type, Value_Type>;
			public:
				void operator()(Object_Type& obj, Buffer& buffer)
				{
					Size_Type size = 0;
					buffer.Advance(From_Format<Size_Type>{}.In_Place(size, buffer));
					obj.clear();
					obj.reserve(size);
					Key_Type k;
					Value_Type v;
					for (size_t i = 0; i < size; i++)
					{
						buffer.Advance(From_Format<Key_Type>{}.In_Place(k, buffer));
						buffer.Advance(From_Format<Value_Type>{}.In_Place(v, buffer));
						obj[k] = v;
					}
					buffer.Reset_Offset();
				}

				Size_Type In_Place(Object_Type& obj, Buffer& buffer)
				{
					Size_Type size = 0, org_off = buffer.Offset();
					buffer.Advance(From_Format<Size_Type>{}.In_Place(size, buffer));
					obj.clear();
					obj.reserve(size);
					Key_Type k{};
					Value_Type v{};
					for (size_t i = 0; i < size; i++)
					{
						buffer.Advance(From_Format<Key_Type>{}.In_Place(k, buffer));
						buffer.Advance(From_Format<Value_Type>{}.In_Place(v, buffer));
						obj[k] = v;
					}
					size = buffer.Offset() - org_off;
					buffer.Reset_Offset(org_off);
					return size;
				}
			};
		public:
			/*
				Constraits:
					Object_Type: either Is_Standard_Type<Object_Type> is true 
						or Is_Fundemental_Type<Object_Type> is true
				Args:
					object: the object to calculate the size of
				Return:
					Returns the number of bytes that object takes up
				Summary:
					Calculates the number of bytes of object
				Complexity:
					O(1) if and only if Is_Fundemental_Type<Object_Type> is true
					O(nk) if and only if Is_Standard_Type<Object_Type> is true 
						where n is the number of elements of the container, and 
						k is the complexity of each element
			*/
			template <typename Object_Type>
			constexpr Size_Type Size_Of(const Object_Type& object)
			{
				return Size<std::decay_t<Object_Type>>{}(object);
			}

			/*
				Constraits:
					Object_Type: either Is_Standard_Type<Object_Type> is true
						or Is_Fundemental_Type<Object_Type> is true
				Args:
					object: the object to convert to the binary format
				Return:
					The buffer containing the bytes of the object in the binary format
				Summary:
					Creates a buffer with the bytes representing the binary format of the object
				Complexity:
					O(1) if and only if Is_Fundemental_Type<Object_Type> is true
					O(nk) if and only if Is_Standard_Type<Object_Type> is true
						where n is the number of elements of the container, and
						k is the complexity of each element
			*/
			template <typename Object_Type>
			Buffer To(const Object_Type& object)
			{
				return To_Format<std::decay_t<Object_Type>>{}(object);
			}

			/*
				Constraits:
					Object_Type: either Is_Standard_Type<Object_Type> is true
						or Is_Fundemental_Type<Object_Type> is true
				Args:
					object: the object to fill from the binary format
					buffer: the buffer containing the bytes of the object in the binary format
				Summary:
					Fills the object from a buffer with the bytes representing the binary format of the object
				Complexity:
					O(1) if and only if Is_Fundemental_Type<Object_Type> is true
					O(nk) if and only if Is_Standard_Type<Object_Type> is true
						where n is the number of elements of the container, and
						k is the complexity of each element
			*/
			template <typename Object_Type>
			void From(Object_Type& object, Buffer& buffer)
			{
				From_Format<std::decay_t<Object_Type>>{}(object, buffer);
			}
		};

		template <typename Type>
		class Configurable
		{
		public:
			template <typename Object_Type>
			constexpr unsigned int Size(Object_Type&& t) 
			{
				return 0;
			}

			template <typename Object_Type>
			Buffer To_Format(Object_Type&& object) 
			{
				return Buffer{};
			}

			template <typename Object_Type>
			void From_Format(Object_Type& object, Buffer& buffer) {}
		};

		//How

		/*
			Summary:
				The different modes to open in
		*/
		enum class Open_Mode : char
		{
			Write = 0,
			Read = 1,
		};
		
		/*
			Summary:
				Adds the local read and write functions to a stream type
		*/
		template <typename Type>
		class Localable
		{
			std::fstream stream;
		public:
			/*
				Summary:
					Closes the stream if open
				Complexity:
					O(1)
			*/
			~Localable()
			{
				Close();
			}

			/*
				Constraits:
					SType: either Is_Standard_Type<SType> is true
						or Is_Fundemental_Type<SType> is true
				Args:
					t: the object to be written to the local stream
				Summary:
					Writes the object to the local stream
				Complexity:
					O(n) where n is the number of bytes of the size of t
			*/
			template <typename SType>
			void Write(SType&& t)
			{
				if (stream.is_open())
				{
					Buffer bytes = Meta::Cast<Type>(*this).To(t);
					stream.write(reinterpret_cast<const char*>(&(bytes.Size())), sizeof(Buffer::Size_Type));
					stream.write(bytes, bytes.Size());
				}
			}

			/*
				Constraits:
					SType: either Is_Standard_Type<SType> is true
						or Is_Fundemental_Type<SType> is true
				Args:
					t: the object to be read from the local stream
				Summary:
					Reads the object from the local stream
				Complexity:
					O(n) where n is the number of bytes of the size of t
			*/
			template <typename SType>
			void Read(SType& t)
			{
				Buffer::Size_Type size;
				stream.read(reinterpret_cast<char*>(&size), sizeof(Buffer::Size_Type));
				Buffer bytes{ size };
				stream.read(bytes, size);
				Meta::Cast<Type>(*this).From(t, bytes);
			}

			/*
				Args:
					file_name: the absolute path of the file to be opened
					mode: the mode to open the file in
				Summary:
					Opens the file with name, file_name in the mode of mode
				Complexity:
					O(1)
			*/
			void Open(const std::string& file_name, Open_Mode mode)
			{
				Close();
				switch (mode)
				{
				case COMPANY_NAME::Open_Mode::Write:
					stream.open(file_name.c_str(), stream.out | stream.trunc);
					return;
				case COMPANY_NAME::Open_Mode::Read:
					stream.open(file_name.c_str(), stream.in);
					return;
				}
			}

			/*
				Summary:
					Closes the opened file
				Complexity:
					O(1)
			*/
			void Close()
			{
				stream.close();
			}

			/*
				Args:
					pos: the position to go to in the file
				Summary:
					Moves the stream to position, pos relative 
					to the beginning og the stream
				Complexity:
					O(1)
			*/
			void Seek(int pos)
			{
				stream.seekp(stream.beg, pos);
			}

			/*
				Constraits:
					Object_Type: either Is_Serializable<Object_Type> is true
						or Is_Standard_Type<Object_Type> is true
						or Is_Fundemental_Type<Object_Type> is true
				Args:
					object: the object to be written to the local stream
				Summary:
					Writes the object to the local stream
				Complexity:
					O(n) if and only if Is_Standard_Type<Object_Type> 
						or Is_Fundemental_Type<Object_Type> is true
						where n is the number of bytes of the size of object
					O(n*k) if and only if Is_Serializable<Object_Type> is true 
						where n is the number of variables and 
						k is the size of each variable in bytes
			*/
			template <typename Object_Type>
			Type& operator<<(Object_Type&& object)
			{
				if constexpr (Is_Serializable<std::decay_t<Object_Type>>)
					object.Write(*this);
				else
					Meta::Cast<Type>(*this).Write(object);
				return Meta::Cast<Type>(*this);
			}

			/*
				Constraits:
					Object_Type: either Is_Serializable<Object_Type> is true
						or Is_Standard_Type<Object_Type> is true
						or Is_Fundemental_Type<Object_Type> is true
				Args:
					object: the object to be read from the local stream
				Summary:
					Reads the object from the local stream
				Complexity:
					O(n) if and only if Is_Standard_Type<Object_Type> 
						or Is_Fundemental_Type<Object_Type> is true
							Is_where n is the number of bytes of the size of object
					O(n*k) if and only if Is_Serializable<Object_Type> is true 
						where n is the number of variables and 
						k is the size of each variable in bytes
			*/
			template <typename Object_Type>
			Type& operator>>(Object_Type&& object)
			{
				if constexpr (Is_Serializable<std::decay_t<Object_Type>>)
					object.Read(*this);
				else
					Meta::Cast<Type>(*this).Read(object);
				return Meta::Cast<Type>(*this);
			}
		};

		template <typename Type>
		class Networkable
		{
		public:
			~Networkable()
			{
				Close();
			}
			template <typename Object_Type>
			void Write(Object_Type&&)
			{

			}
			template <typename Object_Type>
			void Read(Object_Type&&)
			{

			}
			void Connect() {}
			void Close() {}
		};

		//General Stream class
		template <template <typename> typename ... FTypes>
		class Stream : public Is<Stream<FTypes...>, Features<FTypes...>>
		{
		public:
			Stream(const std::string& file_name, Open_Mode mode)
			{
				this->Open(file_name, mode);
			}
			Stream() = default;
		};
	}
}