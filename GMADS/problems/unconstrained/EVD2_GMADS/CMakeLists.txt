add_executable(EVD2_GMADS.exe EVD2.cpp)

target_link_libraries(EVD2_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(EVD2_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS EVD2_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})