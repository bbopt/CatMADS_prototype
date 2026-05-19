add_executable(HS78_GMADS.exe HS78.cpp)

target_link_libraries(HS78_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(HS78_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS HS78_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})