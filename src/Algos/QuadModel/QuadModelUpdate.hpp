#ifndef __NOMAD_4_5_QUAD_MODEL_UPDATE__
#define __NOMAD_4_5_QUAD_MODEL_UPDATE__

#include "../../Algos/Step.hpp"

#include "../../nomad_nsbegin.hpp"

class QuadModelUpdate : public Step
{
private:
    OutputLevel _displayLevel;
    
    /**
     Cache points within a box are selected
     */
    Point _modelCenter;
    ArrayOfDouble _boxSize;
    
    bool _flagUseTrialPointsToDefineBox;
    bool _flagUseScaledModel;
    bool _flagPriorCombineObjsForModel;
    
    bool _buildOnImprovPoints;
    
    const EvalPointSet & _trialPoints;
    const std::vector<Direction> & _scalingDirections;

    const double epsilon = 0.01; ///<  For scaling by directions
    const double epsilonDelta = epsilon/(1.0 - epsilon);
 
    size_t _n ; ///< Pb dimension
    Double _boxFactor ; ///< Box factor (multiplies frame size) to select points when build quad model

    NOMAD::ListOfVariableGroup _listFixVG;
    NOMAD::Point               _forcedFixVG;
    
public:
    explicit QuadModelUpdate(const Step* parentStep,
                             const std::vector<Direction> & scalingDirections,
                             const EvalPointSet & trialPoints,
                             bool flagPriorCombineObjsForModel = false)
      : Step(parentStep),
        _displayLevel(OutputLevel::LEVEL_INFO),
        _flagUseTrialPointsToDefineBox(false),
        _flagUseScaledModel(false),
        _scalingDirections(scalingDirections),
        _trialPoints(trialPoints),
        _flagPriorCombineObjsForModel(flagPriorCombineObjsForModel)
    {
        init();
    }

    virtual ~QuadModelUpdate();

    std::string getName() const override;
    
    void unscalingByDirections( EvalPoint & x );

private:
    void init();

    /**
     No start task is required
     */
    virtual void startImp() override {}

    /// Implementation of the run task.
    /**
     Update the SGTELIB::TrainingSet and SGTELIB::Surrogate contained in the QuadModelIteration ancestor:
     - Get relevant points in cache around current frame center.
     - Add points to training set and build new model.
     - Assess if model is ready. Update its bounds.
     \return \c true if model is ready \c false otherwise.
     */
    virtual bool runImp() override;

    /**
     No end task is required
     */
    virtual void endImp() override {}

    bool isValidForUpdate(const EvalPoint& evalPoint) const; ///< Helper function for cache find.
    bool isValidForUpdateNew(const EvalPoint& evalPoint) const; ///< Helper function for cache find.

    bool isValidForIncludeInModel(const EvalPoint& evalPoint) const; ///< Helper function for cache find.
    
    // ChT Temp for testing
    bool isValidForIncludeInModelAndPlus(const EvalPoint& evalPoint) const; ///< Helper function for cache find.
    
    bool scalingByDirections( Point & x);

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_UPDATE__
