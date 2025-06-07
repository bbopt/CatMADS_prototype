
#ifndef __NOMAD_4_5_MADSLOGBARRIER__
#define __NOMAD_4_5_MADSLOGBARRIER__

#include "../../Algos/Mads/Mads.hpp"

#include "../../nomad_nsbegin.hpp"


/// The (M)esh (A)daptive (D)irect (S)earch algorithm with a Mixed Penalty Logarithmic Barrier approach.
/**
 */
class MadsLogBarrier: public Mads
{
private:
    // G^log and G^ext
    std::list<size_t> _Glog, _Gext, _GSwitchedLogToExt; // indices of Glog and Gext constraints
    
    // Beta: 1+1E-9   --> TODO make it a parameter
    const double _beta = 1.000000001;

    // Zeta: 0.01 --> TODO make it a parameter
    const double _zeta = 0.01;

    // Nu: 2 --> TODO make it a parameter
    const double _nu = 2.0;
    
    // cRhoLog: 1E10 (rhoLog update cst)  --> TODO make it a parameter
    const double _cRhoLog = 1E10;
    
    // cRhoExt: 1E2 (rhoExt update cst) --> TODO make it a parameter
    const double _cRhoExt = 1E+2;
    
    // Constraint penalization weights.
    NOMAD::Double _rhoLog=0.1;
    NOMAD::Double _rhoExt; // Initialized when knowing f(x0): __rhoExt = 1/max(|f(x0)|,10).
    
    bool _exteriorPenaltyOnly = false;
    
    bool _logVariant = false;
    
    bool _logToExtSwitch = false ;
    
    NOMAD::Double _logFeasThres = 0;
    
public:
    /// Constructor
    /**
     \param parentStep          The parent of this step -- \b IN.
     \param stopReasons         The stop reasons for MADS -- \b IN.
     \param runParams           The run parameters that control MADS -- \b IN.
     \param pbParams            The problem parameters that control MADS -- \b IN.
     */
    explicit MadsLogBarrier(const Step* parentStep,
                  std::shared_ptr<AlgoStopReasons<MadsStopType>> stopReasons,
                  const std::shared_ptr<RunParameters>& runParams,
                  const std::shared_ptr<PbParameters>& pbParams )
      : Mads(parentStep, stopReasons, runParams, pbParams, false /* barrier not initialized from cache, use X0s*/ )
    {
        init();
    }

private:
    ///  Initialization of class, to be used by Constructor.
    void init();

    virtual void startImp() override;

    
//    /// Algorithm execution for single-objective.
//    /**
//     Overrides the default algorithm's run
//     \return \c true if a full success was found, \c false otherwise
//     */
//    virtual bool runImp() override;

//    /// Helper for start()
//    void readInformationForHotRestart() override;
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_MADSLOGBARRIER__
