#pragma once

namespace Slate::Stream
{
	/*
		Summary:
			Type that allows an object to written and read from files
	*/
	template <typename Type, typename ... Types>
	class Serializable {};
}