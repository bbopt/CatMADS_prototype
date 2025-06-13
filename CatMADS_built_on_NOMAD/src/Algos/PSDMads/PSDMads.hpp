#ifndef __NOMAD_4_5_PSDMADS__
#define __NOMAD_4_5_PSDMADS__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Algorithm.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Eval/Evaluator.hpp"
#include "../../Math/RandomPickup.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for PSD-Mads
class PSDMads: public Algorithm
{
private:
    RandomPickup                _randomPickup;          ///< To manage selection of variables for subproblems. Reset only by pollster.
    std::shared_ptr<MeshBase>   _psdMainMesh;           ///< Base Mesh to create subproblem Mads. Updated only by pollster.
    std::shared_ptr<BarrierBase>    _barrier;               ///< Barrier with the latest successful values. Updated by all Mads.

    std::shared_ptr<MadsMegaIteration> _masterMegaIteration;
    
    std::atomic<bool>           _lastMadsSuccessful;    ///< Used as indication to enlarge or refine the mesh. Updated by all Mads.

public:
    /// Constructor
    /**
     \param parentStep    The parent of this step -- \b IN.
     \param evaluators     The Evaluators to initialize all main threads -- \b IN.
     \param evalContParams Parameters to initialize all main threads -- \b IN.
     \param stopReasons   The PSD Mads stop reasons -- \b IN/OUT.
     \param runParams     Parameters for algorithm -- \b IN.
     \param refPbParams   Parameters for original optimization problem. PSD-Mads use its own copy -- \b IN.
     */
    explicit PSDMads(const Step* parentStep,
                     const std::vector<EvaluatorPtr>& evaluators,
                     const std::shared_ptr<EvaluatorControlParameters>& evalContParams,
                     std::shared_ptr<AlgoStopReasons<MadsStopType>> stopReasons,
                     const std::shared_ptr<RunParameters>& runParams,
                     const std::shared_ptr<PbParameters>& refPbParams)
      : Algorithm(parentStep, stopReasons, runParams, std::make_shared<PbParameters>(*refPbParams)),
        _randomPickup(_pbParams->getAttributeValue<size_t>("DIMENSION")),
        _psdMainMesh(nullptr),
        _barrier(nullptr),
        _lastMadsSuccessful(false)
    {
        init(evaluators, evalContParams);
    }

    virtual ~PSDMads()
    {
    }

    virtual void startImp() override;
    virtual bool runImp() override;
    virtual void endImp() override;

    void readInformationForHotRestart() override {}

    void setupSubproblemParams(std::shared_ptr<PbParameters> &subProblemPbParams,
                               std::shared_ptr<RunParameters> &subProblemRunParams,
                               const Point& bestPoint,
                               const bool isPollster);

private:
    /// Helper for constructor
    void init(const std::vector<EvaluatorPtr>& evaluators,
              const std::shared_ptr<EvaluatorControlParameters>& evalContParams);

    /// Wait for _barrier to be initialized by pollster before running a worker.
    void waitForBarrier() const;

    /// Assess if it is time to update _mainMesh
    bool doUpdateMesh() const;

    /// Setup fixedVariable to define subproblem
    void generateSubproblem(Point &fixedVariable);
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_PSDMADS__
