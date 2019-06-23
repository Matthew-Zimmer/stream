CMake_Minimum_Required(VERSION 3.13)

Project(Stream)

If(DEFINED TESTING)
	If(${TESTING} STREQUAL true)
		Add_Executable(Stream "Test/Main.cpp")
		Target_Compile_Definitions(Stream PRIVATE TESTING)
		Add_Subdirectory(Test)
	Else()
		Add_Library(Stream STATIC "")
	EndIf()
	Unset(TESTING CACHE)
ELSE()
	Add_Library(Stream STATIC "")
EndIf()

Add_Subdirectory(Source)
Add_Subdirectory(Include)
Add_Subdirectory(Imp)

Target_Compile_Features(Stream 
    PUBLIC cxx_std_17)

Set(CMAKE_CXX_STANDARD "libc++")
#Set(CMAKE_CXX_FLAGS "-Wall" "-Wextra")

Install(TARGETS Stream EXPORT Stream DESTINATION lib/Stream)
Install(EXPORT Stream DESTINATION lib/Stream)
Install(FILES StreamConfig.cmake DESTINATION lib/Stream)

Find_Package(Reflection REQUIRED)
Target_Link_Libraries(Stream PUBLIC Reflection)