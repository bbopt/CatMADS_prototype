#ifndef __NOMAD_4_5_SEARCHMETHODBASE__
#define __NOMAD_4_5_SEARCHMETHODBASE__

#include "../../Algos/IterationUtils.hpp"
#include "../../Algos/Step.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for generic search method of MADS. Run by Search.
/**
 Pure virtual class from which SearchMethodSimple and SearchMethodAlgo derive.
 */
class SearchMethodBase: public Step, public IterationUtils
{
private:

    bool _enabled; ///< Should this search method be used? Modified by parameters.
    
    std::string _comment; ///<  Comment shown when a search method is used

protected:
    
    ArrayOfDouble _lb, _ub;

    // Dynamic search is only for simple search methods.
    bool _dynamicSearch = false;
    bool _dynamicSearchEnabled = true; ///< Should this simple search method generate trial points? Maybe modified dynamically by method (based on success, or alternating rule, ...). Update is done only if dynamicSearch is true.
    std::list<NOMAD::SuccessType> _allSuccessTypes; ///< Used only when dynamicSearch is active. Each search will get a success type. It is UNDEFINED if not dynamic enabled.
    

    
public:
    /// Constructor
    /**
     /param parentStep      The parent of this search step -- \b IN.
     */
    explicit SearchMethodBase( const Step* parentStep )
      : Step( parentStep ),
        IterationUtils ( parentStep ),
        _enabled(true),
        _comment("")
    {
        init();
    }

    bool isEnabled() const { return _enabled; }
    void setEnabled(const bool enabled) { _enabled = enabled; }

    const std::string& getComment() const { return _comment; }
    bool hasComment() const { return (!_comment.empty()); }
    void setComment(const std::string& comment) { _comment = comment; }

    /// Reset the parent step. This is used when search methods are created outside of Search.
    void setParentStep(const Step* parentStep)
    {
        IterationUtils::_parentStep = parentStep;
        Step::_parentStep = parentStep;
        updateMegaIterAncestor();
    }
    
    /**
     - Pure virtual function.
     - The implementation of startImp function in the derived class generates trial points and do some reset (in SearchMethodSimple) OR just reset (stats and success) (in SearchMethodAlgo).
     */
    virtual void startImp() override =0 ;

    /**
     - Pure virtual function.
     - The implementation of runImp function in the derived class evaluates the trial points (in SearchMethodSimple) OR launches an algo (in SearchMethodAlgo).
     */
    virtual bool runImp() override = 0 ;

    /// Implementation of endImp (virtual)
    /**
        Call to the postProcessing function to update the Barrier
    */
    virtual void endImp() override ;

    /// The name of the search method
    /**
        For search methods the name is completed with the number of calls
     */
    std::string getName() const override
    {
        return NOMAD::stepTypeToString(_stepType) + " #" + std::to_string(_trialPointStats.getNbCalls());
    }
    
    size_t getNbTrialPointsGenerated(const NOMAD::EvalType evalType) const
    {
        return _trialPointStats.getNbTrialPointsGenerated(evalType);
    }
    
protected:
    void init();
    
private:
    
    /**
     Base implementation to generate trial points. The trial points are snapped to bounds and projected on mesh.
     Implementation for final derived search methods is in generateTrialPointsFinal.
     */
    void generateTrialPointsImp() override ;
    
    /**
     - Pure virtual function.
     - See derived classes (SearchMethodSimple, SearchMethodAlgo) for implementations.
     */
    virtual void generateTrialPointsFinal() = 0;
    
    /// Implementation to increment the nb of calls counter
    virtual void incrementCounters() override { _trialPointStats.incrementNbCalls() ;}
    
};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SEARCHMETHODBASE__

