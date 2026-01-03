#ifndef __NOMAD_4_5_DMULTIMADSUPDATE__
#define __NOMAD_4_5_DMULTIMADSUPDATE__


#include "../../Algos/Step.hpp"

#include "../../nomad_nsbegin.hpp"

/// The DMultiMads algorithm update step.
/**
 The ref best feasible and ref best infeasible points are updated.
 */
class DMultiMadsUpdate: public Step
{
public:
    // Constructor
    explicit DMultiMadsUpdate(const Step* parentStep)
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
     * Illustrate how the best solution is updated
     *
     \return \c the update status.
     */
    virtual bool    runImp() override ;

    /**
     No end task is required
     */
    virtual void    endImp() override {}

private:
    /// Helper for constructor
    void init();


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DMULTIMADSUPDATE__
