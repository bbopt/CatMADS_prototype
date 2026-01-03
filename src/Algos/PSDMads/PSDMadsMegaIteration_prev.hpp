#ifndef __NOMAD_4_5_PSDMADSMEGAITERATION__
#define __NOMAD_4_5_PSDMADSMEGAITERATION__

#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for the iterations of PSD MADS

class PSDMadsMegaIteration: public MadsMegaIteration
{
private:
    std::shared_ptr<Mads>   _madsOnSubPb; ///< Mads on a subproblem or pollster.
    const Point             _x0; ///< Best point found yet by all subproblems, used as x0
    const Point             _fixedVariable; ///< Fixed variable defining the subproblem

public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param k               The main iteration counter -- \b IN.
     \param barrier         The barrier for constraints handling -- \b IN.
     \param mesh            Mesh on which other Iteration meshes are based -- \b IN.
     \param success         Success type of the previous MegaIteration. -- \b IN.
     \param x0               Best point found yet by all subproblems, used as x0 -- \b IN.
     \param fixedVariable     Fixed variables for subproblem opt. -- \b IN.
     */
    explicit PSDMadsMegaIteration(const Step* parentStep,
                                  const size_t k,
                                  const std::shared_ptr<BarrierBase>& barrier,
                                  const std::shared_ptr<MeshBase>& mesh,
                                  const SuccessType& success,
                                  const Point& x0,
                                  const Point& fixedVariable)
      : MadsMegaIteration(parentStep, k, barrier, mesh, success),
        _madsOnSubPb(nullptr),
        _x0(x0),
        _fixedVariable(fixedVariable)
    {
    }

    virtual ~PSDMadsMegaIteration()
    {
        destroy();
    }

    ///Get / Set
    const std::shared_ptr<Mads>& getMads() const { return _madsOnSubPb; }
    const Point& getFixedVariable() const { return _fixedVariable; }

    /// Implementation of the start tasks for PSD-MADS mega iteration.
    /**
     Creates a MadsIteration for each frame center and each desired mesh size.
     Use all xFeas and xInf available.
     For now, not using other frame centers.
     */
    virtual void startImp() override ;

    /// Implementation of the run tasks for PSD-MADS mega iteration.
    /**
     Manages the generation of points: either all poll and search points are generated all together before starting evaluation using the MegaSearchPoll or they are generated using a MadsIteration with search and poll separately. A run parameter controls the behavior.
     */
    virtual bool runImp() override;

private:
    void setupSubproblemParams(std::shared_ptr<PbParameters> &subProblemPbParams,
                               std::shared_ptr<RunParameters> &subProblemRunParams,
                               const bool isPollster);

    void destroy();
};


#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_PSDMADSMEGAITERATION__
