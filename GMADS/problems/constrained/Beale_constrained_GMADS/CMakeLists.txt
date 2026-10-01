add_executable(Beale_constrained_GMADS.exe Beale_constrained.cpp)

target_link_libraries(Beale_constrained_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Beale_constrained_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Beale_constrained_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})