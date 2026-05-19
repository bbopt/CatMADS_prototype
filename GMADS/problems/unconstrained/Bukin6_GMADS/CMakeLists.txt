add_executable(Bukin6_GMADS.exe Bukin6.cpp)

target_link_libraries(Bukin6_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Bukin6_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Bukin6_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})