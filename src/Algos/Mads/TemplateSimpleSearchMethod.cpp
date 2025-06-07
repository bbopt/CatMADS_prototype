/**
 \file   TemplateSimpleSearchMethod.cpp
 \brief  A template for simple random search without iteration (implementation)
 \author Christophe Tribes
 \date   2022-05-25
 */

#include "../../Algos/Mads/TemplateSimpleSearchMethod.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoSinglePass.hpp"
#include "../../Output/OutputQueue.hpp"

/*-------------------------------------------------------------*/
/*    Template for a simple (no iterations) search method      */
/*-------------------------------------------------------------*/
/*  Can be called (RANDOM_SIMPLE_SEARCH yes)                   */
/*  Generate random points around best incumbent.              */
/*  TEMPLATE use for a new search method: copy and rename the  */
/*  file and the class name. Adapt the code to your needs.     */
/*-------------------------------------------------------------*/

void NOMAD::TemplateSimpleSearchMethod::init()
{
    // TEMPLATE use for a new search method: define a specific step type in ../Type/StepType.hpp.
    setStepType(NOMAD::StepType::SEARCH_METHOD_SIMPLE_RANDOM);

    bool enabled = false;
    // For some testifying, it is possible that _runParams is null
    if (nullptr != _runParams)
    {
        // TEMPLATE use for a new search method: a new parameter must be defined to enable or not the search method (see ../Attributes/runAttributesDefinition.txt)
        enabled = _runParams->getAttributeValue<bool>("RANDOM_SIMPLE_SEARCH");
    }

    setEnabled(enabled);
}


void NOMAD::TemplateSimpleSearchMethod::generateTrialPointsFinal()
{
    // The trial points of one iteration of this template algorithm (random) are generated (not evaluated).

    // Note: Use first point of barrier as center.
    // See issue #392
    NOMAD::TemplateAlgoSinglePass randomAlgo(this,
                                             getMegaIterationBarrier()->getFirstPoint());
    randomAlgo.start();
    randomAlgo.end();

    // Pass the generated trial pts to this
    const auto& trialPtsNM = randomAlgo.getTrialPoints();
    for (const auto& point : trialPtsNM)
    {
        insertTrialPoint(point);
    }
    
}
