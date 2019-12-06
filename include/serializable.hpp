#pragma once

namespace Slate::Streams
{
	/*
		Summary:
			Type that allows an object to written and read from files
	*/
	template <typename Type, typename ... Types>
	class Serializable {};
}