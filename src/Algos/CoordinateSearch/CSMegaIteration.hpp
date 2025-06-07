
#ifndef __NOMAD_4_5_CSMEGAITERATION__
#define __NOMAD_4_5_CSMEGAITERATION__

#include "../../Algos/CoordinateSearch/CSIteration.hpp"
#include "../../Algos/MegaIteration.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for the mega iterations of CS.
/**
Manager for CS iterations.
 Steps:
 - Generate a lot of points over multiple meshes, using a single Poll strategy.
 - Evaluate points
 - Post-processing
*/
class CSMegaIteration: public MegaIteration
{
protected:

    /**
     Main mesh that holds the mesh size and frame size  that we would use in the standard CS algorithm
     */
    MeshBasePtr _mainMesh;
    
    std::unique_ptr<CSIteration> _csIteration;

    void init();

public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param k               The main iteration counter -- \b IN.
     \param barrier         The barrier for constraints handling -- \b IN.
     \param mesh            Mesh on which other Iteration meshes are based -- \b IN.
     \param success         Success type of the previous MegaIteration. -- \b IN.
     */
    explicit CSMegaIteration(const Step* parentStep,
                              size_t k,
                              std::shared_ptr<BarrierBase> barrier,
                              MeshBasePtr mesh,
                              SuccessType success)
      : MegaIteration(parentStep, k, barrier, success),
        _mainMesh(mesh),
        _csIteration(nullptr)
    {
        init();
    }
    virtual ~CSMegaIteration() {}

    /// Implementation of the start tasks for CS mega iteration.
    /**
     Creates a CSIteration for each frame center and each desired mesh size.
     Use all xFeas and xInf available.
     For now, not using other frame centers.
     */
    virtual void startImp() override ;

    /// Implementation of the run tasks for CS mega iteration.
    /**
     Manages the generation of points: either all poll and search points are generated all together before starting evaluation using the MegaSearchPoll or they are generated using a CSIteration with search and poll separately. A run parameter controls the behavior.
     */
    virtual bool runImp() override;


    const MeshBasePtr getMesh() const override         { return _mainMesh; }
    void setMesh(const MeshBasePtr &mesh)      { _mainMesh = mesh; }

    void read(  std::istream& is ) override;
    void display(  std::ostream& os ) const override ;

};

/**
 Display useful values so that a new MegaIteration could be constructed using these values.
 */
std::ostream& operator<<(std::ostream& os, const CSMegaIteration& megaIteration);

/// Get an MegaIteration values from a stream
std::istream& operator>>(std::istream& is, CSMegaIteration& megaIteration);

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_CSMEGAITERATION__

