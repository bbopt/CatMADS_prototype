add_executable(PressureVessel_constrained.exe PressureVessel_constrained.cpp)

target_link_libraries(PressureVessel_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(PressureVessel_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS PressureVessel_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})