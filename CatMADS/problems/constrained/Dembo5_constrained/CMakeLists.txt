add_executable(Dembo5_constrained.exe Dembo5_constrained.cpp)

target_link_libraries(Dembo5_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Dembo5_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Dembo5_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})