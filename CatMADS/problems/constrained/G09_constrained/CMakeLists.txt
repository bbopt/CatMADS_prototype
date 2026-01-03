add_executable(G09_constrained.exe G09_constrained.cpp)

target_link_libraries(G09_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(G09_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS G09_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})