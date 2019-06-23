Target_Include_Directories(Stream PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> 
	$<INSTALL_INTERFACE:include/Stream>)

Set(Headers 
	Serializable.hpp
	Streams.hpp)

Install(FILES ${Headers} DESTINATION include/Stream)