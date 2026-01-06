add_executable(ThreeHump.exe ThreeHump.cpp)

target_link_libraries(ThreeHump.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(ThreeHump.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS ThreeHump.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})