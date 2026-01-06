add_executable(Hartmann.exe Hartmann.cpp)

target_link_libraries(Hartmann.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Hartmann.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Hartmann.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})