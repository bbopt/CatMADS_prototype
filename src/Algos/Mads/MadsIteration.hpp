#ifndef __NOMAD_4_5_MADSITERATION__
#define __NOMAD_4_5_MADSITERATION__

#include "../../Algos/Iteration.hpp"
#include "../../Algos/Mads/MegaSearchPoll.hpp"
#include "../../Algos/Mads/Poll.hpp"
#include "../../Algos/Mads/ExtendedPoll.hpp"
#include "../../Algos/Mads/Search.hpp"
#include "../../Eval/EvalPoint.hpp"
#include "../../Eval/MeshBase.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for MADS iteration
/**
 A MADS iteration consists of a Search step followed by a Poll step depending on the stop reasons and successes.
 Possibly followed by an ExtendedPoll if provided by user (library mode only).
 */
class MadsIteration: public Iteration
{
protected:
    const MeshBasePtr  _mesh;        ///< Mesh on which the points are

    std::unique_ptr<Poll> _poll;
    std::unique_ptr<Search> _search;
    std::unique_ptr<ExtendedPoll> _extendedPoll;
    std::unique_ptr<MegaSearchPoll> _megasearchpoll;
    
#ifdef TIME_STATS
    /// Time counters
    DLL_ALGO_API static double  _iterTime;          ///< Total time spent running this class
    DLL_ALGO_API static double  _searchTime;        ///< Total time spent running searches
    DLL_ALGO_API static double  _searchEvalTime;    ///< Total time spent evaluating search points
    DLL_ALGO_API static double  _pollTime;          ///< Total time spent running polls
    DLL_ALGO_API static double  _pollEvalTime;      ///< Total time spent evaluating poll points
    DLL_ALGO_API static double  _extendedPollTime;    ///< Total time spent running extended polls
    DLL_ALGO_API static double  _extendedPollEvalTime;///< Total time spent evaluating extended poll points

    double                      _iterStartTime;     ///< Time at which the start method was called
#endif // TIME_STATS

public:
    /// Constructor
    /**
     \param parentStep         The parent of this step -- \b IN.
     \param k                  The iteration number -- \b IN.
     \param mesh               The mesh of the iteration -- \b IN.
     */
    explicit MadsIteration(const Step *parentStep,
                           const size_t k,
                           const MeshBasePtr mesh)
      : Iteration(parentStep, k),
        _mesh(mesh),
        _poll(nullptr),
        _search(nullptr),
        _extendedPoll(nullptr),
        _megasearchpoll(nullptr)
#ifdef TIME_STATS
        ,_iterStartTime(0.0)
#endif // TIME_STATS
    {
        init();
    }

    NOMAD::ArrayOfPoint suggest() override;  ///< For suggest and observe PyNomad interface

    // Gets/Sets

    /**
     The Mads algorithm iteration possesses a mesh, unlike the base iteration that has none.
     \remark Used by Step::getIterationMesh() to pass the mesh whenever needed
     */
    const MeshBasePtr getMesh() const override { return _mesh; }


#ifdef TIME_STATS
    /// Time stats
    static double getIterTime()         { return _iterTime; }
    static double getSearchTime()       { return _searchTime; }
    static double getSearchEvalTime()   { return _searchEvalTime; }
    static double getPollTime()         { return _pollTime; }
    static double getPollEvalTime()     { return _pollEvalTime; }
    static double getExtendedPollTime()         { return _extendedPollTime; }
    static double getExtendedPollEvalTime()     { return _extendedPollEvalTime; }

#endif // TIME_STATS

    /*---------------------*/
    /* Other class methods */
    /*---------------------*/

    const EvalPointSet& getPollTrialPoints() const {return _poll->getTrialPoints(); }
    const EvalPointSet& getSearchTrialPoints() const {return _search->getTrialPoints(); }

private:
    /// Helper for constructor
    void init();

    virtual void startImp() override;

    /// Implementation of the run tasks of MADS algorithm.
    /**
     Run a MADS iteration: a Search step followed by a Poll step depending on the stop reasons and successes.
     */
    virtual bool runImp() override;

#ifdef TIME_STATS
    virtual void endImp() override;
#endif // TIME_STATS
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_MADSITERATION__
