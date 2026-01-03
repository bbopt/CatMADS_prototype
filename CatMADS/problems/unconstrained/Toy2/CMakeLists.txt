add_executable(Toy2.exe Toy2.cpp)

target_link_libraries(Toy2.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Toy2.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Toy2.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})