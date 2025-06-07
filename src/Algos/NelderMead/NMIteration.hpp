#ifndef __NOMAD_4_5_NMITERATION__
#define __NOMAD_4_5_NMITERATION__

#include "../../Algos/Iteration.hpp"
#include "../../Algos/NelderMead/NMSimplexEvalPoint.hpp"
#include "../../Eval/MeshBase.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for Nelder Mead (NM) iterations
/**
 The start function for a Nelder Mead iteration creates the initial simplex using a frame ("simplex") center and the points in cache. \n
 The run function of this class iterates between the different reflective step
 (REFLECT, EXPAND, INSIDE_CONTRACTION, OUTSIDE_CONTRACTION) and the SHRINK step
 if it is required. The function also updates the type of success to pass to the NMMegaIteration (if it exists) and manages the stop reason.
 */
class NMIteration: public Iteration
{
private:
    /**
     The simplex is shared among Nelder Mead components. The simplex is initially built by NMInitializeSimplex::run
     */
    std::shared_ptr<NMSimplexEvalPointSet> _nmY ;
    
    bool _nmOpt; ///< NM standalone optimization
    bool _nmSearchStopOnSuccess; ///< Early stop of NM search if a success point is obtained.
    
    
private:
    /// Helper for constructor
    void init ();



    /**
     The simplex "center" at the creation of this iteration.
     The initial simplex is built around this point.
     The frame center of MADS is used when Nelder Mead is used for a search method of MADS.
     */
    const EvalPointPtr _simplexCenter;

    /**
     The Mads mesh can be available if Nelder Mead is used as a Search method. If not, it is set to \c nullptr. When available, trials can be projected on it.
     */
    const MeshBasePtr _madsMesh;

    

public:
    /// Constructor
    /**
     \param parentStep         The parent of this step -- \b IN.
     \param frameCenter        The frame center -- \b IN.
     \param k                  The iteration number -- \b IN.
     \param madsMesh           Mads Mesh for trial point projection (can be null) -- \b IN.
     */
    explicit NMIteration(const Step *parentStep,
                         const EvalPointPtr frameCenter,
                         const size_t k,
                         MeshBasePtr madsMesh)
      : Iteration(parentStep, k),
        _simplexCenter(frameCenter),
        _madsMesh(madsMesh)
    {
        init();

        // Create an empty simplex to be shared among Nelder Mead components
        _nmY = std::make_shared<NMSimplexEvalPointSet>();
    }


    // Get/Set

    const EvalPointPtr getSimplexCenter() const { return _simplexCenter ; }

    const MeshBasePtr getMesh() const override { return _madsMesh; }

    const std::shared_ptr<NMSimplexEvalPointSet> getY( void ) const { return _nmY; }

protected:

    /// Implementation of run task.
    /**
     Sequential run of Nelder Mead steps among INITIAL, ( REFLECT, EXPANSION, INSIDE_CONTRACTION, OUTSIDE_CONTRACTION ), SHRINK.
     Set the stop reason and also updates the Nelder Mead mega iteration success.
     */
    virtual bool runImp() override ;

    /// Implementation of start task.
    /**
     Update the barrier and create the initial simplex if it is empty.
     */
    virtual void startImp() override;


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_NMITERATION__
