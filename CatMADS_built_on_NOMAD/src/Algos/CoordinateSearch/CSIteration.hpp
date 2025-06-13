#ifndef __NOMAD_4_5_CSSITERATION__
#define __NOMAD_4_5_CSSITERATION__

#include "../../Algos/CoordinateSearch/CSPoll.hpp"
#include "../../Algos/Iteration.hpp"
#include "../../Eval/EvalPoint.hpp"
#include "../../Eval/MeshBase.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for CS iteration
/**
 Unlike a Mads iteration that possesses a Search and Poll step, a CS iteration consists of a Poll step.  A stop reasons and a success type are identified.
  Note: it would be possible to add the same search step as in Mads.
 */
class CSIteration: public Iteration
{
private:
    const MeshBasePtr  _mesh;        ///< Mesh on which the points are

    std::unique_ptr<CSPoll> _csPoll;

// TODO
//#ifdef TIME_STATS
//    /// Time counters
//    DLL_ALGO_API static double  _iterTime;          ///< Total time spent running this class
//    DLL_ALGO_API static double  _searchTime;        ///< Total time spent running searches
//    DLL_ALGO_API static double  _searchEvalTime;    ///< Total time spent evaluating search points
//    DLL_ALGO_API static double  _pollTime;          ///< Total time spent running polls
//    DLL_ALGO_API static double  _pollEvalTime;      ///< Total time spent evaluating poll points
//    double                      _iterStartTime;     ///< Time at which the start method was called
//#endif // TIME_STATS

public:
    /// Constructor
    /**
     \param parentStep         The parent of this step -- \b IN.
     \param k                  The iteration number -- \b IN.
     \param mesh               The mesh of the iteration -- \b IN.
     */
    explicit CSIteration(const Step *parentStep,
                           const size_t k,
                           const MeshBasePtr mesh)
      : Iteration(parentStep, k),
        _mesh(mesh)
// TODO
//#ifdef TIME_STATS
//        ,_iterStartTime(0.0)
//#endif // TIME_STATS
    {
        init();
    }



    // Gets/Sets

    /**
     The CS algorithm iteration possesses a mesh, unlike the base iteration that has none.
     \remark Used by Step::getIterationMesh() to pass the mesh whenever needed
     */
    const MeshBasePtr getMesh() const override { return _mesh; }

//#ifdef TIME_STATS
//    /// Time stats
//    static double getIterTime()         { return _iterTime; }
//    static double getPollTime()         { return _pollTime; }
//    static double getPollEvalTime()     { return _pollEvalTime; }
//#endif // TIME_STATS

    /*---------------------*/
    /* Other class methods */
    /*---------------------*/


private:
    /// Helper for constructor
    void init();

    virtual void startImp() override;

    /// Implementation of the run tasks of CS algorithm.
    /**
     Run a CS  iteration: a single CS Poll step is performed.
     */
    virtual bool runImp() override;

//#ifdef TIME_STATS
//    virtual void endImp() override;
//#endif // TIME_STATS
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_CSITERATION__
