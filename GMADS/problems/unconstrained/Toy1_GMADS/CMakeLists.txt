add_executable(Toy1_GMADS.exe Toy1.cpp)

target_link_libraries(Toy1_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Toy1_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Toy1_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})