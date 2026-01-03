add_executable(HS144_constrained.exe HS144_constrained.cpp)

target_link_libraries(HS144_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(HS144_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS HS144_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})