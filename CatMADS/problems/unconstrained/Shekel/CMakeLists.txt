add_executable(Shekel.exe Shekel.cpp)

target_link_libraries(Shekel.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Shekel.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Shekel.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})