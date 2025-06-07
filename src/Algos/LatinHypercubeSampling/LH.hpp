#ifndef __NOMAD_4_5_LH__
#define __NOMAD_4_5_LH__

#include "../../Algos/Algorithm.hpp"
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/IterationUtils.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for Latin Hypercube algorithm sampling.
/**
 Generate the trial points using LHS and evaluate them.
 \todo Complete documentation
 */
class LH: public Algorithm, public IterationUtils
{

public:
    /// Constructor
    explicit LH(const Step* parentStep,
                std::shared_ptr<AlgoStopReasons<LHStopType>> stopReasons,
                const std::shared_ptr<RunParameters>& runParams,
                const std::shared_ptr<PbParameters>& pbParams)
    : Algorithm(parentStep, stopReasons, runParams, pbParams),
      IterationUtils(this)
    {
        init();
    }

    /// Destructor
    virtual ~LH() {}


    virtual void readInformationForHotRestart() override {}
    
    virtual NOMAD::ArrayOfPoint suggest() override; ///< For suggest and observe PyNomad interface

private:
    /// Helper for constructor
    void init();

    /// Implementation for start task.
    /**
     Calls default algorithm start and LH::generateTrialPoints
     */
    virtual void    startImp() override;

    /// Implementation for run tasks.
    /**
     Evaluation of the trial points

     \return \c true a better point has been obtained, \c false otherwise.
     */
    virtual bool    runImp()   override;

    /**
     \copydoc IterationUtils::generateTrialPoints \n
     \note For LH, the generation of points uses LHS for sampling with the problem's bounds and X0.
     */
    void generateTrialPointsImp() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_LH__
