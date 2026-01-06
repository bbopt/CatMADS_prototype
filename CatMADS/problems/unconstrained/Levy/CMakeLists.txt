add_executable(Levy.exe Levy.cpp)

target_link_libraries(Levy.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Levy.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Levy.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})