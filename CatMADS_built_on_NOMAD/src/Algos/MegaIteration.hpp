#ifndef __NOMAD_4_5_MEGAITERATION__
#define __NOMAD_4_5_MEGAITERATION__

#include "../Algos/Iteration.hpp"
#include "../Algos/Step.hpp"

#include "../nomad_nsbegin.hpp"

/// Class to manage the iterations.
/**
 A mega iteration of an algorithm:
 *  - generates a lot of points over multiple strategies (ex., Poll and Search for Mads).
 *  - evaluates points
 *  - performs post-processing

 The run and start functions of mega iteration must be implemented for each optimizer
 that has several phases of points creation that we want to combine before launching evaluations (for example, ::MadsMegaIteration, ::NMMegaIteration, ...).

  \note As an hypothesis, the time load is taken by the evaluation,
  which is parallelized over all evaluations simultaneously.
  The iteration generation, including trial points generation,
  has little time load, so they do not need to be parallelized.
  It is also preferable to keep parallelization to the only place where
  it matters the most to avoid errors. \n
  There is no parallelization at the algorithmic level.
  The algorithms are run in main thread(s) only; other threads are available for evaluations.
*/
class MegaIteration: public Step
{
protected:
    /**
     The barrier holds xFeas, xInf and hMax.
    xFeas and xInf will be used as frame centers.
    \note This barrier is in subspace.
     */
    std::shared_ptr<BarrierBase> _barrier;

    size_t _k;          ///< Main counter

    
    // TODO check if _megaIterationSuccess is needed
    /**
     Success type of this step. Initialized with the run of the previous
     mega iteration, so that the update step knows what to do
     (for example, enlarge or reduce the mesh).
     At the end of run step during the mega iteration of an algorithm,
     the success type is updated (MegaIteration::setSuccessType) with the latest
     success type.
     */
    SuccessType _megaIterationSuccess;

public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param k               The initial main iteration counter -- \b IN.
     \param barrier         The barrier for constraints handling. Belongs to subproblem space. -- \b IN.
     \param success         Success type of the previous MegaIteration. -- \b IN.
     */
    explicit MegaIteration(const Step* parentStep,
                              size_t k,
                              std::shared_ptr<BarrierBase> barrier,
                              SuccessType success);

    virtual ~MegaIteration() {}

    /*---------*/
    /* Get/Set */
    /*---------*/
    std::string getName() const override;

    size_t getK() const                                         { return _k; }
    void setK(const size_t k)                                   { _k = k; }


    // Barrier
    const std::shared_ptr<BarrierBase>& getBarrier() const          { return _barrier; }
    void setBarrier(const std::shared_ptr<BarrierBase> &barrier)    { _barrier = barrier; }
    
    /**
     \return \c nullptr for algorithms that do not use a mesh. Otherwise, this function must be reimplemented in algorithm specific MegaIteration (for example, MadsMegaIteration).
     */
    virtual const MeshBasePtr getMesh() const { return nullptr; }

    /// Compute the number of xFeas and xInf points to use to create iterations
    void computeMaxXFeasXInf(size_t &maxXFeas, size_t &maxXInf);

    /*---------*/
    /* Others  */
    /*---------*/

    virtual void read(std::istream& is);
    virtual void display(std::ostream& os) const ;

protected:
    /// Helper for constructor
    void init();

    virtual void startImp()    override ;
    virtual bool runImp()      override = 0;

    /// Implementation of the end task.
    /**
     The default implement performs callback check for user termination AND increment counter k.
     If an end implementation function specific to an algorithm is required, this function MUST be called for default tasks.
     */
     void endImp()      override;
    
private:
    
    /// Implementation to increment the mega iteration counter
    virtual void incrementCounters() override { _k++ ;}
};


/**
 Display useful values so that a new MegaIteration could be constructed using these values.
 */
std::ostream& operator<<(std::ostream& os, const MegaIteration& megaIteration);

/// Get an MegaIteration values from a stream
std::istream& operator>>(std::istream& is, MegaIteration& megaIteration);


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_MEGAITERATION__
