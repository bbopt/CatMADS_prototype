#ifndef __NOMAD_4_5_TEMPLATEALGOSEARCHMETHOD__
#define __NOMAD_4_5_TEMPLATEALGOSEARCHMETHOD__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Mads/SearchMethodAlgo.hpp"
#include "../../Algos/TemplateAlgo/TemplateAlgo.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class to perform a dummy random search method using an algorithm.
/**
 Randomly generates trial points.
 
 Can be used as a TEMPLATE for a new search method: copy and rename the file and the class name. Adapt the code to your needs. It is IMPORTANT to register the new search method in ../Algos/Mads/Search.cpp (NOMAD::Search::init()).
 
 
 */
class TemplateAlgoSearchMethod final : public SearchMethodAlgo
{
private:
    
    // TEMPLATE for a new search method: A new StopType must be defined in ../Util/StopReason.hpp and ../Util/StopReason.hpp to store the new algorithm accepted stop reasons.
    std::shared_ptr<AlgoStopReasons<RandomAlgoStopType>> _randomAlgoStopReasons;
    
    // TEMPLATE for a new search method: A new Algo must be defined. Create a directory in ../Algos and use the files provided ../Algos/TemplateAlgo to create your own Algorithm.
    std::unique_ptr<TemplateAlgo> _randomAlgo;

public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit TemplateAlgoSearchMethod(const Step* parentStep )
      : SearchMethodAlgo(parentStep ),
        _randomAlgoStopReasons(nullptr),
        _randomAlgo(nullptr)
    {
        init();
    }


    /**
     Execute (start, run, end) of the template algorithm (random). Returns a \c true flag if the algorithm found better point.
     */
    virtual bool runImp() override ;


private:

    /// Helper for constructor.
    /**
     Test if search is enabled or not. Set the maximum number of trial points.
     */
    void init();

    /// Generate new points (no evaluation)
    /**
     \copydoc SearchMethodAlgo::generateTrialPointsFinal 
     Iterative random generation of trial points
     */
     void generateTrialPointsFinal() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_TEMPLATEALGOSEARCHMETHOD__

