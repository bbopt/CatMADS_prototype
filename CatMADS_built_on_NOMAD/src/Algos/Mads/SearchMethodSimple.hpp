#ifndef __NOMAD_4_5_SEARCHMETHODSIMPLE__
#define __NOMAD_4_5_SEARCHMETHODSIMPLE__

#include "../../Algos/Mads/SearchMethodBase.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for simple search methods (no iterations) of MADS. Run by Search.
/**
 - Pure virtual class. Final class derived of this must implement ::generateTrialPointsImp.
  - The trial points information is completed (model or surrogate evals used for sorting) is completed before evaluation.
 - The trial points evaluation (derived classes) are performed when ::runImp is called.
 - Projection on mesh and bounds is performed after ::generateTrialPointsImp is called by the base class SearchMethodBase.
 */
class SearchMethodSimple: public SearchMethodBase
{
        
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit SearchMethodSimple( const Step* parentStep )
      : SearchMethodBase( parentStep )
    {
    }

    /// Intermediate function called to generate the trial points
    /**
     - Call for the intermediate base SearchMethodBase::generateTrialPoints (call generateTrialPointsImp, snap on bounds and mesh).
     - Complete trial points information (sorting is done before evaluation)
     - Sanity check on generated trial points
     - Update the points with frame center
     */
    void startImp() override;

    /// Function called to evaluate the trial points
    /**
     - Evaluate the trial points and update the barrier.
     - The projection of trial points on bounds and on mesh is performed before this function is called and after the function SearchMethodBase::generateTrialPointsImp is called.
     */
    bool runImp() override;
    
    /// Implementation of endImp (not virtual)
    /**
        Must call for SearchMethodBase endImp
        Maybe '
    */
    void endImp() override ;
    
protected:
    
    virtual void updateDynamicEnabled() {} ;
    
    virtual void updateAtStepEnd() {} ;


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SEARCHMETHODSIMPLE__

