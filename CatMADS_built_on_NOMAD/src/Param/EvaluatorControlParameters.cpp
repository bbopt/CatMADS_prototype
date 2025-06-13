
#include "../Param/EvaluatorControlParameters.hpp"
#include "../Type/EvalSortType.hpp"


/*----------------------------------------*/
/*         initializations (private)      */
/*----------------------------------------*/
void NOMAD::EvaluatorControlParameters::init()
{
    _typeName = "EvaluatorControl";

    try
    {
        #include "../Attribute/evaluatorControlAttributesDefinition.hpp"
        registerAttributes(_definition);

        // Note: we cannot call checkAndComply() here, the default values
        // are not valid, for instance DIMENSION, X0, etc.

    }
    catch (NOMAD::Exception& e)
    {
        std::string errorMsg = "Attribute registration failed: ";
        errorMsg += e.what();
        throw NOMAD::Exception(__FILE__,__LINE__, errorMsg);
    }

}

/*----------------------------------------*/
/*            check the parameters        */
/*----------------------------------------*/
void NOMAD::EvaluatorControlParameters::checkAndComply(
                        const std::shared_ptr<NOMAD::EvaluatorControlGlobalParameters>& evaluatorControlGlobalParams,
                        const std::shared_ptr<NOMAD::RunParameters>& runParams)
{
    checkInfo();

    if (!toBeChecked())
    {
        // Early out
        return;
    }

    // When runParameters are provided, update internal parameter SUBPROBLEM_MAX_BB_EVAL.
    if (nullptr != runParams)
    {
        auto psdMadsOpt = runParams->getAttributeValue<bool>("PSD_MADS_OPTIMIZATION");
        auto ssdMadsOpt = runParams->getAttributeValue<bool>("SSD_MADS_OPTIMIZATION");
        if (psdMadsOpt)
        {
            setAttributeValue("SUBPROBLEM_MAX_BB_EVAL", getAttributeValueProtected<size_t>("PSD_MADS_SUBPROBLEM_MAX_BB_EVAL", false));
        }
        else if (ssdMadsOpt)
        {
            setAttributeValue("SUBPROBLEM_MAX_BB_EVAL", getAttributeValueProtected<size_t>("SSD_MADS_SUBPROBLEM_MAX_BB_EVAL", false));
        }
        else
        {
            setAttributeValue("SUBPROBLEM_MAX_BB_EVAL", NOMAD::INF_SIZE_T);
        }
    }

    if (nullptr != evaluatorControlGlobalParams)
    {
        if (evaluatorControlGlobalParams->toBeChecked())
        {
            evaluatorControlGlobalParams->checkAndComply();
        }
        auto maxSurrogateEval = evaluatorControlGlobalParams->getAttributeValue<size_t>("MAX_SURROGATE_EVAL_OPTIMIZATION");
        bool isSurrogateOptimization = getAttributeValueProtected<bool>("EVAL_SURROGATE_OPTIMIZATION", false);
        if (isSurrogateOptimization)
        {
            // If this is a surrogate optimization, it has to have a maximum number of surrogate evaluations or MAX_EVAL < Inf.
            if (NOMAD::INF_SIZE_T == maxSurrogateEval &&
                NOMAD::INF_SIZE_T == evaluatorControlGlobalParams->getAttributeValue<size_t>("MAX_EVAL"))
            {
                throw NOMAD::Exception(__FILE__, __LINE__,
                    "EVAL_SURROGATE_OPTIMIZATION is used. Parameter MAX_SURROGATE_EVAL_OPTIMIZATION should be set.");
            }
            if (evaluatorControlGlobalParams->getAttributeValue<size_t>("MAX_BB_EVAL") < NOMAD::INF_SIZE_T)
            {
                throw NOMAD::Exception(__FILE__, __LINE__,
                    "Parameter MAX_BB_EVAL should not be set when EVAL_SURROGATE_OPTIMIZATION is used. Use MAX_SURROGATE_EVAL_OPTIMIZATION instead.");
            }
            if (NOMAD::EvalSortType::SURROGATE == getAttributeValueProtected<NOMAD::EvalSortType>("EVAL_QUEUE_SORT",false))
            {
                throw NOMAD::InvalidParameter(__FILE__, __LINE__, "Parameter EVAL_QUEUE_SORT cannot be SURROGATE when EVAL_SURROGATE_OPTIMIZATION is set");
            }
        }
        else
        {
            if (maxSurrogateEval < NOMAD::INF_SIZE_T)
            {
                throw NOMAD::InvalidParameter(__FILE__,__LINE__, "Parameter MAX_SURROGATE_EVAL_OPTIMIZATION should be set only when EVAL_SURROGATE_OPTIMIZATION is used.");
            }
        }

    }

    _toBeChecked = false;

}
// End checkAndComply()




