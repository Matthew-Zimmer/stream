Target_Include_Directories(Stream PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> 
	$<INSTALL_INTERFACE:include/Stream>)

Set(Imps 
	)

Install(FILES ${Imps} DESTINATION include/Stream)