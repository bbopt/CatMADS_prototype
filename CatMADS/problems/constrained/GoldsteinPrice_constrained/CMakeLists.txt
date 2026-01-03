add_executable(GoldsteinPrice_constrained.exe GoldsteinPrice_constrained.cpp)

target_link_libraries(GoldsteinPrice_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(GoldsteinPrice_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS GoldsteinPrice_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})