/**
 \file   BBOutput.hpp
 \brief  Output of a Blackbox evaluation
 \author Viviane Rochon Montplaisir
 \date   January 2018
 \see    BBOutput.cpp
 */


#ifndef __NOMAD_4_5_BB_OUTPUT__
#define __NOMAD_4_5_BB_OUTPUT__

#include "../Type/BBOutputType.hpp"
#include "../Math/ArrayOfDouble.hpp"

#include "../nomad_nsbegin.hpp"


/// Class for the representation of the output of a blackbox evaluation.
/**
 *
 * Manage output from blackbox:
 *  - Raw output (string)
 *  - Is eval ok. This is a boolean indicating that there were no problem during evaluation.
 *  - Scaling (future work)
 */
class DLL_EVAL_API BBOutput {
public:
    static const std::string bboStart; ///< Static variable used for field delimitation.
    static const std::string bboEnd; ///< Static variable used for field delimitation.


private:
    std::string             _rawBBO;    ///< Actual output string
    ArrayOfDouble           _BBO;       ///< Actual numerical values
    bool                    _evalOk;    ///< Flag for evaluation

public:

    /*---------------*/
    /* Class Methods */
    /*---------------*/

    /// Constructor
    /**
     Usually if we have a rawBBO we can assume that the evaluation went OK.
     \param rawBBO  The outputs of the blackbox as a string -- \b IN.
     \param evalOk  The eval ok flag -- \b IN.
     */
    explicit BBOutput(std::string rawBBO, const bool evalOk = true);
    
    /// Constructor #2
    /**
     Usually if we have an array of double well defined for BBO, the evaluation went well.
     \param bbo  The outputs of the blackbox as a array of double -- \b IN.
     */
    explicit BBOutput(const ArrayOfDouble &bbo);

    /*---------*/
    /* Get/Set */
    /*---------*/

    /// Get the first objective from raw blackbox evaluation
    /**
     \param bbOutputType    The list of blackbox output types -- \b IN.
     \return                The objective value (single objective).
     */
    Double getObjective(const BBOutputTypeList &bbOutputType) const;

    /// Get the objectives from raw blackbox evaluation
    /**
     \param bbOutputType    The list of blackbox output types -- \b IN
     \return                The objectives as an array of values.
     */
    ArrayOfDouble getObjectives(const BBOutputTypeList &bbOutputType) const;

    /// Get the constraints from raw blackbox evaluation
    /**
     \param bbOutputType    The list of blackbox output types -- \b IN.
     \return                The constraints as an array of values.
     */
    ArrayOfDouble getConstraints(const BBOutputTypeList &bbOutputType) const;
    

    /// Get the user extra outputs from raw blackbox evaluation
    /**
     \param bbOutputType    The list of blackbox output types -- \b IN.
     \return                The extra outputs as a ArrayOfDouble
     */
    ArrayOfDouble getExtraOutputs(const BBOutputTypeList &bbOutputType) const;

    /// Set each blackbox output separately from a string.
    /**
     \param bbOutputString    The string returned by blackbox evaluation -- \b IN.
     \param evalOk            The evaluation status -- \b IN.
     */
    void setBBO(const std::string &bbOutputString, const bool evalOk = true);

    /// Get if this evaluation proceeded properly
    /**
     \return \c True if the evaluation ended normally; \c False if there was an error.
     */
    bool getEvalOk() const { return _evalOk; }

    /// Does this evaluation count?
    /**
     \return \c True if it does
     */
    bool getCountEval(const BBOutputTypeList &bbOutputType) const;

    /// Get the raw blackbox outputs
    /**
     \return    A single string containing the raw blackbox outputs.
     */
    const std::string& getBBO() const { return _rawBBO; }

    /// Test if raw blackbox outputs for functions (OBJ, PB, EB) is complete
    /**
     \return    .
     */
    bool isComplete(const BBOutputTypeList &bbOutputType) const;

    /// Get the blackbox outputs separately.
    /**
     \return    An array of double of the blackbox outputs.
     */
    const ArrayOfDouble & getBBOAsArrayOfDouble() const;

    /// Display
    void display (std::ostream & out) const;

    /**
     Verify if the BBOutputList and the raw BBO have consistent size.
     A warning is displayed if this is not the case.
     \param bbOutputType    The list of blackbox output type -- \b IN.
     \return                \c true if the sizes match
     */
    bool checkSizeMatch(const BBOutputTypeList &bbOutputType) const;

};


/// Display, using field delimitors BBOutput::bboStart and BBOutput::bboEnd.
DLL_EVAL_API std::ostream& operator<<(std::ostream& os, const BBOutput &bbo);

/// Read bbo that has been written by BBOutput display operator.
DLL_EVAL_API std::istream& operator>>(std::istream& is, BBOutput &bbo);


#include "../nomad_nsend.hpp"
#endif // __NOMAD_4_5_BB_OUTPUT__
