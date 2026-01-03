add_executable(Bukin6_constrained.exe Bukin6_constrained.cpp)

target_link_libraries(Bukin6_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Bukin6_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Bukin6_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})