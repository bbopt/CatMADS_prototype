
#ifndef __NOMAD_4_5_CSPOLL__
#define __NOMAD_4_5_CSPOLL__

#include "../../Algos/Mads/Poll.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for  CS poll.
/**
  CS Poll is like Mads Poll but there is only one direction type: the +1/-1 base direction for each variable separately.
 */
class CSPoll final : public Poll
{

public:
    /// Constructor
    /**
     /param parentStep      The parent of this poll step -- \b IN.
     */
    explicit CSPoll(const Step* parentStep)
      : Poll(parentStep)
    {
        init();
    }
    
    /// Implementation for start tasks for CS poll.
    /**
     Call to generate the CS poll method
     */
    virtual void    startImp() override ;

protected:
    /// Helper for start: create CS poll method
    void createPollMethods(const bool isPrimary, const EvalPointPtr& frameCenter) override;
    
    virtual void setMeshPrecisionStopType() override;
    
private:
       
    void init();
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_CSPOLL__


