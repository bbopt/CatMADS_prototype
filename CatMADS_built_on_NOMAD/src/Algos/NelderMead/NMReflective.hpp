#ifndef __NOMAD_4_5_NMREFLECTIVE__
#define __NOMAD_4_5_NMREFLECTIVE__

#include "../../Algos/NelderMead/NMIterationUtils.hpp"
#include "../../Algos/Step.hpp"
#include "../../Eval/EvalPoint.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform the Reflect, Expansion, Inside_Contraction, Outside_Contraction steps of a Nelder Mead algorithm.
/**
  The function NMReflective::getNextNMStepType decides which Nelder Mead step comes next. The next step type is updated when calling run (NMReflective::runImp).\n

 The class maintains the lists of undominated (Y0) and dominated points (Yn) extracted from simplex. See the NM-Mads paper: https://link.springer.com/article/10.1007/s10589-018-0016-0 for details.

 To compare points we use a dominance operator from EvalPoint::dominates.

  * \note The name "reflective" is because all those steps are reflections with different delta.
 */
class NMReflective: public Step, public NMIterationUtils
{
private:
    StepType _nextStepType;
    Double _delta, _deltaE, _deltaOC, _deltaIC;

    EvalPoint _xr, _xe, _xoc, _xic;

    std::vector<EvalPoint> _nmY0; ///< Vector of undominated points extracted from simplex (the simplex has a loose ordering-->tied points can exist).
    std::vector<EvalPoint> _nmYn;  ///< Vector of dominated points extracted from simplex (the simplex has a loose ordering-->tied points can exist).

    size_t _n;
    
    ArrayOfDouble _lb, _ub;
    
public:
    /// Constructor
    /**
     \param parentStep The parent of this NM step
     */
    explicit NMReflective(const Step* parentStep)
      : Step(parentStep),
        NMIterationUtils(parentStep)
    {
        init();
    }
    virtual ~NMReflective() {}

    /// Generate new points to evaluate
    /**
     A new point is obtained using the simplex. xt = yc + dela*d. Delta is the reflective factor. The value depends the step type (REFLECT, EXPAND, INSIDE_CONTRACTION, OUTSIDE_CONTRACTION). yc is the barycenter of the simplex. We have d=yc-yn where yn is the last point of Y. \n

     The point is snapped to bounds and projected on the mesh.
     */
    void generateTrialPointsImp() override;

    /// Set the parameters for a given step type.
    /**
     The name of the step and the value of delta are changed according to stepType.
     */
    void setCurrentNMStepType(StepType stepType);


    StepType getNextNMStepType() const { return _nextStepType ; }

private:

    /**
     The delta parameter used to create the trial point is different for EXPANSION, INSIDE_CONTRACTION, OUTSIDE_CONTRACTION. The possible delta parameters are obtained from _runParams. The validity of the parameters are checked. \n
     The flag to perform a standalone Nelder Mead optimization is also set.
     */
    void init();

    /// Implementation of the start task.
    /**
     Call NMReflective::generateTrialPoints and update the trial points.
     */
    virtual void    startImp() override ;

    /// Implementation of the run task.
    /**
     Evaluate the trial point and store it locally. Call IterationUtils::postProcessing.

     \return \c true if a better point is obtained \c false otherwise.
     */
    virtual bool    runImp() override ;

    /// No end task is required
    virtual void    endImp() override {}

    /// Helper for NMReflective::runImp
    void setNextNMStepType();

    /**
     Reflect is always the first step of Nelder Mead iteration. The reflect point is xr. In NM-Mads paper, depending on which zone xr belongs to, we perform another step:
     - If xr dominates Y0 -> EXPAND
     - If Yn dominates xr -> INSIDE_CONTRACTION
     - If xr dominates 2 points or more in Y -> iteration completed
     - If xr dominates 0 or 1 point in Y -> OUTSIDE_CONTRACTION
     */
    void setAfterReflect() ;


    /**
     EXPAND follows REFLECT. The expand point is xe. \n
     In NM-Mads paper: xr belongs to the expansion zone. xe has been evaluated. The best point between xr and xe is inserted in the simplex Y. If a proper simplex Y is obtained the iteration is completed, if not the next step can be a SHRINK.
     */
    void setAfterExpand();

    /**
     OUTSIDE_CONTRACTION follows REFLECT. The outside contraction point is xoc. \n
     In NM-Mads paper: xr belongs to the outside contraction zone.  The best point between xr and xoc is inserted in Y. If a proper simplex Y is obtained the iteration is completed, if not the next step can be a SHRINK.
     */
    void setAfterOutsideContract();

    /**
     INSIDE_CONTRACTION follows REFLECT. The inside contraction point is xic. \n
     In NM-Mads paper: xr belongs to the inside contraction zone. If xic belongs to the inside contraction zone (that is Yn dominates xic), iteration is completed. Otherwise insert xic in Y. If a proper simplex Y is obtained, the iteration is completed, if not the next step can be a SHRINK.
    */
    void setAfterInsideContract();

    /**
     Insert a point in the simplex Y. If a point is inserted, the last point of Y is removed, return \c true. If Y is unchanged, the insertion failed, return \c false. \n
     Update Y0, Yn and the simplex characteristic if necessary.
     */
    bool insertInY(const EvalPoint& x);

    /**
     Try to insert the best of two points in the simplex.

     \return \c true if the simplex has changed and \c false otherwise.
     */
    bool insertInYBest(const EvalPoint& x1, const EvalPoint& x2);

    /// Helper for the setAfterXXXX functions
    bool pointDominatesY0(const EvalPoint& x) const ;

    /// Helper for the setAfterXXXX functions
    bool YnDominatesPoint(const EvalPoint& x) const ;

    /// Helper for the setAfterXXXX functions
    bool pointDominatesPtsInY(const EvalPoint& x, size_t nb) const ;

    bool makeListY0(); ///< Create the undominated list of points from Y
    bool makeListYn(); ///< Create the dominated list of points from Y

    void displayY0nInfo() const ;
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_NMREFLECTIVE__
