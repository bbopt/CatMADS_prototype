#ifndef __NOMAD_4_5_ALGOSTOPREASONS__
#define __NOMAD_4_5_ALGOSTOPREASONS__

#include <memory>   // for shared_ptr
#include "../Algos/EvcInterface.hpp"    // For access to EvalMainThreadStopType
#include "../Util/Exception.hpp"
#include "../Util/AllStopReasons.hpp"

#include "../nomad_nsbegin.hpp"


/// Template class for algorithm stop reasons.
/**

 The class is templated with a StopType defined according to which algorithm is considered. For example, we have ::MadsStopType, ::LHStopType and ::NMStopType. \n
 At some point during an algorithm a stop reason is set. It can be specific to the algorithm or generic (that is be an AllStopReasons stop type). \n
 The stop reasons in AllStopReasons are private and not directly accessible. But the AlgoStopReasons::checkTerminate() function, checks both AllStopReasons and AlgoStopReasons.
 */
template <typename StopType>
class AlgoStopReasons : public AllStopReasons
{
public:
    /// Constructor
    /*
     */
    explicit AlgoStopReasons() : AllStopReasons()
    {
    }

    /// Destructor
    virtual ~AlgoStopReasons()
    {}


private:
    StopReason<StopType> _algoStopReason;   ///< Stop reason specific to this algorithm

public:
    /// Set the algo stop reason to a specific stop type.
    void set(StopType s)
    {
        _algoStopReason.set(s);
    }

    StopType getStopType() const
    {
        return _algoStopReason.get();
    }

    std::string getStopReasonAsString() const override
    {
        std::string stopReasonStr = AllStopReasons::getStopReasonAsString();

        if (!_algoStopReason.isStarted())
        {
            stopReasonStr += _algoStopReason.getStopReasonAsString() + " (Algo)";
        }

        auto evc = NOMAD::EvcInterface::getEvaluatorControl();
        if (nullptr != evc)
        {
            int mainThreadNum = NOMAD::getThreadNum();  // The code in Algos/ is always called from a main thread.
            auto evalStopReason = evc->getStopReason(mainThreadNum);
            if (!evalStopReason.isStarted())
            {
                stopReasonStr += (stopReasonStr.empty() ? "" : " ") + evalStopReason.getStopReasonAsString();
            }
        }

        return stopReasonStr;
    }


    /// Check among generic stop reasons and algo stop reason if the algorithm must terminate
    bool checkTerminate() const override
    {
        auto evc = NOMAD::EvcInterface::getEvaluatorControl();
        return (NOMAD::AllStopReasons::checkTerminate()
                || _algoStopReason.checkTerminate()
                || ((nullptr != evc) && evc->getStopReason(NOMAD::getThreadNum()).checkTerminate()));
    }


    /// Access to the AlgoStopReasons
    static std::shared_ptr<AlgoStopReasons<StopType>> get(std::shared_ptr<AllStopReasons> allStopReasons)
    {
        std::shared_ptr<AlgoStopReasons<StopType>> stopReasons = std::dynamic_pointer_cast<AlgoStopReasons<StopType>>( allStopReasons );

        if ( stopReasons == nullptr )
            throw Exception(__FILE__, __LINE__, "Invalid shared pointer cast");
        return stopReasons;
    }


    /// Test for a specific algorithm stop type.
    /**
     Used to pass a sub-algorithm stop reason to a parent algorithm stop reason.
     */
    bool testIf (const StopType& s) const
    {
        return ( _algoStopReason.get() == s );
    }


    /// Reset stop reasons to their default STARTED state.
    void setStarted() override
    {
        _algoStopReason.setStarted();
        AllStopReasons::setStarted();
        auto evc = NOMAD::EvcInterface::getEvaluatorControl();
        if (nullptr != evc)
        {
            evc->setStopReason(NOMAD::getThreadNum(), NOMAD::EvalMainThreadStopType::STARTED);
        }
    }

};


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_ALGOSTOPREASONS__
