
#include "Algos/Mads/Mads.hpp"
#include "Algos/AlgoStopReasons.hpp"
#include "Type/BBInputType.hpp"

#include "MySimplePoll.hpp"


/// The (M)esh (A)daptive (D)irect (S)earch algorithm. Simple version for extended poll
/**
 */
class MySimpleMads: public NOMAD::Mads
{
private:
    
    MySimplePoll _poll;
    MySuccessType _myPollSuccessType = MySuccessType::UNDEFINED;
    
    NOMAD::BBOutputTypeList _bbot;
    
    const size_t _maxEval;
    
    NOMAD::ArrayOfDouble _minFrameSize;
    
    
public:

    
    /// Constructor #1 for eval_x
    /**
     \param parentStep                  The parent of this step -- \b IN.
     \param stopReasons                The stop reasons for MADS -- \b IN.
     \param runParams                    The run parameters that control MADS -- \b IN.
     \param pbParams                      The problem parameters that control MADS -- \b IN.
     \param bbot                               The bb output type -- \b IN.
     \param eval_x                           The function to compute outputs -- \b IN.
     \param maxEval                         Max evaluation stopping criterion -- \b IN.
     \param refBestFeas                         Reference best eval point for poll opportunistic stop -- \b IN.
     \param refBestInf                         Reference best eval point for poll opportunistic stop -- \b IN.
     \param hMaxMainAlgo                    Current value of hMax in the main algo
     */
    explicit MySimpleMads(const NOMAD::Step* parentStep,
                  std::shared_ptr<NOMAD::AlgoStopReasons<NOMAD::MadsStopType>> stopReasons,
                  const std::shared_ptr<NOMAD::RunParameters>& runParams,
                  const std::shared_ptr<NOMAD::PbParameters>& pbParams,
                  const NOMAD::BBOutputTypeList & bbot,
                  std::shared_ptr<NOMAD::Evaluator>& eval_x,
                  const size_t maxEval,
                  //const NOMAD::EvalPoint & refBestFeas,
                  //const NOMAD::EvalPoint & refBestInf,
                  NOMAD::EvalPointPtr refBestFeas,
                  NOMAD::EvalPointPtr refBestInf,
                  const NOMAD::EvalPoint & firstFrameCenter,
                  const NOMAD::Double & hMaxMainAlgo)
      : Mads(parentStep, stopReasons, runParams, pbParams, false /* false: barrier not initialized from cache */, false /* false: do not use local fixed variables */),
        _poll(this, bbot, eval_x, refBestFeas, refBestInf, firstFrameCenter, hMaxMainAlgo),
        _bbot(bbot),
        _maxEval(maxEval)
    {
        init();
    }
    
    const NOMAD::SimpleEvalPoint & getBestSimpleSolution(bool bestFeas) const ;
    
    NOMAD::EvalPoint getBestSolution (bool bestFeas) const override;
    
    const std::vector<NOMAD::EvalPoint> & getAllEvaluatedTrialPoints() const { return _poll.getAllEvaluatedTrialPoints() ; }
    
    MySuccessType getMySuccessType() const { return _myPollSuccessType;}
    

private:
    ///  Initialization of class, to be used by Constructor.
    void init();

    virtual void startImp() override {};
    
    /// Algorithm execution for single-objective.
    /**
     Overrides the default algorithm's run
     \return \c true if a full success was found, \c false otherwise
     */
    virtual bool runImp() override;
    
    virtual void endImp() override;
    
    void endDisplay() const ;

};

