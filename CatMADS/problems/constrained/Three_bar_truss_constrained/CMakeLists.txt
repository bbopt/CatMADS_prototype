add_executable(Three_bar_truss_constrained.exe Three_bar_truss_constrained.cpp)

target_link_libraries(Three_bar_truss_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(Three_bar_truss_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Three_bar_truss_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})