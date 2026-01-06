add_executable(MAD_constrained.exe MAD_constrained.cpp)

target_link_libraries(MAD_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(MAD_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS MAD_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})