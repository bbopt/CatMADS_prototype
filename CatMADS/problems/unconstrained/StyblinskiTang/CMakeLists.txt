add_executable(StyblinskiTang.exe StyblinskiTang.cpp)

target_link_libraries(StyblinskiTang.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(StyblinskiTang.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS StyblinskiTang.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})