add_executable(HS78.exe HS78.cpp)

target_link_libraries(HS78.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(HS78.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS HS78.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})