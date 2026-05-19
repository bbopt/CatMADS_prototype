add_executable(GoldsteinPrice2_GMADS.exe GoldsteinPrice2.cpp)

target_link_libraries(GoldsteinPrice2_GMADS.exe PUBLIC GMADS_shared)

# Set installation properties for the executable
set_target_properties(GoldsteinPrice2_GMADS.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS GoldsteinPrice2_GMADS.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})