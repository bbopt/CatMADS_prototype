#ifndef __NOMAD_4_5_ALLSTOPREASONS__
#define __NOMAD_4_5_ALLSTOPREASONS__

#include "../nomad_platform.hpp"
#include "../Util/StopReason.hpp"

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"


/// Class combining all stop reasons that are not algorithmic stop reasons.
/**

 Several stop reasons are members of this class. The stop reasons are templated on stop type. Several stop types are available in this class:
 - a ::BaseStopType for high level stop reasons.
 - an ::EvalGlobalStopType for global evaluation stop reasons.
 - an ::IterStopType for stop reasons during iteration of an algorithm (for example, maximum iteration number reached).

 The static stop reasons ::BaseStopType and ::EvalGlobalStopType are shared.
 */
class DLL_UTIL_API AllStopReasons
{
public:
    /// Constructor
    explicit AllStopReasons ()
    {
    }

    /// Destructor
    virtual ~AllStopReasons()
    {}

private:
    static StopReason<BaseStopType> _baseStopReason; ///< A single base stop reason is considered for NOMAD.
    static StopReason<EvalGlobalStopType> _evalGlobalStopReason; ///< An eval stop reason valuable for the whole of NOMAD.
    StopReason<IterStopType> _iterStopReason; ///< An iteration stop reason.

public:
    /*---------*/
    /* Get/Set */
    /*---------*/

    static const StopReason<BaseStopType>& getBaseStopReason() { return _baseStopReason; }
    const StopReason<EvalGlobalStopType>& getEvalGlobalStopReason() { return _evalGlobalStopReason; }
    const StopReason<IterStopType>& getIterStopReason() const { return _iterStopReason; }

    static void set(const BaseStopType& s)
    {
        _baseStopReason.set(s);
    }

    static void set(const EvalGlobalStopType& s)
    {
        _evalGlobalStopReason.set(s);
    }

    void set(const IterStopType& s)
    {
        _iterStopReason.set(s);
    }

    /*---------*/
    /* Other   */
    /*---------*/

    /// Test static BaseStopType
    static bool testIf(const BaseStopType& s)
    {
        return (_baseStopReason.get() == s);
    }

    /// Test static EvalGlobalStopType
    static bool testIf (const EvalGlobalStopType& s)
    {
        return (_evalGlobalStopReason.get() == s);
    }

    /// Test IterStopType
    bool testIf (IterStopType s)
    {
        return (_iterStopReason.get() == s);
    }

    /// Reset all stop reasons to their default STARTED state
    virtual void setStarted();

    /// Get the stop reason that requires termination as a string.
    /**
     If no termination is required, an empty string is returned.
     */
    virtual std::string getStopReasonAsString() const;


    /// Get the global eval stop reason as a string.
    /**
    \return An empty string is in STARTED state, the stop reason otherwise.
     */
    static std::string getEvalGlobalStopReasonAsString();

    /// Get the base stop reason as a string.
    /**
     \return An empty string is in STARTED state, the stop reason otherwise.
     */
    static std::string getBaseStopReasonAsString();

    /// Check if among all stop reasons, one requires a termination.
    /**
     \see StopReason::checkTerminate()

     \return \c true if a termination is required, \c false otherwise.
     */
    virtual bool checkTerminate() const;

    static bool checkBaseTerminate()
    {
        return _baseStopReason.checkTerminate();
    }

    static bool checkEvalGlobalTerminate()
    {
        return _evalGlobalStopReason.checkTerminate();
    }
};


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_ALLSTOPREASONS__
