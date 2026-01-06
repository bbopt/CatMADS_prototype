add_executable(Roustant.exe Roustant.cpp)

target_link_libraries(Roustant.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Roustant.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Roustant.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})