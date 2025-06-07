#ifndef __NOMAD_4_5_INITIALIZATION__
#define __NOMAD_4_5_INITIALIZATION__

#include "../Algos/Step.hpp"
#include "../Algos/TrialPointStats.hpp"

#include "../nomad_nsbegin.hpp"

/// Class for initialization (step 0) of an Algorithm
/**
 This an abstract class, each algorithm should probably implement an initialization.
 */
class Initialization: public Step
{
protected:
    
    ArrayOfPoint _x0s;
    size_t _n;
    TrialPointStats                        _trialPointStats;   ///< The trial point counters stats for initialization
    
    std::shared_ptr<BarrierBase> _barrier;   ///< Barrier constructed from evaluated X0s

    
public:
    /// Constructor
    /*
     \param parentStep      The parent of this step -- \b IN.
     */
    explicit Initialization(const Step* parentStep)
      : Step(parentStep),
        _trialPointStats(parentStep),
        _barrier(nullptr)
    {
        init();
    }

    /// Destructor
    /**
     Upon destruction, print all that is in the output queue.
     */
    virtual ~Initialization();

    std::string getName() const override;

    const std::shared_ptr<BarrierBase>& getBarrier() const { return _barrier; }

protected:
    /// Helper for constructor
    void init();
    
    void validateX0s() const;

public:
    virtual void startImp()    override {} ;
    virtual bool runImp()      override = 0;
    virtual void endImp()      override;

private:
    void incrementCounters () override;
    
};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_INITIALIZATION__
