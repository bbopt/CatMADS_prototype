# CatMADS shared library (compiled once)
add_library(GMADS_shared STATIC)

target_sources(GMADS_shared PRIVATE
    ${CMAKE_SOURCE_DIR}/GMADS/GMADS.cpp
)

target_include_directories(GMADS_shared PUBLIC
    ${CMAKE_SOURCE_DIR}/GMADS
)

# Define base path once for everyone linking this library
target_compile_definitions(GMADS_shared PUBLIC
    GMADS_BASE_PATH="${CMAKE_SOURCE_DIR}/GMADS/"
)

# Link NOMAD + OpenMP once
if(OpenMP_CXX_FOUND)
    target_link_libraries(GMADS_shared PUBLIC nomadAlgos nomadUtils nomadEval OpenMP::OpenMP_CXX)
else()
    target_link_libraries(GMADS_shared PUBLIC nomadAlgos nomadUtils nomadEval)
endif()


add_subdirectory(problems)