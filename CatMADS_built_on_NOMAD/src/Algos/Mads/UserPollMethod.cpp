#include "../../Algos/Mads/UserPollMethod.hpp"

#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/SubproblemManager.hpp"

void NOMAD::UserPollMethod::init()
{
    setStepType(NOMAD::StepType::POLL_METHOD_USER);
    verifyParentNotNull();

    // Query the enabling parameter here
    if (nullptr == _iterAncestor)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"UserPollMethod: must have an iteration ancestor");
    }
    auto mads = dynamic_cast<const NOMAD::Mads*>(_iterAncestor->getRootAlgorithm());
    if ( nullptr == mads || (!mads->hasUserPollMethod() && !mads->hasUserFreePollMethod()))
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"UserPollMethod: the custom callback function for the user poll method must be added. This works only in library mode. See example in $NOMAD_HOME/examples/advanced/library/CustomPollMethod");
    }

}


// Generate poll directions
void NOMAD::UserPollMethod::generateUnitPollDirections(std::list<NOMAD::Direction> &directions, const size_t n) const
{

    directions.clear();

    auto mads = dynamic_cast<const NOMAD::Mads*>(_iterAncestor->getRootAlgorithm());

    // NOTE: cannot have both USER_METHOD_POLL and USER_METHOD_FREE_POLL callbacks provided.
    bool success;
    if (isFreePoll())
    {
        success = mads->runCallback(NOMAD::CallbackType::USER_METHOD_FREE_POLL, *this, directions, n);
    }
    else
    {
        success = mads->runCallback(NOMAD::CallbackType::USER_METHOD_POLL, *this, directions, n);
    }

    if (!success || directions.empty())
    {
        OUTPUT_INFO_START
        AddOutputInfo("User-defined poll method did not produced directions.");
        OUTPUT_INFO_END
        return;
    }

    // Free user poll method allows to provide any directions.
    // Regular user poll method must follow the same conditions as other poll method.
    if (!isFreePoll())
    {
        bool shownWarningMessage = false;
        for (const auto & dir: directions)
        {
            if (! shownWarningMessage && dir.squaredL2Norm() != 1)
            {
                OUTPUT_INFO_START
                AddOutputInfo("WARNING: User-defined poll method produces directions of L2 norm not equal to one. For proper scaling on the mesh in Mads, the directions should have norm 1.");
                OUTPUT_INFO_END
                shownWarningMessage = true;
            }
        }
    }
}


void NOMAD::UserPollMethod::updateEndUserPoll()
{
    auto mads = dynamic_cast<const NOMAD::Mads*>(_iterAncestor->getRootAlgorithm());

    bool success = mads->runCallback(NOMAD::CallbackType::USER_METHOD_FREE_POLL_END, *this);

    if (!success)
    {
        OUTPUT_INFO_START
        AddOutputInfo("User poll post evaluation function is not working properly.");
        OUTPUT_INFO_END
        return;
    }
}
