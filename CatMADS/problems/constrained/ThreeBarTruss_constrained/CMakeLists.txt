add_executable(ThreeBarTruss_constrained.exe ThreeBarTruss_constrained.cpp)

target_link_libraries(ThreeBarTruss_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(ThreeBarTruss_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS ThreeBarTruss_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})