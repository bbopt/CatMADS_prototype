#ifndef __NOMAD_4_5_NMUPDATE__
#define __NOMAD_4_5_NMUPDATE__


#include "../../Algos/Step.hpp"

#include "../../nomad_nsbegin.hpp"

/// Nelder Mead algorithm update step.
/**
 The ref best feasible and ref best infeasible points are updated.
 */
class NMUpdate: public Step
{
public:
    // Constructor
    explicit NMUpdate(const Step* parentStep)
      : Step(parentStep)
    {
        init();
    }

    std::string getName() const override;

    /**
     No start task is required
     */
    virtual void    startImp() override {}

    /**
     * No run required. Nothing to do.
     * In MADS algorithm, Update enlarges or refines the mesh.
     * Before February 2020, Update also updated the Barrier. Now, the steps
     * take care of this during postProcessing.
     *
     \return \c true (always).
     */
    virtual bool    runImp() override { return true; }

    /**
     No end task is required
     */
    virtual void    endImp() override {}

private:
    /// Helper for constructor
    void init();


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_NMUPDATE__
