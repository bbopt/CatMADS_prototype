#ifndef __NOMAD_4_5_QUADANDLHSEARCHMETHOD__
#define __NOMAD_4_5_QUADANDLHSEARCHMETHOD__

#include "../../Algos/Mads/SearchMethodAlgo.hpp"
#ifdef USE_SGTELIB
#include "../../Algos/QuadModel/QuadModelSinglePass.hpp"
#endif

#include "../../nomad_nsbegin.hpp"

/// Class to perform a Search method using the quadratic model optimization algorithm with enriched point from LH.
/**
TODO
 */
class QuadAndLHSearchMethod final : public SearchMethodAlgo
{
private:
    OutputLevel _displayLevel;

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit QuadAndLHSearchMethod(const Step* parentStep)
      : SearchMethodAlgo(parentStep),
        _displayLevel(OutputLevel::LEVEL_NORMAL)
    {
        init();
    }

private:

    /// Helper for constructor.
    /**
     Test if the Quad Search is enabled or not. Test if the Sgtelib library has been linked. Manage displays.
     */
    void init();

    
    bool runImp() override;
    
    void generateTrialPointsFinal() override ;
    
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUADANDLHSEARCHMETHOD__

