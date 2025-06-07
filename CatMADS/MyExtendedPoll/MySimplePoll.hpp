

#include "Algos/Mads/GMesh.hpp"
#include "Algos/Mads/PollMethodBase.hpp"
//#include "Algos/SimpleMads/SimpleProgressiveBarrier.hpp"
#include "SimpleProgressiveBarrier.hpp"

#include "../ext/sgtelib/src/Surrogate.hpp"


enum class MySuccessType
{
    UNDEFINED,          ///< Default type set at start
    NO_TRIALS,          ///< No trial points produced
    UNSUCCESSFUL,       ///< Trial point is not a success
    PARTIAL_SUCCESS,    ///< Partial success (improving). Found an infeasible
    ///< solution with a better h. f is worse.
    FULL_SUCCESS,       ///< Full success (dominating). NOTE: for the extended poll, it is relative to the subproblem.
    MEGA_SUCCESS        ///< Special succes for the extended poll. A new incumbent is found for the main problem.
};

/// Class for MySimplePoll.
/**
 Generate the trial points and llaunch evaluation.
 */
class MySimplePoll: public NOMAD::Iteration
{
private:
    
    std::vector<NOMAD::SimpleEvalPoint> _trialPoints; ///< The points generated during the start(). Used for run() and postProcessing().
    std::vector<NOMAD::EvalPoint> _allEvaluatedTrialPoints; ///< All evaluated trial points
    
    //NOMAD::EvalPoint _refBest; ///< The reference eval point for opportunistic stop
    //NOMAD::EvalPoint _refBestFeas; ///< The reference eval point for opportunistic stop
    //NOMAD::EvalPoint _refBestInf; ///< The reference eval point for opportunistic stop
    NOMAD::EvalPointPtr _refBestFeas;
    NOMAD::EvalPointPtr _refBestInf;

    NOMAD::EvalPoint _firstFrameCenter;

    std::shared_ptr<NOMAD::Evaluator> _evaluator;
    
    NOMAD::BBOutputTypeList _bbot;
    
    std::unique_ptr<NOMAD::SimpleProgressiveBarrier> _barrier;
    std::shared_ptr<NOMAD::GMesh> _mesh;
    
    MySuccessType _myPollSuccessType = MySuccessType::UNDEFINED;
    
    std::vector<NOMAD::SimpleEvalPoint> _frameCenters;  ///< The frame centers (primary and secondary) of the poll methods.
    NOMAD::SimpleEvalPoint _extendedPollFrameCenter;

    NOMAD::Double _hMaxMainAlgo;

    NOMAD::DirectionType _primaryDirectionType, _secondaryDirectionType;  ///< The poll methods implement different direction types for primary and secondary poll centers.

    NOMAD::Double _rho; ///< Rho parameter of the progressive barrier. Used to choose if the primary frame center is the feasible or infeasible  incumbent.

    size_t _nbEval;
    
    std::vector<std::shared_ptr<NOMAD::PollMethodBase>> _pollMethods;  ///< Unlike for Search, Poll methods generate all their points and only then they are evaluated.

    size_t _n, _nSimple, _nbOutputs;  ///< Pb dimension
    
    NOMAD::Point _fixedVariable;
    
    bool _refineOnPartial;
    bool _twoPointsBarrier;
    
public:
    /// Constructor #1 for model optim
    /**
     */
    explicit MySimplePoll(const NOMAD::Step* parentStep, const NOMAD::BBOutputTypeList & bbot, std::shared_ptr<NOMAD::Evaluator>& eval_x,
        //const NOMAD::EvalPoint & refBestFeas, const NOMAD::EvalPoint & refBestInf,
        NOMAD::EvalPointPtr refBestFeas, NOMAD::EvalPointPtr refBestInf,  
        const NOMAD::EvalPoint & firstFrameCenter, const NOMAD::Double & hMaxMainAlgo)
      : Iteration(parentStep, 0),
        _barrier(nullptr),
        _mesh(nullptr),
        _bbot(bbot),
        _evaluator(eval_x),
        _nbEval(0),
        _refBestFeas(refBestFeas),
        _refBestInf(refBestInf),
        _firstFrameCenter(firstFrameCenter),
        _hMaxMainAlgo(hMaxMainAlgo) 
    {
        init();
    }
    
    virtual ~MySimplePoll() {}

    const std::unique_ptr<NOMAD::SimpleProgressiveBarrier> & getBarrier() const { return _barrier; }
    
    const NOMAD::MeshBasePtr getMesh() const override { return _mesh; }
    
    size_t getNbEval () const { return _nbEval;}
    
    MySuccessType getMySuccessType() const {return _myPollSuccessType; }
    
    const NOMAD::Double getHMaxMainAlgo() {return _hMaxMainAlgo;}

    const std::vector<NOMAD::EvalPoint>& getAllEvaluatedTrialPoints() const { return _allEvaluatedTrialPoints ;}
    

protected:
    /// Helper for start: get lists of Primary and Secondary Polls
    void computePrimarySecondaryPollCenters(NOMAD::SimpleEvalPoint & primaryCenter, NOMAD::SimpleEvalPoint & secondaryCenter) const;
    
    /// Helper for start: create a poll method
    virtual void createPollMethod(const bool isPrimary, const NOMAD::SimpleEvalPoint & frameCenter);

    /// Helper to create poll methods for current poll centers
    virtual void createPollMethodsForPollCenters();
    
    

private:
    /// Helper for constructor
    void init();

    /// Helpers
    void generateTrialPoints();
    void evalTrialPoints();
    bool dominatesRef(const NOMAD::SimpleEvalPoint & p1, const NOMAD::EvalPoint & refBest, const NOMAD::Double hMax) const;
    bool dominates(const NOMAD::SimpleEvalPoint & p1, const NOMAD::SimpleEvalPoint & p2, const NOMAD::Double hMax) const;
    void writeStats(const NOMAD::EvalPoint & ep) const;

    /// Implementation for start tasks for MADS poll.
    /**
     Call to generate the poll methods
     */
    virtual void    startImp()  override {};

    /// Implementation for run tasks for MADS poll.
    /**
     Call poll methods and perform trial points evaluation.
     \return Flag \c true if found better solution \c false otherwise.
     */
    virtual bool    runImp() override;

    /// Implementation for end tasks for MADS simple poll.
    /**
     Call the IterationUtils::postProcessing of the points.
     */
    virtual void    endImp() override ;



};
