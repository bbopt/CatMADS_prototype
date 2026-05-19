add_executable(Roustant_GMADS.exe Roustant.cpp)

target_link_libraries(Roustant_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Roustant_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Roustant_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})