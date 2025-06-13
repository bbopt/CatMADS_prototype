#ifndef __NOMAD_4_5_VNSSEARCHMETHOD__
#define __NOMAD_4_5_VNSSEARCHMETHOD__

#include <string>

#include "../../Algos/Mads/SearchMethodAlgo.hpp"
#include "../../Algos/VNSMads/VNS.hpp"

#include "../../nomad_nsbegin.hpp"

/// Implementation of VNS Mads search method
class VNSSearchMethod final: public SearchMethodAlgo
{
private:
    OutputLevel _displayLevel;

    Point   _refFrameCenter;    ///< The reference frame center for the last call. If frame center same as reference, do not perform search.
        
    double _trigger; ///< Evaluation ratio (vns evals vs all evals) to trigger vns search

    bool _VNSUseSurrogate; ///< Flag to enable VNS with surrogate.
    
    /**
        The algorithm used by the search method.
     */
    std::unique_ptr<VNS> _vnsAlgo;
    
    /**
        VNS has its own stop reasons
     */
    std::shared_ptr<NOMAD::AlgoStopReasons<NOMAD::VNSStopType>>        _vnsStopReasons;

    
/*----------------------------------------------------------------------*/


public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit VNSSearchMethod(const Step* parentStep)
      : SearchMethodAlgo(parentStep),
        _displayLevel(OutputLevel::LEVEL_NORMAL),
        _vnsAlgo(nullptr)
    {
        init();
    }

private:
    void init();
    
    bool runImp() override;

    ///Generate new points (no evaluation)
    /*! \copydoc SearchMethodBase::generateTrialPointsFinal() /n
     * This function is used only when a VNS MADS search with
     * the option to generate all points before evaluation. It performs a single
     * Mads iteration (search and poll) around the best incumbent points in the Barrier.
     */
    void generateTrialPointsFinal() override;


    
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_VNSSEARCHMETHOD__

