
#include "../../Algos/CoordinateSearch/CSInitialization.hpp"
#include "../../Algos/CoordinateSearch/CSMesh.hpp"

void NOMAD::CSInitialization::init()
{
    _initialMesh = std::make_shared<NOMAD::CSMesh>(_pbParams);
}


