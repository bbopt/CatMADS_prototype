#ifndef __NOMAD_4_5_SGTELIB_MODEL_ITERATION__
#define __NOMAD_4_5_SGTELIB_MODEL_ITERATION__

#include "../../Algos/Iteration.hpp"
#include "../../Algos/SgtelibModel/SgtelibModelOptimize.hpp"

#include "../../nomad_nsbegin.hpp"

/// Iteration for Sgtelib model deriving from Step
class SgtelibModelIteration: public Iteration
{
private:
    // Optimizer for model on sgtelib model function
    std::shared_ptr<SgtelibModelOptimize> _optimize;

public:
    /// Constructor
    /**
     \param parentStep         The parent of this step -- \b IN.
     \param k                  The iteration number -- \b IN.
     */
    explicit SgtelibModelIteration(const Step *parentStep,
                        const size_t k)
      : Iteration(parentStep, k),
        _optimize(nullptr)
    {
        init();
    }


    // Get/Set
    /// Return oracle points found by the Optimizer
    const EvalPointSet& getOraclePoints() const;


    virtual void startImp() override;
    virtual bool runImp() override;


private:
    void init();

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SGTELIB_MODEL_ITERATION__
