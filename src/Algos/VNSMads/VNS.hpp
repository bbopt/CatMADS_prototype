#ifndef __NOMAD_4_5_VNS__
#define __NOMAD_4_5_VNS__


#include "../../Algos/Algorithm.hpp"
#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/Mads.hpp"
#include "../../nomad_nsbegin.hpp"

/// Class implementing VNS Mads algorithm for constrained problems.

class VNS: public Algorithm
{
private:
    
    std::shared_ptr<AlgoStopReasons<MadsStopType>>    _madsStopReasons;
    
    std::shared_ptr<BarrierBase>    _barrier;
    
    std::shared_ptr<RunParameters>      _optRunParams; ///< run parameters for Mads sub optimization
    std::shared_ptr<PbParameters>       _optPbParams; ///< pb parameters for mads sub optimization
    
    EvalPointPtr          _frameCenter; ///< frame center to start mads sub optimization
    
    Point                           _refFrameCenter; ///<  The reference frame center to test if frame center is modified
    
    /**
     The neighborhood parameter is used to multiply the shake direction. Explore further away when neighborhood parameter is increased.
     */
    double _neighParameter;
    
public:
    /// Constructor
    /**
     \param parentStep          The parent of this Step -- \b IN.
     \param stopReasons         The stop reasons for NM -- \b IN.
     \param runParams           The run parameters that control NM -- \b IN.
     \param pbParams            The problem parameters that control NM -- \b IN.
     */
    explicit VNS(const Step* parentStep,
                std::shared_ptr<AlgoStopReasons<VNSStopType>> stopReasons,
                const std::shared_ptr<RunParameters>& runParams,
                const std::shared_ptr<PbParameters>& pbParams )
      : Algorithm(parentStep, stopReasons, runParams, pbParams),
        _barrier (nullptr),
        _frameCenter (nullptr),
        _neighParameter (0.0)
    {
        init();
    }

    /// Destructor
    virtual ~VNS() {}

    virtual void readInformationForHotRestart() override {}

    std::shared_ptr<BarrierBase> getBarrier() {return _barrier; }
    
    /// The frame center is used as initial point for the sub-optimization
    void setFrameCenter(const EvalPointPtr& frameCenter);
    
private:
    
    /// Helper for constructor
    void init();

    /// Implementation for run tasks.
    /**
     - Algorithm execution.
     - Shake current incumbents.
     - Perform mads.
     - Update the success type
     - Perform Termination tasks (start, run, end)
     - Update the SearchMethod success type with best success found.
     \return \c true
     */
    virtual bool runImp() override;

    /// Implementation for start tasks.
    /**
     - Set the stop reason to STARTED
     - Reset sub-algorithm counter
     - Perform Initialization tasks (start, run, end)
     */
    virtual void startImp() override;

    virtual void endImp() override;
    
    // Helpers
    void setupRunParameters();
    void setupPbParameters(const NOMAD::Point & center, const NOMAD::ArrayOfDouble & currentMadsFrameSize);

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_VNS__
