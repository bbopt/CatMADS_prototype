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


# Add problems directories
add_subdirectory(problems/AckleyHard)
add_subdirectory(problems/Beale)
add_subdirectory(problems/Beale_constrained)
add_subdirectory(problems/Branin)
add_subdirectory(problems/Branin_constrained)
add_subdirectory(problems/Bukin6)
add_subdirectory(problems/Bukin6_constrained)
add_subdirectory(problems/Dembo5_constrained)
add_subdirectory(problems/EDV2)
add_subdirectory(problems/EDV2_constrained)
add_subdirectory(problems/G09_constrained)
add_subdirectory(problems/GoldsteinPrice_constrained)
add_subdirectory(problems/GoldsteinPrice1)
add_subdirectory(problems/GoldsteinPrice2)
add_subdirectory(problems/Himmelblau_constrained)
add_subdirectory(problems/HS78)
add_subdirectory(problems/HS144_constrained)
add_subdirectory(problems/Pentagon_constrained)
add_subdirectory(problems/PressureVessel_constrained)
add_subdirectory(problems/Rastragin)
add_subdirectory(problems/ReinforcedConcreteBeam_constrained)
add_subdirectory(problems/Rosenbrock_constrained)
add_subdirectory(problems/RosenbrockMixed)
add_subdirectory(problems/RosenSuzuki)
add_subdirectory(problems/StyblinskiTang)
add_subdirectory(problems/StyblinskiTang_constrained)
add_subdirectory(problems/Toy_constrained)
add_subdirectory(problems/Toy1)
add_subdirectory(problems/Toy2)
add_subdirectory(problems/Wong1)
add_subdirectory(problems/Wong2_constrained)
add_subdirectory(problems/Zakharov)
