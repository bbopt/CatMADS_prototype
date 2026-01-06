add_executable(Wong3_constrained.exe Wong3_constrained.cpp)

target_link_libraries(Wong3_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Wong3_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Wong3_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})