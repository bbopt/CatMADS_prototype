add_executable(Beale.exe Beale.cpp)

target_link_libraries(Beale.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Beale.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Beale.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})