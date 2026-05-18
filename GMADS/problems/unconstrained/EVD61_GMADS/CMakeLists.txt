add_executable(EVD61_GMADS.exe EVD61.cpp)

target_link_libraries(EVD61_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(EVD61_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS EVD61_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})