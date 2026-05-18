add_executable(Ishigami_GMADS.exe Ishigami.cpp)

target_link_libraries(Ishigami_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Ishigami_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Ishigami_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})