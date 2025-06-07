/**
 \file   ComparePriority.hpp
 \brief  Compare EvalQueuePoints for sorting
 \author Viviane Rochon Montplaisir
 \date   November 2020
 \see    ComparePriority.cpp
 */

#ifndef __NOMAD_4_5_COMPAREPRIORITY__
#define __NOMAD_4_5_COMPAREPRIORITY__

#include "../Algos/Step.hpp"
#include "../Eval/EvalQueuePoint.hpp"
#include "../Math/Direction.hpp"
#include "../Math/RandomPickup.hpp"

#include "../nomad_nsbegin.hpp"


/// Definition for compare priority method.
/**
 An instance of this class has a comp() method that compares two EvalQueuePoints for ordering.
*/
class ComparePriorityMethod
{
private:
    std::string _name;  ///< Method name, useful for information or debugging

public:
    virtual bool comp(EvalQueuePointPtr& NOMAD_UNUSED(point1), EvalQueuePointPtr& NOMAD_UNUSED(point2)) const
    {
        return false;
    }

    // Called before sorting with all eval queue points.
    virtual void completeTrialPointsInformation(const Step * step, EvalPointSet & trialPoints) {return; }

    void setName(const std::string& name) { _name = name; }
    const std::string& getName() const { return _name; }
};


class BasicComp : public ComparePriorityMethod
{
public:
    /// Constructor
    explicit BasicComp()
    {
        setName("BasicComp");
    }

    bool comp(EvalQueuePointPtr& point1, EvalQueuePointPtr& point2) const override;
};


// Class for comparison using a direction.
class OrderByDirection : public ComparePriorityMethod
{
private:
    /** Vector of directions: One per main thread; one list for feasible and
      * infeasible points. Makes it possible
      * to compare points from different algorithms.
     **/
    std::vector<std::shared_ptr<Direction>> _lastSuccessfulFeasDirs;
    std::vector<std::shared_ptr<Direction>> _lastSuccessfulInfDirs;

    FHComputeType _computeType;

public:
    /// Constructor
    ///
    explicit OrderByDirection(const std::vector<std::shared_ptr<Direction>>& feasDirs,
                              const std::vector<std::shared_ptr<Direction>>& infDirs,
                              const NOMAD::FHComputeType &computeType)
      : _lastSuccessfulFeasDirs(feasDirs),
        _lastSuccessfulInfDirs(infDirs),
        _computeType(computeType)
    {
        setName("OrderByDirection");
    }

    bool comp(EvalQueuePointPtr& point1, EvalQueuePointPtr& point2) const override;
};


// Class for mixing points randomly
class RandomComp : public ComparePriorityMethod
{
private:
    mutable RandomPickup                _randomPickup;
    mutable std::map<size_t, size_t>    _tagToRank;

public:
    /// Constructor
    explicit RandomComp(const size_t n);

    bool comp(EvalQueuePointPtr& point1, EvalQueuePointPtr& point2) const override;
};


// Class for comparison using static surrogate or model evaluations.
class OrderByEval : public ComparePriorityMethod
{
private:

    FHComputeType _computeType;
public:
    /// Constructor
    explicit OrderByEval(const NOMAD::FHComputeType &computeType):
    _computeType(computeType)
    {
        if (_computeType.evalType == NOMAD::EvalType::SURROGATE)
        {
            setName("OrderBySurrogate");
        }
        else if (_computeType.evalType == NOMAD::EvalType::MODEL)
        {
            setName("OrderByModel");
        }
        else
        {
            throw NOMAD::Exception(__FILE__, __LINE__, "OrderByEval: Eval Type " + evalTypeToString(_computeType.evalType) + " cannot be used for ordering points") ;
        }
    }

    bool comp(EvalQueuePointPtr& point1, EvalQueuePointPtr& point2) const override;
};

/// Class to compare priority of two EvalQueuePoint
class ComparePriority
{
private:
    std::shared_ptr<ComparePriorityMethod>  _compMethod; ///< Comparison method to be used to sort eval queue points

public:
    /// Constructor
    explicit ComparePriority(const std::shared_ptr<ComparePriorityMethod>& compMethod)
      : _compMethod(compMethod)
    {}

    ///  Function call operator
    /**
     \param p1  First eval queue point -- \b IN.
     \param p2  Second eval queue point -- \b IN.
     \return    \c true if p1 has a lower priority than p2. \c false otherwise.
     */
    bool operator()(EvalQueuePointPtr& p1, EvalQueuePointPtr& p2);
};


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_COMPAREPRIORITY__
