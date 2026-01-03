
#ifndef __NOMAD_4_5_QUADMODELSLDITERATIONUTILS__
#define __NOMAD_4_5_QUADMODELSLDITERATIONUTILS__

#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/QuadModelSLD/QuadModelSld.hpp"

#include "../../nomad_nsbegin.hpp"


/// Class of utils for QuadModel iterations.
class QuadModelSldIterationUtils : public IterationUtils
{
private:

    void init();

protected:
    
    EvalPointPtr _bestXFeas;
    EvalPointPtr _bestXInf;
    
////    std::shared_ptr<SGTELIB::TrainingSet>   _trainingSet;
    std::shared_ptr<QuadModelSld>     _model;

public:
    /// Constructor
    /**
     The model and training set are obtained from QuadModelIteration.

     \param parentStep      The calling iteration Step.
     */
    explicit QuadModelSldIterationUtils(const Step* parentStep)
      : IterationUtils(parentStep),
        // _trainingSet(nullptr),
        _bestXFeas(nullptr),
        _bestXInf(nullptr),
        _model(nullptr)
    {
        init();
    }

    void displayModelInfo() const;
    
    EvalPointPtr getBestFeas() const { return _bestXFeas; }
    EvalPointPtr getBestInf() const { return _bestXInf; }


};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_QUADMODELSLDITERATIONUTILS__
