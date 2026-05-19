add_executable(GoldsteinPrice1_GMADS.exe GoldsteinPrice1.cpp)

target_link_libraries(GoldsteinPrice1_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(GoldsteinPrice1_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS GoldsteinPrice1_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})