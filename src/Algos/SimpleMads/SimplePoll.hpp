#ifndef __NOMAD_4_5_SIMPLEPOLL__
#define __NOMAD_4_5_SIMPLEPOLL__

#include "../../Algos/Mads/GMesh.hpp"
#include "../../Algos/Mads/PollMethodBase.hpp"
#include "../../Algos/QuadModel/QuadModelEvaluator.hpp"
#include "../../Algos/SimpleMads/SimpleProgressiveBarrier.hpp"

#include "../../Math/SimpleRNG.hpp"

#include "../../../ext/sgtelib/src/Surrogate.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for the poll (step 3) of MADS algorithm.
/**
 Generate the trial points (Poll::startImp), launch evaluation (Poll::runImp) and postprocessing (Poll::endImp).
 */
class SimplePoll: public Iteration
{
private:

    std::vector<NOMAD::SimpleEvalPoint> _trialPoints; ///< The points generated during the start(). Used for run() and postProcessing().


    NOMAD::BBOutputTypeList _bbot;

    std::unique_ptr<SimpleProgressiveBarrier> _barrier;
    std::shared_ptr<GMesh> _mesh;

    std::shared_ptr<SGTELIB::Surrogate> _model;

    const singleOutputComputeFType & _singleObjCompute;

    std::vector<SimpleEvalPoint> _frameCenters;  ///< The frame centers (primary and secondary) of the poll methods.

    DirectionType _primaryDirectionType, _secondaryDirectionType;  ///< The poll methods implement different direction types for primary and secondary poll centers.

    NOMAD::Double _rho; ///< Rho parameter of the progressive barrier. Used to choose if the primary frame center is the feasible or infeasible  incumbent.

    size_t _nbEval;

    std::vector<std::unique_ptr<PollMethodBase>> _pollMethods;  ///< Unlike for Search, Poll methods generate all their points and only then they are evaluated.

    size_t _n, _nSimple, _nbOutputs;  ///< Pb dimension

    Point _fixedVariable;

    bool _phaseOneSearch;

    bool _refineOnPartial;
    bool _twoPointsBarrier;
    
    std::shared_ptr<SimpleRNG> _rng = nullptr;

    std::function<bool(std::vector<NOMAD::SimpleEvalPoint>&)> _eval_x; ///< Function for outputs evaluation

public:
    /// Constructor #1 for model optim
    /**
     \param parentStep The parent of this poll step
     */
    explicit SimplePoll(const Step* parentStep, const std::shared_ptr<SGTELIB::Surrogate> & model, const NOMAD::BBOutputTypeList & bbot, const singleOutputComputeFType & singleObjCompute)
      : Iteration(parentStep, 0),
        _barrier(nullptr),
        _mesh(nullptr),
        _model(model),
        _bbot(bbot),
        _nbEval(0),
        _phaseOneSearch(false),
        _singleObjCompute(singleObjCompute)
    {
        init();
    }
    /// Constructor #2, for user function optim
    /**
     \param parentStep The parent of this poll step
     */
    explicit SimplePoll(const Step* parentStep, const NOMAD::BBOutputTypeList & bbot, std::function<bool(std::vector<NOMAD::SimpleEvalPoint> &)> eval_x)
      : Iteration(parentStep, 0),
        _barrier(nullptr),
        _mesh(nullptr),
        _bbot(bbot),
        _nbEval(0),
        _phaseOneSearch(false),
        _eval_x(eval_x),
        _singleObjCompute(NOMAD::defaultEmptySingleOutputCompute)
    {
        init();
    }

    virtual ~SimplePoll() {}

    const std::unique_ptr<SimpleProgressiveBarrier> & getBarrier() const { return _barrier; }

    const MeshBasePtr getMesh() const override { return _mesh; }

    size_t getNbEval () const { return _nbEval;}

    bool getPhaseOneSearch () const { return _phaseOneSearch; }

protected:
    /// Helper for start: get lists of Primary and Secondary Polls
    void computePrimarySecondaryPollCenters(SimpleEvalPoint & primaryCenter, SimpleEvalPoint & secondaryCenter) const;

    /// Helper for start: create a poll method
    // virtual void createPollMethod(const bool isPrimary, const EvalPointPtr frameCenter);
    virtual void createPollMethod(const bool isPrimary, const SimpleEvalPoint & frameCenter);

    /// Helper to create poll methods for current poll centers
    virtual void createPollMethodsForPollCenters();


private:
    /// Helper for constructor
    void init();

    /// Helpers
    void generateTrialPoints();
    void evalTrialPoints();

    NOMAD::Double getF(const NOMAD::ArrayOfDouble & out) const;
    NOMAD::Double getH(const NOMAD::ArrayOfDouble & out) const;

    /// Implementation for start tasks for MADS poll.
    /**
     Call to generate the poll methods
     */
    virtual void    startImp()  override;

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

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SIMPLEPOLL__
