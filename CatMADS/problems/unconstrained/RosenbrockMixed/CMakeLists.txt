add_executable(RosenbrockMixed.exe RosenbrockMixed.cpp)

target_link_libraries(RosenbrockMixed.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(RosenbrockMixed.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS RosenbrockMixed.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})