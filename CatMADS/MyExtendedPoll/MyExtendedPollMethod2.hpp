
#include "Algos/AlgoStopReasons.hpp"
#include "Algos/Mads/ExtendedPollMethod.hpp"
#include "Eval/Evaluator.hpp"


/// Class to perform a custom extended poll method.
/**
TODO explain custom method
 */
class MyExtendedPollMethod2 : public NOMAD::ExtendedPollMethod
{
    shared_ptr<NOMAD::Evaluator> _my_evaluator ;
    
public:
    /// Constructor
    /**
     /param parentStep      The parent of this extended poll method step -- \b IN.
     */
    explicit MyExtendedPollMethod2(shared_ptr<NOMAD::Mads> & mads, shared_ptr<NOMAD::Evaluator> & evaluator )
      : ExtendedPollMethod( mads ),
        _my_evaluator(evaluator)
    {
        init();
    }

    /**
     Execute (start, run, end) of the NM algorithm. Returns a \c true flag if the algorithm found better point.
     */
    virtual bool runImp() override ;

private:

    /// Helper for constructor.
    /**
     Test if the NM search is enabled or not. Set the maximum number of trial points.
     */
    void init();
    

    //bool runOptim(const NOMAD::EvalPoint & pp, const NOMAD::EvalPoint & refBestFeas, const NOMAD::EvalPoint & refBestInf);
    bool runOptim(const NOMAD::EvalPoint & pp, NOMAD::EvalPointPtr refBestFeas, NOMAD::EvalPointPtr refBestInf);    

};

