add_executable(StyblinskiTang_GMADS.exe StyblinskiTang.cpp)

target_link_libraries(StyblinskiTang_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(StyblinskiTang_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS StyblinskiTang_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})