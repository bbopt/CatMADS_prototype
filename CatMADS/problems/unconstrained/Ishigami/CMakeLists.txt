add_executable(Ishigami.exe Ishigami.cpp)

target_link_libraries(Ishigami.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Ishigami.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Ishigami.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})