Get_Filename_Component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
Include(${SELF_DIR}/Stream.cmake)

Find_Package(Reflection REQUIRED)