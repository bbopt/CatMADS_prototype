/**
 \file   CallbackType.hpp
 \brief  Types for Callback
 \author Viviane Rochon Montplaisir
 \date   November 2019
 \see    CallbackType.cpp
 */


#ifndef __NOMAD_4_5_CALLBACK_TYPE__
#define __NOMAD_4_5_CALLBACK_TYPE__

#include "../nomad_nsbegin.hpp"

enum class CallbackType
{
    ITERATION_END,      ///< Called at the end of an Iteration
    MEGA_ITERATION_START, ///< Called at the start of a MegaIteration (after defaultStart, that is,  _success is reset to undefined. After Update to have mesh/barrier updated)
    MEGA_ITERATION_END, ///< Called at the end of a MegaIteration (after defaultEnd, no update done but success is up to date)
    EVAL_OPPORTUNISTIC_CHECK, ///< Called after each evaluation for a special opportunistic check for cancelling remaining queue evaluations (maybe not a success) or propagate stop to iteration (customOpportunisticIterStop)
    EVAL_FAIL_CHECK, ///< Called after each failed evaluation for handling eval point: re-evaluated fail or change the eval status and update value (DiscoMads for hidden constraints)
    EVAL_STOP_CHECK, ///< Called after each evaluation for handling global stop
    PRE_EVAL_UPDATE,  ///< Called before each evaluation for discarding an evaluation point (using a user surrogate for example)
    PRE_EVAL_BLOCK_UPDATE,  ///< Called before each evaluation for discarding an evaluation point (using a user surrogate for example)
    POST_EVAL_UPDATE, ///< Called just after each evaluation (in EvaluatorControl::evalBlock) for a special update defined by the user
    POSTPROCESSING_CHECK, ///< Called during postProcessing after trial points have been evaluated. Step is passed for check but evalPoint is not (this is different than Eval Stop Check)
    HOT_RESTART,        ///< Called at the beginning of Hot Restart process
    USER_METHOD_SEARCH,  ///< Called for a user search method
    USER_METHOD_SEARCH_2,  ///< Called for the second user search method
    USER_METHOD_SEARCH_END,  ///< Called after evaluation of trial points proposed by user search method
    USER_METHOD_POLL,       ///< Called for a user poll method
    USER_METHOD_FREE_POLL,    ///< Called for a user poll method as a third pass independent of the regular method.
    USER_METHOD_FREE_POLL_END,   ///< Called after evaluation of trial points proposed by user free poll method
};


#include "../nomad_nsend.hpp"
#endif  // __NOMAD_4_5_CALLBACK_TYPE__
