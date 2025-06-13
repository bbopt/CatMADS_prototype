#ifndef __NOMAD_4_5_VNSMARTALGOSEARCHMETHOD__
#define __NOMAD_4_5_VNSMARTALGOSEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/SearchMethodAlgo.hpp"
#include "../../Algos/VNSMads/VNS.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Algos/Algorithm.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform a dummy random search method using an algorithm.
/**
 Randomly generates trial points.
 
 Can be used as a TEMPLATE for a new search method: copy and rename the file and the class name. Adapt the code to your needs. It is IMPORTANT to register the new search method in ../Algos/Mads/Search.cpp (NOMAD::Search::init()).
 
 
 */
class VNSmartAlgoSearchMethod final : public SearchMethodAlgo
{
private:
    
    OutputLevel _displayLevel;

    Point   _refFrameCenter;    ///< The reference frame center for the last call. If frame center same as reference, do not perform search.
    /**
        The algorithm used by the search method.
     */
    std::unique_ptr<VNS> _vnsAlgo;
    
    /**
        VNS has its own stop reasons
     */
    std::shared_ptr<NOMAD::AlgoStopReasons<NOMAD::VNSStopType>>    _vnsStopReasons;
    
    // Threshold
    int _stopConsFailures; // Threshold for the number of successive failure

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit VNSmartAlgoSearchMethod(const Step* parentStep )
    : SearchMethodAlgo(parentStep),
      _displayLevel(OutputLevel::LEVEL_NORMAL),
      _vnsAlgo(nullptr),
      _stopConsFailures(P_INF_INT)
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

#endif // __NOMAD_4_5_VNSMARTALGOSEARCHMETHOD__

