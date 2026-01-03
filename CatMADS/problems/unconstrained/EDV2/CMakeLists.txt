add_executable(EDV2.exe EDV2.cpp)

target_link_libraries(EDV2.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(EDV2.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS EDV2.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})