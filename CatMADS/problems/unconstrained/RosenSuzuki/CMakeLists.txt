add_executable(RosenSuzuki.exe RosenSuzuki.cpp)

target_link_libraries(RosenSuzuki.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(RosenSuzuki.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS RosenSuzuki.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})