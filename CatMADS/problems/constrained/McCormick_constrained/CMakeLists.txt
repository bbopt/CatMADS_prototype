add_executable(McCormick_constrained.exe McCormick_constrained.cpp)

target_link_libraries(McCormick_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(McCormick_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS McCormick_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})