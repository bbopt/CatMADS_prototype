add_executable(Ishigami_constrained.exe Ishigami_constrained.cpp)

target_link_libraries(Ishigami_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Ishigami_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Ishigami_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})