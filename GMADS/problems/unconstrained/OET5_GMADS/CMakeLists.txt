add_executable(OET5_GMADS.exe OET5.cpp)

target_link_libraries(OET5_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(OET5_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS OET5_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})