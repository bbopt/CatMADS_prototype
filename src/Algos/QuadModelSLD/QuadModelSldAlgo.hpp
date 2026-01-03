#ifndef __NOMAD_4_5_QUAD_MODEL_SLD_ALGO__
#define __NOMAD_4_5_QUAD_MODEL_SLD_ALGO__

#include "../../Algos/AlgoStopReasons.hpp"
#include "../../Algos/Algorithm.hpp"
#include "../../Algos/EvcInterface.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for implementation of quadratic model optimization algorithm using Bastien Talgorn's sgtelib.
/**
 * Use the start, run and end tasks. Iterate on the following sequence:
 *
 * 1- Points provided as X0s and points in cache are put in a training set.
 * 2- These points are used to build a dynamic model.
 * 3- The model is optimized. This gives oracle points.
 * 4- The oracle points are evaluated by the blackbox.
 * 5- As long as new oracle points are found, the process is repeated.
 *
 * When used by Mads SearchMethod (QuadSearchMethod):
 * - Steps 1, 2, 3 and 4 are the same.
 * - The oracle points are send back to QuadSearchMethod, which takes care
 *   of projecting them to mesh and evaluate them.
 *
 * Training set and model are stored here to allow access to other Quad classes.
 *
 */

class QuadModelSldAlgo: public Algorithm
{
public:
    /// Constructor
    explicit QuadModelSldAlgo(const Step* parentStep,
                           std::shared_ptr<AlgoStopReasons<ModelStopType>> stopReasons,
                           const std::shared_ptr<RunParameters>& runParams,
                           const std::shared_ptr<PbParameters>& pbParams)
      : Algorithm(parentStep, stopReasons, runParams, pbParams)
    {
        init();
    }

    virtual ~QuadModelSldAlgo();


    void readInformationForHotRestart() override {}

private:
    void init();

    bool runImp() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_SLD_ALGO__

