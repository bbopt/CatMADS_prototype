add_executable(Toy_constrained.exe Toy_constrained.cpp)

target_include_directories(Toy_constrained.exe PRIVATE
    ${CMAKE_SOURCE_DIR}/CatMADS
    ${CMAKE_SOURCE_DIR}/examples/MyExtendedPoll
)

# Link the shared CatMADS.cpp file
target_sources(Toy_constrained.exe PRIVATE 
    ${CMAKE_SOURCE_DIR}/CatMADS/CatMADS.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MyExtendedPollMethod2.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MySimpleMads.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MySimplePoll.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/SimpleEvalPoint.cpp
)

# Link necessary libraries (e.g., NOMAD libraries)
if(OpenMP_CXX_FOUND)
    target_link_libraries(Toy_constrained.exe PUBLIC nomadAlgos nomadUtils nomadEval OpenMP::OpenMP_CXX)
else()
    target_link_libraries(Toy_constrained.exe PUBLIC nomadAlgos nomadUtils nomadEval)
endif()

# Set installation properties for the executable
set_target_properties(Toy_constrained.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Toy_constrained.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})