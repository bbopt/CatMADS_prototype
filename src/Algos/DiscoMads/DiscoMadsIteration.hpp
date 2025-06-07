/**
 \file   DiscoMadsIteration.hpp
 \brief  The DiscoMads algorithm main iteration
 \author Solene Kojtych
 \see    DiscoMadsIteration.cpp
 */
#ifndef __NOMAD_4_5_DISCOMADSITERATION__
#define __NOMAD_4_5_DISCOMADSITERATION__


#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/DiscoMads/RevealingPoll.hpp"
#include "../../nomad_nsbegin.hpp"

/// Class for the iterations of DiscoMads
/**
Manager for DiscoMads iterations.

*/
class DiscoMadsIteration: public NOMAD::MadsIteration
{

protected:
    std::unique_ptr<NOMAD::RevealingPoll> _revealingPoll;


public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param k               The main iteration counter -- \b IN.
     \param mesh            Mesh on which other Iteration meshes are based -- \b IN.
     */
    explicit DiscoMadsIteration(const Step *parentStep,
                                const size_t k,
                                const MeshBasePtr mesh)
      : MadsIteration(parentStep, k, mesh),
        _revealingPoll(nullptr)
    {
        init();
    }

private:
    /// Helper for constructor
    void init ();

    /// Implementation of the run tasks of DiscoMADS algorithm.
    /**
     Run a DiscoMads iteration: a Search step followed by a Revealing Poll step followed by a Poll step, depending on the stop reasons and successes.
     If a revelation occurs, the current iteration is immediately terminated (all remaining evaluations are cancelled)
     */
    virtual bool runImp() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DISCOMADSITERATION__
