target_include_directories(stream INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> 
	$<INSTALL_INTERFACE:include/stream>)

Set(headers 
	binary_format.hpp
	local_stream.hpp
	serializable.hpp
	stream.hpp)

Install(FILES ${headers} DESTINATION include/stream)