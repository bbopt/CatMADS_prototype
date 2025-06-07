#ifndef __NOMAD_4_5_USERSEARCHMETHOD__
#define __NOMAD_4_5_USERSEARCHMETHOD__

#include "../../Algos/Mads/SearchMethodSimple.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class UserSearchMethod: Search method defined by user.
/// User search can be combined with other search methods.
/// Each search methods generate and evaluate trial points independently.
/// Managing fixed variables is possible but requires to use updateAtStepEnd()
/// to revert the value of fixed variables if required.
/// Snap to bounds and on mesh is done automatically.
class UserSearchMethod: public SearchMethodSimple
{
    const size_t _id;
    
public:
    /// Constructor
    /**
     \param parentStep      The parent of this search step -- \b IN.
     \param id                        The user search number (2 are available) --\b IN.
     */
    explicit UserSearchMethod(const Step* parentStep, size_t id=0)
      : SearchMethodSimple(parentStep),
       _id(id)
    {
        init();
    }

private:

    /// Helper for constructor
    void init();

    /**
     \copydoc SearchMethodSimple::generateTrialPointsFinal \n
     */
    virtual void generateTrialPointsFinal() override;
    
    
    void updateDynamicEnabled() override {};
    
    void updateAtStepEnd() override;
    

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_USERSEARCHMETHOD__

