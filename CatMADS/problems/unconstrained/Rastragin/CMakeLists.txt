add_executable(Rastragin.exe Rastragin.cpp)

target_link_libraries(Rastragin.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Rastragin.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Rastragin.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})