add_executable(Levy_GMADS.exe Levy.cpp)

target_link_libraries(Levy_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Levy_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Levy_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})