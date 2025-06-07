/**
 \file   Direction.hpp
 \brief  Direction: Represent geometrical vectors
 \author Viviane Rochon Montplaisir
 \date   October 2017
 \see    Direction.cpp
 */

#ifndef __NOMAD_4_5_DIRECTION__
#define __NOMAD_4_5_DIRECTION__

#include <numeric>
#include "../Math/ArrayOfDouble.hpp"
#include "../Math/SimpleRNG.hpp"

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

/// Type of norm
enum class NormType
{
    L1,   ///< Norm L1
    L2,   ///< Norm L2
    LINF  ///< Norm LInf
};


/// Class for the representation of a direction.
/**
 A direction is defined by its size and its coordinates from ArrayOfDouble. In addition, it provides functions to
 calculate different norms, dot product and cosine of the angle between two directions.
*/
class DLL_UTIL_API Direction : public ArrayOfDouble
{
public:
    /*-------------*/
    /* Constructor */
    /*-------------*/
    /**
     \param n   Dimension of the direction -- \b IN.
     \param val Value for all coordinates -- \b IN.
     */
    explicit Direction(const size_t n = 0, const Double &val = Double())
      : ArrayOfDouble(n, val)
    {}

    /// Copy constructors.
    Direction(const Direction& dir)
      : ArrayOfDouble(dir)
    {}

    /// Copy constructors.
    /**
     \param pt The copied object -- \b IN.
     */
    Direction(const ArrayOfDouble& pt)
      : ArrayOfDouble(pt)
    {}

    /// Assignment operator
    /**
     \param dir The object to assign -- \b IN.
     */
    Direction& operator=(const Direction &dir);

    /// Destructor.
    virtual ~Direction() {}

    /*-----------*/
    /* Operators */
    /*-----------*/

    /// Operator \c +=.
    /**
     Vector operation dir + dir1.
     \param dir1  Direction to add -- \b IN.
     \return      Reference to \c *this after adding dir1.
     */
    const Direction& operator+=(const Direction& dir1);

    /// Operator \c -=.
    /**
     Vector operation dir - dir1.
     \param dir1  Direction to subtract -- \b IN.
     \return      Reference to \c *this after subtracting dir1.
     */
    const Direction& operator-=(const Direction& dir1);

    /*----------*/
    /*   Norm   */
    /*----------*/
    /// Squared L2 norm
    Double squaredL2Norm() const;

    /** Compute norm - L1, L2 or infinite norm.
     * Default is L2.
     */
    Double norm(NormType normType = NormType::L2) const;

    /// Infinite norm
    Double infiniteNorm() const;

    /*-------------*/
    /* Dot product */
    /*-------------*/

    /// Dot product
    /**
     \param dir1    First \c Direction -- \b IN.
     \param dir2    Second \c Direction -- \b IN.
     \return        The dot product of dir1 and dir2.
     */
    static Double dotProduct(const Direction& dir1,
                             const Direction& dir2);
    /// Cosine of the angle between two vectors
    /**
     \param dir1    First \c Direction -- \b IN.
     \param dir2    Second \c Direction -- \b IN.
     \return        The cosine of the angle between the two \c Directions.
     */
    static Double cos(const Direction& dir1, const Direction& dir2);

    static Double angle(const Direction& dir1, const Direction& dir2);    // From NOMAD 3

    /// Compute a random direction on a unit N-Sphere
    /**
     \param randomDir of the desired dimension -- \b IN/OUT
     */
    static void computeDirOnUnitSphere(Direction &randomDir, const std::shared_ptr<SimpleRNG> & rng = nullptr);

    /// Compute a random direction in a unit N-Sphere
    /**
     \param randomDir of the desired dimension -- \b IN/OUT
     */
    static void computeDirInUnitSphere(Direction &randomDir, const std::shared_ptr<SimpleRNG> & rng = nullptr);
    
    /// Householder transformation
    /** Householder transformation to generate n directions from a given direction. Also computes H[i+n] = -H[i] (completion to 2n directions).
    \param dir given direction -- \b IN.
    \param completeTo2n completion to 2n directions -- \b IN.
    \param H matrix for Householder transformation -- \b OUT.
    */
    static void householder(const Direction &dir,
                            bool completeTo2n,
                            Direction ** H);

};

/// Inverse operator.
DLL_UTIL_API Direction operator-(const Direction &dir);

/// Display of \c Direction
/**
 \param d    Object to be displayed -- \b IN.
 \param out  Reference to stream -- \b IN.
 \return     Reference to stream.
 */
DLL_UTIL_API std::ostream& operator<< (std::ostream& out, const Direction& d);


#include "../nomad_nsend.hpp"
#endif // __NOMAD_4_5_DIRECTION__
