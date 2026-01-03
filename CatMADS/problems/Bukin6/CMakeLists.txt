add_executable(Bukin6.exe Bukin6.cpp)

target_include_directories(Bukin6.exe PRIVATE
    ${CMAKE_SOURCE_DIR}/CatMADS
    ${CMAKE_SOURCE_DIR}/examples/MyExtendedPol
)

# Link the shared CatMADS.cpp file
target_sources(Bukin6.exe PRIVATE 
    ${CMAKE_SOURCE_DIR}/CatMADS/CatMADS.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MyExtendedPollMethod2.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MySimpleMads.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MySimplePoll.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/SimpleEvalPoint.cpp
)

# Link necessary libraries (e.g., NOMAD libraries)
if(OpenMP_CXX_FOUND)
    target_link_libraries(Bukin6.exe PUBLIC nomadAlgos nomadUtils nomadEval OpenMP::OpenMP_CXX)
else()
    target_link_libraries(Bukin6.exe PUBLIC nomadAlgos nomadUtils nomadEval)
endif()

# Set installation properties for the executable
set_target_properties(Bukin6.exe PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" SUFFIX "")

# Install the executable to the current directory
install(TARGETS Bukin6.exe
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})