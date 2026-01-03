#ifndef __NOMAD_4_5_QUAD_MODEL_MEGAITERATION__
#define __NOMAD_4_5_QUAD_MODEL_MEGAITERATION__


#include "../../Algos/MegaIteration.hpp"
#include "../../Algos/QuadModel/QuadModelIteration.hpp"

#include "../../nomad_nsbegin.hpp"

/// Manager class for QuadModelAlgo iterations.
/**
 Steps:
 - Start: generate points, using quad model
 - Run: Evaluate points
 - End: Post-processing
*/
class QuadModelMegaIteration: public MegaIteration
{
private:
    std::vector<std::shared_ptr<QuadModelIteration>> _iterList;
    
public:
    /// Constructor
    /**
     \param parentStep      The parent step of this step -- \b IN.
     \param k               The main iteration counter -- \b IN.
     \param barrier         The barrier for constraints handling -- \b IN.
     \param success         Success type of the previous MegaIteration. -- \b IN.
     */
    explicit QuadModelMegaIteration(const Step* parentStep,
                              size_t k,
                              std::shared_ptr<BarrierBase> barrier,
                              SuccessType success)
      : MegaIteration(parentStep, k, barrier, success)
    {
        init();
    }

    virtual ~QuadModelMegaIteration();

private:

    void init();

    virtual void startImp() override ;
    virtual bool runImp() override;
    virtual void endImp() override;


};

/**
 Display useful values so that a new MegaIteration could be constructed using these values.
 */
std::ostream& operator<<(std::ostream& os, const QuadModelMegaIteration& megaIteration);

/// Get an MegaIteration values from a stream
std::istream& operator>>(std::istream& is, QuadModelMegaIteration& megaIteration);

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUAD_MODEL_MEGAITERATION__
