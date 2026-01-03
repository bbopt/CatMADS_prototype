add_executable(Branin.exe Branin.cpp)

target_link_libraries(Branin.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Branin.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Branin.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})