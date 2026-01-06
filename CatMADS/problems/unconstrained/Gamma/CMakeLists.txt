add_executable(Gamma.exe Gamma.cpp)

target_link_libraries(Gamma.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Gamma.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Gamma.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})