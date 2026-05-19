add_executable(Hal04_GMADS.exe Hal04.cpp)

target_link_libraries(Hal04_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Hal04_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Hal04_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})