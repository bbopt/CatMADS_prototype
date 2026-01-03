#ifndef __NOMAD_4_5_CACHESEARCHMETHOD__
#define __NOMAD_4_5_CACHESEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/SearchMethodSimple.hpp"


#include "../../nomad_nsbegin.hpp"

/// Class to perform a Cache Search for COOP-Mads optimization algorithm.
/**
TODO
 */
class CacheSearchMethod final : public SearchMethodSimple
{
private:

    
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit CacheSearchMethod(const Step* parentStep)
      : SearchMethodSimple(parentStep)
    {
        init();
    }
    
    virtual bool evalTrialPoints(const Step* step,
                                 const size_t keepN = INF_SIZE_T,
                                 StepType removeStepType = StepType::UNDEFINED) override;

private:
    
    

    /// Helper for constructor.
    /**
     Test if the quad model search is enabled or not. 
     */
    void init();

    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodAlgo::generateTrialPointsFinal 
     
     Perform one quad model optimization to produce trial points.
     */
     virtual void generateTrialPointsFinal() override;
    


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_CACHESEARCHMETHOD__

