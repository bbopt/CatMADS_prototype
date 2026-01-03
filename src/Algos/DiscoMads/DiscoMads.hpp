/**
 \file   DiscoMads.hpp
 \brief  The DiscoMads algorithm (main)
 \author Solene Kojtych
 \see    DiscoMads.cpp
 */
#ifndef __NOMAD_4_5_DISCOMADS__
#define __NOMAD_4_5_DISCOMADS__

#include "../../Algos/Algorithm.hpp"
#include "../../Algos/AlgoStopReasons.hpp"

#include "../../nomad_nsbegin.hpp"


/// The DiscoMads algorithm. It is implemented to reveal and escape either weak discontinuities or regions of hidden constraints.
/**
\note AllParameters and EvaluatorControl are held by MainStep.
Cache is a singleton all by itself.
MegaIteration holds the algorithm-related structures: Mesh, Barrier.
 */
class DiscoMads: public Algorithm
{
public:
    /// Constructor
    /**
     \param parentStep          The parent of this step -- \b IN.
     \param stopReasons         The stop reasons for DiscoMADS -- \b IN.
     \param runParams           The run parameters that control DiscoMADS -- \b IN.
     \param pbParams            The problem parameters that control DiscoMADS -- \b IN.
     \param barrierInitializedFromCache  Flag to initialize barrier from cache or not -- \b IN.
     \param forceRootAlgo   Flag to make a root algorithm -- \b IN.
     */
    explicit DiscoMads(const Step* parentStep,
                  std::shared_ptr<AlgoStopReasons<MadsStopType>> stopReasons,
                  const std::shared_ptr<RunParameters>& runParams,
                  const std::shared_ptr<PbParameters>& pbParams,
                  bool barrierInitializedFromCache = true,
                  bool forceRootAlgo = true )
      : Algorithm(parentStep, stopReasons, runParams, pbParams, forceRootAlgo)
    {
        init(barrierInitializedFromCache);
    }
      
    virtual ~DiscoMads() {}
  
    /// Helper for hot restart
    void hotRestartOnUserInterrupt() override; 

private:
    ///  Initialization of class, to be used by Constructor.
    /**
    \param barrierInitializedFromCache  Flag to initialized barrier from cache or not -- \b IN.
    */
    void init(bool barrierInitializedFromCache);

    /// Algorithm execution for single-objective.
    /**
     Overrides the default algorithm's run
     \return \c true if a full success was found, \c false otherwise
     */
    virtual bool runImp() override;

    /// Helper for start()
    void readInformationForHotRestart() override;
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DISCOMADS__
