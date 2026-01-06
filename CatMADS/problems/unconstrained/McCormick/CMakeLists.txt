add_executable(McCormick.exe McCormick.cpp)

target_link_libraries(McCormick.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(McCormick.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS McCormick.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})