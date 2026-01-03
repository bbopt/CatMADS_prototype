
#ifndef __NOMAD_4_5_CSINITIALIZATION__
#define __NOMAD_4_5_CSINITIALIZATION__

#include "../../Algos/Mads/MadsInitialization.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for CS initialization (step 0)
/**
 The parent run function of this step validates and evaluates X0(s).
 The initialization creates the CS Mesh
 */
class CSInitialization final: public MadsInitialization
{


public:
    /// Constructor
    /*
     \param parentStep      The parent of this step -- \b IN.
     \param barrierInitializedFromCache  Flag to initialize barrier from cache or not -- \b IN.
     */
    explicit CSInitialization(const Step* parentStep, bool barrierInitializedFromCache=true)
      : MadsInitialization(parentStep, barrierInitializedFromCache)
    {
        init();
    }

    virtual ~CSInitialization() {}

    
private:
    void init();


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_CSINITIALIZATION__
