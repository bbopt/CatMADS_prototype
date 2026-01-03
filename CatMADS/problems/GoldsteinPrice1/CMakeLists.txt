add_executable(GoldsteinPrice1.exe GoldsteinPrice1.cpp)

target_link_libraries(GoldsteinPrice1.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(GoldsteinPrice1.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS GoldsteinPrice1.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})