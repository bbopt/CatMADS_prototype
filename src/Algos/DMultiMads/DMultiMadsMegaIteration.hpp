#ifndef __NOMAD_4_5_DMULTIMADSMEGAITERATION__
#define __NOMAD_4_5_DMULTIMADSMEGAITERATION__

#include "../../Algos/MegaIteration.hpp"
#include "../../Algos/DMultiMads/DMultiMadsIteration.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for DMultiMads mega iteration.
/**
 * Manager for DMultiMads iterations starts, runs and ends.
 * Steps:
 * - Generate points over simplices (start).
 * - Evaluate points (run)
 * - Post-processing (end)
 */
class DMultiMadsMegaIteration: public MegaIteration
{
protected:

    std::shared_ptr<DMultiMadsIteration> _dMultiMadsIteration;

private:
    
    /**
     Initial mesh to generate trial points. After first iteration, use the mesh associated with the frame center.
     */
    const MeshBasePtr _initialMesh;
    
    void init();

public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param k               The main iteration counter -- \b IN.
     \param barrier         The barrier for constraints handling -- \b IN.
     \param initialMesh               The initial mesh to pass to the iteration -- \b IN.
     \param success         Success type of the previous MegaIteration. -- \b IN.
     */
    explicit DMultiMadsMegaIteration(const Step* parentStep,
                              size_t k,
                              std::shared_ptr<BarrierBase> barrier,
                              MeshBasePtr initialMesh,
                              SuccessType success)
    : MegaIteration(parentStep, k,barrier,success),
    _dMultiMadsIteration(nullptr),
    _initialMesh(initialMesh)
    {
        init();
    }
    // No Destructor needed - keep defaults.

    /**
     \return Mesh hold by DMultiMadsIteration.
     */
    const MeshBasePtr getMesh() const override { return _dMultiMadsIteration->getMesh() ; }
    
    void read(  std::istream& is ) override;
    void display(  std::ostream& os ) const override ;

private:

    /// Implementation of start task.
    /**
     \note Running the algorithm requires a single iteration object with several start, run, end for the various iterations of the algorithm.
     */
    virtual void startImp() override ;

    /// Implementation of run task.
    /**
     The algorithm iterations are started, ran and ended sequentially until a stop reason to terminate is obtained. \n
     We have a success if either a better xFeas or
     a dominating or partial success for xInf was found.
     See Algorithm 12.2 from DFBO.
     */
    virtual bool runImp() override;
    
    virtual void endImp() override;
    

};

/**
 Display useful values so that a new MegaIteration could be constructed using these values.
 */
std::ostream& operator<<(std::ostream& os, const DMultiMadsMegaIteration& megaIteration);

/// Get an MegaIteration values from a stream
std::istream& operator>>(std::istream& is, DMultiMadsMegaIteration& megaIteration);

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DMULTIMADSMEGAITERATION__
