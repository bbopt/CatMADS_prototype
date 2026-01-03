#ifndef __NOMAD_4_5_SPECULATIVESEARCHMETHOD__
#define __NOMAD_4_5_SPECULATIVESEARCHMETHOD__

#include "../../Algos/Mads/SearchMethodSimple.hpp"

#include "../../nomad_nsbegin.hpp"


/// Speculative search.
/**
 The speculative search consists in looking further away along
 the successful direction after an improvement.
 */
class SpeculativeSearchMethod  final : public SearchMethodSimple
{
private:
    
    size_t _nbSearches; ///< Number of speculative search trial points for a pass
    
    Double _baseFactor;  ///< Base factor to control the extent of the speculative direction
    
public:
    /// Constructor
    /**
     \param parentStep      The parent of this search step -- \b IN.
     */
    explicit SpeculativeSearchMethod(const Step* parentStep )
    : SearchMethodSimple( parentStep )
    {
        init();
    }

private:
    void init();

    /// Generate new points to evaluate
    /**
     \copydoc SearchMethodSimple::generateTrialPointsFinal \n
     The speculative search generates points in the direction of success.
     */
    void generateTrialPointsFinal() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SPECULATIVESEARCHMETHOD__
