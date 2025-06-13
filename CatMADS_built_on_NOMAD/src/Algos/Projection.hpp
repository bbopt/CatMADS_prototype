
#ifndef __NOMAD_4_5_PROJECTION__
#define __NOMAD_4_5_PROJECTION__

#include "../Algos/IterationUtils.hpp"
#include "../Algos/Step.hpp"

#include "../nomad_nsbegin.hpp"


/// Manage projection of points to mesh
/**
Generalization from projection used in sgtelib in NOMAD 3. See file: Sgtelib_Model_Search.cpp
Currently not implemented, not used, does nothing.
Should be generic for any case that needs projection.
\todo Implement and use
*/
class Projection : public Step, public IterationUtils
{
private:
    EvalPointSet                _oraclePoints;
    OutputLevel                 _displayLevel;

    // Vector of EvalPoints which have a model eval
    std::vector<EvalPoint>      _cacheModelEval;

    // Mesh and frame center to project on
    MeshBasePtr   _mesh;
    EvalPointPtr  _frameCenter;

    std::set<size_t>            _indexSet;
    size_t                      _nbProjTrial;

public:
    // Constructor
    explicit Projection(const Step* parentStep,
                        const EvalPointSet &oraclePoints);

    // Destructor
    virtual ~Projection();

    // Get / Set
    EvalPointSet getOraclePoints() const { return getTrialPoints(); }

private:
    void init();
    void buildIndexSet(const size_t n);

    virtual void startImp() override;
    virtual bool runImp() override;
    virtual void endImp() override;

    void generateTrialPointsImp() override;

    void projectPoint(const EvalPoint& oraclePoint);

    void nonProjectedPoint(const EvalPoint& oraclePoint);

    void stdProjectedPoint(const EvalPoint& oraclePoint);

    Direction computePerturbation(const EvalPoint& oraclePoint, size_t index);

    EvalPoint buildProjectionTrialPoint(const Point& xRef, const Direction& perturbation);

    void doGreedySelection(const EvalPoint& oraclePoint,
                           const EvalPointSet& trySet,
                           std::vector<bool>& keep);

/*
    void evaluateProjectionTrialPoints(const EvalPointSet& trySet,
                           const Evaluator& ev,
                           const std::vector<bool>& keep,
                           EvalPoint& bestEvalPoint);
    */

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_PROJECTION__
