#ifndef __NOMAD_4_5_TEMPLATEALGOINITIALIZATION__
#define __NOMAD_4_5_TEMPLATEALGOINITIALIZATION__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Initialization.hpp"
#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgoInitialization.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Output/OutputQueue.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for template algo initialization
/**
 * Evaluate initial trial points (x0).
 * If x0 is not provided simply pass.
 */
class TemplateAlgoInitialization: public Initialization, public IterationUtils
{
private:

    std::shared_ptr<AlgoStopReasons<RandomAlgoStopType>> _templateAlgoStopReason;

public:
    /// Constructor
    /*
     \param parentStep      The parent of this step -- \b IN.
     */
    explicit TemplateAlgoInitialization(const Step* parentStep)
      : Initialization(parentStep),
        IterationUtils(parentStep)
    {
        init();
    }

    /// Destructor
    virtual ~TemplateAlgoInitialization() {}


private:
    /// Helper for constructor
    void init();

    /// Implementation of start task
    /**
     If needed, randomly generate trial points and put them in cache.
     For a standalone optimization (RANDOM_ALGO_OPTIMIZATION true), initial trial point can be provided in x0 or we simply pass.
     */
    virtual void startImp() override ;

    /// Implementation of run task
    /**
     For a standalone template algorithm, evaluate the trial points generated during start.
     Otherwise, there are no trial points available and a failed stop reason is set.
     */
    virtual bool runImp() override ;

    // Update _evalPointList member with evaluated trial points for future use
    void endImp() override;

    /// Generate initial trial point randomly
    void generateTrialPointsImp() override;


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_TEMPLATEALGOINITIALIZATION__
