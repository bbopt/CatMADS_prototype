/**
 \file   DiscoMadsMegaIteration.hpp
 \brief  The DiscoMads algorithm iteration (more specific)
 \author Solene Kojtych
 \see    DiscoMadsMegaIteration.cpp
 */
#ifndef __NOMAD_4_5_DISCOMADSMEGAITERATION__
#define __NOMAD_4_5_DISCOMADSMEGAITERATION__

#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/DiscoMads/DiscoMadsIteration.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for the iterations of DiscoMADS
/**
Manager for Mads iterations.

*/
class DiscoMadsMegaIteration: public MadsMegaIteration
{

private:
    // vector of indices of revealing outputs
    std::vector<int> _idxRevealingOutput;

    // DiscoMads parameters  // TODO: better as const but should be defined in constructor then
    NOMAD::Double  _exclusionRadius;   // used to penalize revealed regions 
    NOMAD::Double  _detectionRadius;   // detectionRadius and limitRate are used to reveal discontinuities
    NOMAD::Double  _limitRate;   

    bool _detectHiddConst;               // if true, discoMads used to reveal hidden constraints instead of discontinuities      
    NOMAD::Double _hiddConstOutputValue;  // only used to reveal hidden constraints regions

    bool _isRevealing ;  // Flag for indicating if MegaIteration is revealing. Reset at start, after DiscoMadsUpdate because it uses the flag.

public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param k               The main iteration counter -- \b IN.
     \param barrier         The barrier for constraints handling -- \b IN.
     \param mesh            Mesh on which other Iteration meshes are based -- \b IN.
     \param success         Success type of the previous MegaIteration. -- \b IN.
     */
    explicit DiscoMadsMegaIteration(const Step* parentStep,
                                  size_t k,
                                  std::shared_ptr<BarrierBase> barrier,
                                  MeshBasePtr mesh,
                                  SuccessType success)
      : MadsMegaIteration(parentStep, k, barrier, mesh, success),
        _isRevealing(false)
    {
        // Replace MadsIteration by DiscoMadsIteration
        _madsIteration = std::make_unique<NOMAD::DiscoMadsIteration>(this, k, mesh);
        init();
    }
    // No Destructor needed - keep defaults.

    // Access to the revealing status of the MegaIteration run.
    bool isRevealing() const { return _isRevealing; }


private:
    /// Implementation of the start tasks for DiscoMads mega iteration.
    void startImp() override ;

    /// Implementation of the run tasks for DiscoMads mega iteration.
    /**
     Manages the generation of points: either all poll and search points are generated all together before starting evaluation using the MegaSearchPoll or they are generated using a MadsIteration with search and poll separately. A run parameter controls the behavior.
     */
    virtual bool runImp() override;

    /// Implementation of the end tasks for DiscoMads mega iteration.
    /**
    Increment number of revealing iterations if required and calls MegaIteration::endImp()
     */
    virtual void endImp() override;

    void init();

    // Test presence of weak discontinuity between x1 and x2
    bool discontinuityTest(const NOMAD::EvalPoint & x1, const NOMAD::EvalPoint & x2);

    // Test if x1 and x2 are at distance less than exclusion radius and that x2 is revealing
    bool proximityTestOnRevealingPoint(const NOMAD::Point & x1, const NOMAD::EvalPoint & x2);



    // Callback attached to evaluator: test if evalQueuePoint is a revealing point (for discontinuities or hidden constraints) and update its revealed constraint
    void callbackCheckIfRevealingAndUpdate(EvalQueuePointPtr & evalQueuePoint);

    // Callback attached to evaluator: put high value of objective function f for failed eval
    /**
    Only used if DiscoMads is used to reveal hidden constraints regions)
    */
    void callbackFailedEval(EvalQueuePointPtr & evalQueuePoint);

    // Callback attached to evaluator: check after each evaluation if there was a revelation and set opportunisticIterStop to True.
    // This will trigger an iter stop.
    void callbackEvalOpportStop(bool &opportunisticIterStop, EvalQueuePointPtr & evalQueuePoint );

    // Callback attached to postProcessing : if there has been a revelation, stop current step and stop megaiteration without doing remaining eval
    void callbackPostProcessing(const NOMAD::Step & step, bool &stop);

    // Only use for debug: save a special text file with cache information at the end of each megaiteration
    void exportCache(const std::string& cacheFile);

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DISCOMADSMEGAITERATION__
