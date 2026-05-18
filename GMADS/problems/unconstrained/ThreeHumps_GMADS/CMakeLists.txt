add_executable(ThreeHumps_GMADS.exe ThreeHumps.cpp)

target_link_libraries(ThreeHumps_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(ThreeHumps_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS ThreeHumps_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})