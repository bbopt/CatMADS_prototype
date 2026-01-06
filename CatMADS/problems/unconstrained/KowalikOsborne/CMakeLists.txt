add_executable(KowalikOsborne.exe KowalikOsborne.cpp)

target_link_libraries(KowalikOsborne.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(KowalikOsborne.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS KowalikOsborne.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})