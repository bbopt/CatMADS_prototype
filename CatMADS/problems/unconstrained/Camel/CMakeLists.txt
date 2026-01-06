add_executable(Camel.exe Camel.cpp)

target_link_libraries(Camel.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Camel.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Camel.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})