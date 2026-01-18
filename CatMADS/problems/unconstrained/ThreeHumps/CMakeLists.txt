add_executable(ThreeHumps.exe ThreeHumps.cpp)

target_link_libraries(ThreeHumps.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(ThreeHumps.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS ThreeHumps.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})