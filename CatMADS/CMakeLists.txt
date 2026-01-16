# CatMADS shared library (compiled once)
add_library(CatMADS_shared STATIC)

target_sources(CatMADS_shared PRIVATE
    ${CMAKE_SOURCE_DIR}/CatMADS/CatMADS.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MyExtendedPollMethod2.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MySimpleMads.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/MySimplePoll.cpp
    ${CMAKE_SOURCE_DIR}/CatMADS/MyExtendedPoll/SimpleEvalPoint.cpp
)

target_include_directories(CatMADS_shared PUBLIC
    ${CMAKE_SOURCE_DIR}/CatMADS
    ${CMAKE_SOURCE_DIR}/examples/MyExtendedPoll
)

# Define base path once for everyone linking this library
target_compile_definitions(CatMADS_shared PUBLIC
    CATMADS_BASE_PATH="${CMAKE_SOURCE_DIR}/CatMADS/"
)

# Link NOMAD + OpenMP once
if(OpenMP_CXX_FOUND)
    target_link_libraries(CatMADS_shared PUBLIC nomadAlgos nomadUtils nomadEval OpenMP::OpenMP_CXX)
else()
    target_link_libraries(CatMADS_shared PUBLIC nomadAlgos nomadUtils nomadEval)
endif()


