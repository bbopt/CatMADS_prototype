
#ifndef __NOMAD_4_5_NMITERATIONUTILS__
#define __NOMAD_4_5_NMITERATIONUTILS__

#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/NelderMead/NMIteration.hpp"
#include "../../Algos/NelderMead/NMSimplexEvalPoint.hpp"
#include "../../Algos/Step.hpp"

#include "../../nomad_nsbegin.hpp"


/// Class of utils for NM iterations.
/**
 - Manage the simplex: update the characteristics (diameter, volume and normalized volume). The diameter is max(distance(y_i,y_j)). The volume is det(y_k-y_0)/!n (k=1,..n). The normalized volume is volume/diameter^n. \n

 - Hold a variable NMIterationUtils::_currentStepType of type ::StepType for the phase of Nelder Mead algorithm.
 - Calculate the rank of DZ=[y_i-y_0] using NMIterationUtils::_rankEps as trigger (see ::getRank function)

 */
class NMIterationUtils : public IterationUtils
{
private:

    // Simplex characteristics
    double _simplexDiam, _simplexVol, _simplexVon ;
    const EvalPoint * _simplexDiamPt1; /// First point used for simplex diameter
    const EvalPoint * _simplexDiamPt2; /// Second point used for simplex diameter

    ArrayOfDouble _Delta;  /// Delta mads frame size for scaling DZ (can be undefined)

    /// Helper for NMIterationUtils::updateYCharacteristics
    void updateYDiameter ( void );

protected:
    /// The precision for the rank calculation. Default is ::DEFAULT_EPSILON.
    Double _rankEps;

    /// The step type (REFLECT, EXPAND, INSIDE_CONTRACTION, OUTSIDE_CONTRACTION)
    StepType _currentStepType;

    std::shared_ptr<NMSimplexEvalPointSet> _nmY;  ///< The Nelder Mead simplex.

    /// Update the simplex diameter and volumes from NMIterationUtils::_nmY
    void updateYCharacteristics ( void ) ;

    /// Display all the characteristics of a simplex
    /**
     The diameter is max(distance(y_i,y_j)). The volume is det(y_k-y_0)/!n (k=1,..n). The normalized volume is volume/diameter^n.
     */
    void displayYInfo ( void ) const ;

    /**
     \return The rank of DZ=[y_i-y_0]/Delta (Delta can be ones if mesh is not available)
     */
    int getRankDZ ( ) const ;

    /// Set the stop reason according to NMIterationUtils::_currentStepType
    void setStopReason ( ) const;

public:
    /// Constructor
    /**
     The simplex is obtained from NMIteration.

     \param parentStep      The calling iteration Step.
     */
    explicit NMIterationUtils(const Step* parentStep)
      : IterationUtils(parentStep),
        _simplexDiam(0),
        _simplexVol(0),
        _simplexVon(0),
        _simplexDiamPt1(nullptr),
        _simplexDiamPt2(nullptr),
        _Delta(ArrayOfDouble()),
        _rankEps(DEFAULT_EPSILON),
        _currentStepType(StepType::NM_UNSET),
        _nmY(nullptr)
    {
        auto iter = dynamic_cast<const NMIteration*>(_iterAncestor);
        if ( nullptr != iter )
        {
            _nmY = iter->getY();

            // If a Mads mesh is available, initialize the frame size.
            auto madsMesh = iter->getMesh();
            if ( nullptr !=
                madsMesh )
            {
                _Delta = madsMesh->getDeltaFrameSize();
            }
        }
    }


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_NMITERATIONUTILS__
