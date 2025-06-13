
#ifndef __NOMAD_4_5_CSUPDATE__
#define __NOMAD_4_5_CSUPDATE__

#include "../../Algos/Step.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for Step 1. of CS algorithm: parameter update.
/**
 The update is performed when calling the CSUpdate::run function.
 The update is very similar to MadsUpdate except for the mesh update.
 
  Note: refactoring of MegaIteration could be done to handle the mesh (can be null or not). Than we could use MadsUpdate instead of having a CS update. Issue #642.
 */
class CSUpdate: public Step
{
public:
    // Constructor
    explicit CSUpdate(const Step* parentStep)
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

    /// Implementation of the run tasks.
    /**
     Gets the best feasible point (xFeas) and best infeasible point (xInf)
     from the cache, and updates the MegaIteration's Barrier member with it.
     Compares new values of xFeas and xInf with previous ones
     - i.e., compute success or failure.
     Enlarges or shrinks the delta (mesh size)  ( to remove) and Delta (frame size)
     accordingly.
     */
    virtual bool    runImp()   override;

    /// No implementation is required for end.
    virtual void    endImp()   override {}

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_CSUPDATE__

