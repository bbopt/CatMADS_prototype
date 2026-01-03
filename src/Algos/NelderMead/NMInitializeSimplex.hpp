#ifndef __NOMAD_4_5_NMINITIALIZESIMPLEX__
#define __NOMAD_4_5_NMINITIALIZESIMPLEX__

#include "../../Algos/NelderMead/NMIterationUtils.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for simplex initialization of NM algorithm.
/**
 The simplex is obtained by calling run (done in NMIteration).
 */
class NMInitializeSimplex: public Step, public NMIterationUtils
{
public:
    /// Constructor
    /**
     \param parentStep The parent of this NM step
     */
    explicit NMInitializeSimplex(const Step* parentStep)
      : Step(parentStep) ,
        NMIterationUtils(parentStep)
    {
        init();
    }
    virtual ~NMInitializeSimplex() {}

    /// No new points are generated
    void generateTrialPointsImp() override {}

private:
    /// Helper for constructor
    void init();

    /// No start task is required
    virtual void    startImp() override {}

    /// Implementation of the run task for simplex initialization.
    /**
     Calls createSimplex if required.
     */
    virtual bool    runImp() override ;

    /// No end task is required
    virtual void    endImp() override {}

    /// Helper for run
    /**
     From the Cache or from the Barrier, a set of points within a radius of the current frame center is considered before creating the initial simplex. The radius depends on a two given parameters and the frame size.
     The initial simplex is obtained by adding points to obtain dim=n+1 and simplex being affinely independent. We use the rank of DZ=[(y1-y0 (y2-y0) ...(yk-y0)] to decide if a point yk increases the rank or not.
     The characteristics of the initial simplex (volumes and diameter) are updated.
     */
    bool createSimplex();


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_NMINITIALIZESIMPLEX__
