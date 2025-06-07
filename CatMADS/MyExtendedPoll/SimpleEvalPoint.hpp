/**
 \file   SimpleEvalPoint.hpp
 \brief  Evaluation point for simple mads barrier
 \author Christophe Tribes
 \date   April 2024
 \see    SimpleEvalPoint.cpp
 */

#ifndef __NOMAD_4_5_SIMPLEEVALPOINT__
#define __NOMAD_4_5_SIMPLEEVALPOINT__

#include "../../Eval/EvalPoint.hpp"


#include "../../nomad_platform.hpp"
#include "../../nomad_nsbegin.hpp"


/// Class for the representation of a simple form for evaluation point.
/**
 Simple evaluation point for the point coordinates \c x, and the blackbox
 outputs at these coordinates summarized into f(x) and h(x).
*/
class DLL_ALGO_API SimpleEvalPoint : public Point
{
private:

    Double _f, _h;
    
public:

    /*---------------*/
    /* Class Methods */
    /*---------------*/

    /// Constructor #1.
    explicit SimpleEvalPoint()
    : NOMAD::Point() {};

    /// Constructor #2.
    /**
     \param n Number of variables -- \b IN.
     */
    explicit SimpleEvalPoint(size_t n)
    : NOMAD::Point(n) {};

    /// Constructor #3.
    /**
      \param x Coordinates of the eval point -- \b IN.
      */
    explicit SimpleEvalPoint(const Point& x)
    : NOMAD::Point(x) {}; // Let F and H undefined
    

    /// Copy constructor.
    /**
     \param evalPoint The copied object -- \b IN.
     */
    SimpleEvalPoint(const SimpleEvalPoint& evalPoint)
    : NOMAD::Point(evalPoint),
      _f(evalPoint._f),
      _h(evalPoint._h) {};
    

public:

    /// Affectation operator to base class.
    /**
     \param evalPoint The right-hand side object -- \b IN.
     \return           \c *this as the result of the affectation.
     */
    SimpleEvalPoint& operator= (const SimpleEvalPoint& evalPoint);


    /// Destructor.
    virtual ~SimpleEvalPoint() {};
    
    const Double & getF() const { return _f;}
    const Double & getH() const { return _h;}
    
    void setF(const Double & f) {_f = f;}
    void setH(const Double & h) {_h = h;}
    
    bool isDefined() const ;

};

//
///// Display useful values so that a new SimpleEvalPoint could be constructed using these values.
//DLL_ALGO_API std::ostream& operator<<(std::ostream& os,
//                         const SimpleEvalPoint &evalPoint);
//
///// Get these values from stream
//DLL_ALGO_API std::istream& operator>>(std::istream& is, SimpleEvalPoint &evalPoint);
//

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SIMPLEEVALPOINT__
