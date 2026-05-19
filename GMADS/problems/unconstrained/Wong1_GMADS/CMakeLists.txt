add_executable(Wong1_GMADS.exe Wong1.cpp)

target_link_libraries(Wong1_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Wong1_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Wong1_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})