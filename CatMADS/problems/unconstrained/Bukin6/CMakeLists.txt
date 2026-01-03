add_executable(Bukin6.exe Bukin6.cpp)

target_link_libraries(Bukin6.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Bukin6.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Bukin6.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})