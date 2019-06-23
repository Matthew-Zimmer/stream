#pragma once
#include <Reflection/Reflection.hpp>

namespace COMPANY_NAME
{
	/*
		Summary:
			Type that allows an object to written and read from files
	*/
	template <typename Type, typename ... Types>
	class Serializable
	{
	public:

		/*
			Constraits:
				Stream_Type: Is_Stream<Stream_Type> is true
			Args:
				stream: the stream to be written to
			Summary:
				Writes all variables to the stream
			Complexity:
				O(nk) where n is number of variables and 
				k is the complexity of the writing of that variable to the stream 
		*/
		template<typename Stream_Type>
		void Write(Stream_Type& stream)
		{
			([&]() 
			{
				if constexpr(Is_Variable<Types>)
					stream << Meta::Cast<Types, Type>(*this).Variable();
			}
			(), ...);
		}

		/*
			Constraits:
				Stream_Type: Is_Stream<Stream_Type> is true
			Args:
				stream: the stream to be read from
			Summary:
				Reads all variables from the stream
			Complexity:
				O(nk) where n is number of variables and
				k is the complexity of the reading of that variable from the stream
		*/
		template<typename Stream_Type>
		void Read(Stream_Type& stream)
		{
			([&]()
			{
				if constexpr (Is_Variable<Types>)
					stream >> Meta::Cast<Types, Type>(*this).Variable();
			}
			(), ...);
		}
	public:

		/*
			Summary:
				Marks that this type is serializable
		*/
		using Serializable_Type = void;
	};
}