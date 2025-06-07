/**
 \file   DiscoMadsBarrier.cpp
 \brief  The DiscoMads algorithm barrier
 \author Solene Kojtych
 \see    DiscoMadsBarrier.hpp
 */
#ifndef __NOMAD_4_5_DISCOMADSBARRIER
#define __NOMAD_4_5_DISCOMADSBARRIER


#include "../../Eval/ProgressiveBarrier.hpp"
#include "../../Eval/EvalPoint.hpp"
#include "../../Type/BBInputType.hpp"
#include "../../nomad_nsbegin.hpp"
#include "Exception.hpp"

/// Class for DiscoMadsBarrier
class DiscoMadsBarrier : public ProgressiveBarrier
{
private:
    NOMAD::Double  _exclusionRadius;     // used to detect points close to revealing points, for which we should update the revealed constraint (RPB constraint)

public:
    /// Constructor
    DiscoMadsBarrier(const Double& hMax = INF,
            const Point& fixedVariable = Point(),
            EvalType evalType = EvalType::BB,
            FHComputeTypeS computeType = defaultFHComputeTypeS,
            const std::vector<EvalPoint>& evalPointList = std::vector<EvalPoint>(),
            bool barrierInitializedFromCache= true,
            const Double& exclusionRadius=1.0)
      : ProgressiveBarrier(hMax,
            fixedVariable,
            evalType,
            computeType,
            evalPointList,
            barrierInitializedFromCache)
    {
      _exclusionRadius = exclusionRadius;
        
        // Just in case
        if(_computeType.evalType == NOMAD::EvalType::MODEL)
        {
            throw NOMAD::Exception(__FILE__, __LINE__, "DiscoMAdsBarrier:: should not be used on quadratic model optimization.");
        }
    }
    
    DiscoMadsBarrier(const DiscoMadsBarrier & b): ProgressiveBarrier(b)
    {
        // TODO check if some attribute must be copied
    }

    std::shared_ptr<BarrierBase> clone() const override
    {
      return std::make_shared<DiscoMadsBarrier>(*this);
    }


    /// Update xFeas and xInf according to given points. // H1 corriger commentaire
    /* \param evalPointList vector of EvalPoints  -- \b IN.
     * \param keepAllPoints TODO see if it is needed \b IN.
     * \return true if the barrier feasible and/or infeasible incumbents are changed, false otherwise
     * \note Input EvalPoints are already in subproblem dimension
     */
    virtual bool updateWithPoints(const std::vector<EvalPoint>& evalPointList,
                                  const bool keepAllPoints = false,
                                  const bool updateInfeasibleIncumbentAndHmax = false ) override;

private:
     /** Helper for updateWithPoints
        * return true if dist(x1,x2) < exclusion radius
     */
    bool proximityTest(const NOMAD::Point & x1, const NOMAD::EvalPoint & x2);

    /** Helper for updateWithPoints
        *  Get the h-value of the k-th point of a list of points sorted with increasing h values
     */
    NOMAD::Double getKiemeHvalue(const std::vector<EvalPointPtr>& evalPointList, const size_t k) const;

     /** Helper for updateWithPoints
        * Get non dominated points with h<= hmax from barrier infeasible points and return number of points
     \param evalPointList vector of non dominated infeasible points with h<= hmax  -- \b OUT.
     \return                The number of eval points found.
     */
     size_t getNonDominatedInfPoints(std::vector<EvalPointPtr>& evalPointList);
};


#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DISCOMADSBARRIER__
