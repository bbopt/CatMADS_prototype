#ifndef __NOMAD_4_5_MADSUPDATE__
#define __NOMAD_4_5_MADSUPDATE__

#include "../../Algos/Step.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for Step 1. of MADS algorithm: parameter update.
/**
 The update is performed when calling the MadsUpdate::run function.
 */
class MadsUpdate: public Step
{
    
protected:
    
    bool _clearEvalQueue ; ///< Clear the eval queue between iterations
public:
    // Constructor
    explicit MadsUpdate(const Step* parentStep)
      : Step(parentStep)
    {
        init();
    }

    std::string getName() const override;

private:

    /// Helper for constructor to check for valid ancestor.
    void init();

    /// No implementation is required for start.
    virtual void    startImp() override {}

    /// No implementation is required for end.
    virtual void    endImp()   override {}

protected:
    /// Implementation of the run tasks.
    /**
     Gets the best feasible point (xFeas) and best infeasible point (xInf)
     from the cache, and updates the MegaIteration's Barrier member with it.
     Compares new values of xFeas and xInf with previous ones
     - i.e., compute success or failure.
     Enlarges or shrinks the delta (mesh size) and Delta (frame size)
     accordingly.
     */
    virtual bool    runImp()   override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_MADSUPDATE__
