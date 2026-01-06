add_executable(Wong3.exe Wong3.cpp)

target_link_libraries(Wong3.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Wong3.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Wong3.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})