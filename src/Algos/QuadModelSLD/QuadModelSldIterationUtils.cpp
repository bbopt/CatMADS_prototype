
#include "../../Algos/QuadModelSLD/QuadModelSldIterationUtils.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSldIteration.hpp"
#include "../../Output/OutputQueue.hpp"

void NOMAD::QuadModelSldIterationUtils::init()
{
    auto iter = dynamic_cast<const QuadModelSldIteration*>(_iterAncestor);
    if ( nullptr != iter )
    {
        _model = iter->getModel();
//        _trainingSet = iter->getTrainingSet();
    }
}


void NOMAD::QuadModelSldIterationUtils::displayModelInfo() const
{
    if ( nullptr == _model /* || nullptr == _trainingSet */ )
        throw NOMAD::Exception(__FILE__, __LINE__, "The iteration utils must have a model to work with");

    OUTPUT_DEBUG_START
    NOMAD::OutputInfo dbgInfo("Quad Model SLD iteration utils", "", NOMAD::OutputLevel::LEVEL_DEBUG );

    NOMAD::OutputQueue::Add(std::move(dbgInfo));
    NOMAD::OutputQueue::Flush();
    OUTPUT_DEBUG_END
}
