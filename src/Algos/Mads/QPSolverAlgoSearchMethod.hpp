#ifndef __NOMAD_4_5_QPSOLVERALGOSEARCHMETHOD__
#define __NOMAD_4_5_QPSOLVERALGOSEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/SearchMethodSimple.hpp"
// #include "../../Algos/QPSolverAlgo/QPSolverAlgo.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform QP optimization.
/**
Generates trial points
 
 
 */
class QPSolverAlgoSearchMethod final : public SearchMethodSimple
{
private:
    OutputLevel _displayLevel;

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit QPSolverAlgoSearchMethod(const Step* parentStep )
      : SearchMethodSimple(parentStep )
    {
        init();
    }

private:

    /// Helper for constructor.
    /**
     Test if search is enabled or not. 
     */
    void init();

    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodSimple::generateTrialPointsFinal
     Single pass for generation of trial points
     */
     void generateTrialPointsFinal() override;
    
    /// Update flag to dynamically enabled or not the search method
    /// Tests have shown this is not efficient. Let's keep the function for future use.
    void updateDynamicEnabled() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QPSOLVERALGOSEARCHMETHOD__

