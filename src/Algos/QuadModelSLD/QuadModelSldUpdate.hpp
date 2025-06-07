#ifndef __NOMAD_4_5_QUAD_MODEL_SLD_UPDATE__
#define __NOMAD_4_5_QUAD_MODEL_SLD_UPDATE__

#include "../../Algos/Step.hpp"

#include "../../nomad_nsbegin.hpp"

class QuadModelSldUpdate : public Step
{
private:
    OutputLevel _displayLevel;
    
    /**
     Cache points within a box are selected
     */
    Point _modelCenter;
    ArrayOfDouble _boxSize;
    
    bool _flagUseTrialPointsToDefineBox;
    
    const EvalPointSet & _trialPoints;

    const double epsilon = 0.01; ///<  For scaling by directions
    const double epsilonDelta = epsilon/(1.0 - epsilon);
    
    
public:
    explicit QuadModelSldUpdate(const Step* parentStep,
                             const EvalPointSet & trialPoints )
      : Step(parentStep),
        _displayLevel(OutputLevel::LEVEL_INFO),
        _flagUseTrialPointsToDefineBox(false),
        _trialPoints(trialPoints)
    {
        init();
    }

    virtual ~QuadModelSldUpdate();

    std::string getName() const override;
    
    bool unscale(EvalPoint & ep) const;
    
    // void unscalingByDirections( EvalPoint & x );

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

    bool isValidForIncludeInModel(const EvalPoint& evalPoint) const; ///< Helper function for cache find.


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_SLD_UPDATE__
