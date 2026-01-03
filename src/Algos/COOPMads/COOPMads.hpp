#ifndef __NOMAD_4_5_COOPMADS__
#define __NOMAD_4_5_COOPMADS__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Algorithm.hpp"
#include "../../Eval/Evaluator.hpp"

#include "../../nomad_nsbegin.hpp"

// Class for COOP-Mads
class COOPMads: public Algorithm
{
public:
    /// Constructor
    /**
     \param parentStep    The parent of this step -- \b IN.
     \param evaluators     The Evaluators to initialize all main threads -- \b IN.
     \param evalContParams Parameters to initialize all main threads -- \b IN.
     \param stopReasons   The COOP Mads stop reasons -- \b IN/OUT.
     \param runParams     Parameters for algorithm -- \b IN.
     \param refPbParams   Parameters for original optimization problem. PSD-Mads use its own copy -- \b IN.
     */
    explicit COOPMads(const Step* parentStep,
                      const std::vector<EvaluatorPtr>& evaluators,
                      const std::shared_ptr<EvaluatorControlParameters>& evalContParams,
                      std::shared_ptr<AlgoStopReasons<MadsStopType>> stopReasons,
                      const std::shared_ptr<RunParameters>& runParams,
                      const std::shared_ptr<PbParameters>& refPbParams)
    : Algorithm(parentStep, stopReasons, runParams, std::make_shared<PbParameters>(*refPbParams))
    {
        init(evaluators, evalContParams);
    }
    
    virtual ~COOPMads()
    {
    }
    
    virtual void startImp() override {};
    virtual bool runImp() override;
    virtual void endImp() override;

    void readInformationForHotRestart() override {}

private:
    
    /// Helper for constructor
    void init(const std::vector<EvaluatorPtr>& evaluators,
              const std::shared_ptr<EvaluatorControlParameters>& evalContParams);



};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_COOPMADS__
