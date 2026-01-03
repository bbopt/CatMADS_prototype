add_executable(EDV2_constrained.exe EDV2_constrained.cpp)

target_link_libraries(EDV2_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(EDV2_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS EDV2_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})