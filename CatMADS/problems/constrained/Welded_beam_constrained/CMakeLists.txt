add_executable(Welded_beam_constrained.exe Welded_beam_constrained.cpp)

target_link_libraries(Welded_beam_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Welded_beam_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Welded_beam_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})