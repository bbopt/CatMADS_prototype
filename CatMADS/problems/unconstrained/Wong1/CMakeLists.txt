add_executable(Wong1.exe Wong1.cpp)

target_link_libraries(Wong1.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Wong1.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Wong1.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})