add_executable(SpeedReducer_constrained.exe SpeedReducer_constrained.cpp)

target_link_libraries(SpeedReducer_constrained.exe PUBLIC CatMADS_shared)

# Set installation properties for the executable
set_target_properties(SpeedReducer_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS SpeedReducer_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})