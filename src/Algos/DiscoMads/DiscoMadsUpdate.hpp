/**
 \file   DiscoMadsUpdate.cpp
 \brief  The DiscoMads algorithm update step: implementation
 \author Solene Kojtych
 \see    DiscoMadsUpdate.hpp
 */
#ifndef __NOMAD_4_5_DISCOMADSUPDATE__
#define __NOMAD_4_5_DISCOMADSUPDATE__

#include "../../Algos/Step.hpp"
#include "../../Algos/Mads/MadsUpdate.hpp"
#include "../../nomad_nsbegin.hpp"

/// Class for Step 1. of DiscoMADS algorithm: parameter update, inherited from MadsUpdate but with special treatment if revealing iteration
/**
 The update is performed when calling the DiscoMadsUpdate::run function.
 */
class DiscoMadsUpdate: public MadsUpdate
{
    
public:
    // Constructor
    explicit DiscoMadsUpdate(const Step* parentStep)
      : MadsUpdate(parentStep)
    {
        init();
    }

private:

    /// Helper for constructor to check for valid ancestor.
    void init();

    /// I1: comment
    virtual void    startImp() override {}

    /// Implementation of the run tasks.
    /**
     If last iteration is revealing : keep same delta (mesh size) and Delta (frame size).
     If last iteration is not revealing, do the update of Mads (MadsUpdate)
     */
    virtual bool    runImp()   override;


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DISCOMADSUPDATE__
