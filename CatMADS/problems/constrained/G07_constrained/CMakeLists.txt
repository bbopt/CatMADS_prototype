add_executable(G07_constrained.exe G07_constrained.cpp)

target_link_libraries(G07_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(G07_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS G07_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})