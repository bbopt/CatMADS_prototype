add_executable(Beale_GMADS.exe Beale.cpp)

target_link_libraries(Beale_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Beale_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Beale_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})