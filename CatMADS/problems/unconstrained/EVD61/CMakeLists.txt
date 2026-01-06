add_executable(EVD61.exe EVD61.cpp)

target_link_libraries(EVD61.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(EVD61.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS EVD61.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})