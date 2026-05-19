add_executable(Branin_GMADS.exe Branin.cpp)

target_link_libraries(Branin_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(Branin_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Branin_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})