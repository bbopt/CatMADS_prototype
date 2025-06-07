#ifndef __NOMAD_4_5_ITERATION__
#define __NOMAD_4_5_ITERATION__

#include "../Algos/Step.hpp"

#include "../nomad_nsbegin.hpp"

/// Class for iteration of an Algorithm.
/**
 This is an abstract class, each algorithm must implement its own iteration.
 */
class Iteration: public Step
{
    
protected:
    bool _userCallbackEnabled;
    
protected:

    size_t _k; ///< Iteration number

    void init(); ///< Utility for constructor

public:
    /// Constructor
    /**
     \param parentStep         The parent of this step -- \b IN.
     \param k                  The initial iteration counter -- \b IN.
     */
    explicit Iteration(const Step *parentStep,
                       const size_t k)
      : Step( parentStep ),
        _k(k)
    {
        init();
    }

    /// Destructor
    /**
     When iteration is done, Flush prints output queue.
     */
    virtual ~Iteration();

    // Get/Set
    /// Get name
    virtual std::string getName() const override;

    /// Get iteration number
    /**
     Iteration number is incremented when calling the default Iteration::start().
     */
    size_t getK() const { return _k; }

    /**
     \return \c nullptr for algorithms that do not use a mesh. Otherwise, this function must be reimplemented in algorithm specific iteration (for example, MadsIteration, NMIteration).
     */
    virtual const MeshBasePtr getMesh() const { return nullptr; }
    
    
    // virtual const EvalPointSet& getMethodsTrialPoints() const { return emptyEvalPointSet; }

protected:

    /**
     This must be implemented when an algorithm has its own iteration.
     */
    virtual void startImp()    override = 0;

    /**
     This must be implemented when an algorithm has its own iteration.
     */
    virtual bool runImp()      override = 0;

    /**
     The default implement for end function displays the stop reason, calls the customized end function if provided by the user and increment the counter. \n
     If an end implementation function specific to an algorithm is required, this function MUST be called for default tasks.
     */
     virtual void endImp()      override;

private:
    
    /// Implementation to increment the iteration counter
    virtual void incrementCounters() override { _k++ ;}

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_ITERATION__
