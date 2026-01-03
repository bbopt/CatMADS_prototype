add_executable(Zakharov.exe Zakharov.cpp)

target_link_libraries(Zakharov.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Zakharov.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Zakharov.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})