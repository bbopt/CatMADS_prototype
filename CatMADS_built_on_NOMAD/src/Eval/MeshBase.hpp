/**
 * \file   MeshBase.hpp
 * \brief  Base class for mesh
 * \author Viviane Rochon Montplaisir
 * \date   November 2017
 */

#ifndef __NOMAD_4_5_MESHBASE__
#define __NOMAD_4_5_MESHBASE__

#include <memory>   // for shared_ptr

#include "../Math/ArrayOfDouble.hpp"
#include "../Math/Direction.hpp"
#include "../Math/Point.hpp"
#include "../Param/PbParameters.hpp"
#include "../Util/AllStopReasons.hpp"

#include "../nomad_nsbegin.hpp"

/// The generic class for meshes (discretization of design variables space).
/**
 \note To be implemented by the derived mesh used by an algorithm.

 \note This class encompasses the mesh discretization and the frame on it.

 * In the algorithm, we have delta for mesh size parameter and
  Delta for frame size parameter. To avoid confusion in function calls,
  using deltaMeshSize and DeltaFrameSize in function names.
 * Functions for delta: MeshBase::getdeltaMeshSize, MeshBase::updatedeltaMeshSize
 * Functions for Delta: MeshBase::getDeltaFrameSize, MeshBase::enlargeDeltaFrameSize, MeshBase::refineDeltaFrameSize
 */
class DLL_EVAL_API MeshBase
{
protected:
    const size_t _n;     ///< Dimension
    std::shared_ptr<PbParameters>  _pbParams; ///< The parameters for a mesh are in problem parameters.
    const ArrayOfDouble  _initialMeshSize;  ///< The initial mesh size.
    const ArrayOfDouble  _minMeshSize;   ///< The minimum mesh size (stopping criterion).
    const ArrayOfDouble  _initialFrameSize; ///< The initial frame size.
    const ArrayOfDouble  _minFrameSize; ///< The minimum frame size (stopping criterion).
    const ArrayOfDouble _lowerBound;
    const ArrayOfDouble _upperBound;
    
    /*
     The mesh index is a compact indicator of the level of mesh refinement/coarsening.
     A default stopping criterion on mesh index is available for GMesh. With this criterion, there is no need to set a default min mesh size.
     Warning: The algebraic relation between mesh/frame size and mesh index is not always maintained. When updating mesh/frame size (success/failure), the mesh index is  updated. However, when forcing values of mesh/frame size (set functions), the mesh index is not updated. Also, when forcing the mesh index, the mesh/frame size is not updated.
     */
    ArrayOfDouble    _r; ///< Mesh index per coordinate.
    ArrayOfDouble    _rMin; ///< Lowest mesh index reached per coordinate.
    ArrayOfDouble    _rMax; ///< Highest mesh index reached per coordinate.
    
    
    /* The limit mesh indices are hardcoded (no parameters). This can trigger a stopping criterion (MIN_MESH_INDEX_REACHED) indicative of the cumulative number of mesh refinements regardless of the initial mesh size. There is a symmetric for mesh coarsening.
       It is possible to change those criteria programmatically with
     setLimitMeshIndices function.
   */
    int _limitMinMeshIndex; ///< The min limit of mesh index (smaller index -> finer mesh)
    int _limitMaxMeshIndex; ///< The max limit of mesh index (larger index -> coarser mesh)
    
    bool _isFinest;
    


public:

    /// Constructor From [mesh] parameters
    /**
     \param pbParams    The mesh parameters are taken from problem parameters -- \b IN.
     \param limitMinMeshIndex    The min limit of mesh index  -- \b IN.
     \param limitMaxMeshIndex    The max limit of mesh index  -- \b IN.
     */
    explicit MeshBase(const std::shared_ptr<PbParameters>& pbParams,
                      int limitMinMeshIndex = M_INF_INT ,
                      int limitMaxMeshIndex = P_INF_INT );

    /**
     Virtual destructor needed to avoid compilation warning:
     destructor called on non-final 'GMesh' that has virtual functions
     but non-virtual destructor [-Wdelete-non-virtual-dtor]
     */
    virtual ~MeshBase() {}

    /**
     Pure virtual function. A derived class must implement a clone function to return a pointer to MeshBase
     */
    virtual std::unique_ptr<MeshBase> clone() const = 0;
    
    /*-----------------------------------------------------*/
    /* Get / Set                                           */
    /* Set removed for members because they are all const. */
    /*-----------------------------------------------------*/
    size_t getSize() const { return _n; }

    const ArrayOfDouble& getInitialMeshSize() const { return _initialMeshSize; }
    const ArrayOfDouble& getMinMeshSize() const { return _minMeshSize; }
    const ArrayOfDouble& getInitialFrameSize() const { return _initialFrameSize; }
    const ArrayOfDouble& getMinFrameSize() const { return _minFrameSize; }

    /*------------------*/
    /*   Mesh methods   */
    /*------------------*/

    /// Update mesh size.
    /**
     Update mesh size (small delta) based on frame size (big Delta)
     */
    virtual void updatedeltaMeshSize() = 0;

    /// Enlarge frame size.
    /**
     * Update frame size (big Delta) after a success.

     * The successful direction is used to ensure integrity of the mesh (no variable collapse).

     \param direction          The direction of success of the iteration       -- \b IN.
     \return                   \c true if the mesh has changed, \c false otherwise.
     */
    virtual bool enlargeDeltaFrameSize(const Direction& direction) = 0;

    /// Refine frame size.
    /**
     Update frame size after a failure.
     */
    virtual void refineDeltaFrameSize() = 0;


    /// Check the mesh stopping condition
    virtual void checkMeshForStopping( std::shared_ptr<AllStopReasons> algoStopReason ) const =0;


    /// Access to the ratio of frame size / mesh size parameter rho^k.
    /**
     \param i       Index of the dimension of interest -- \b IN.
     \return        The ratio frame/mesh size rho^k for index i.
     */
    virtual Double getRho(const size_t i) const = 0;

    /// Access to the ratio of frame size / mesh size parameter rho^k.
    /**
     \return The ratio frame/mesh size rho^k.
     */
    virtual ArrayOfDouble getRho() const;

    
    /// Access to the mesh indices.
    /**
     \return        The mesh indices.
     */
    virtual ArrayOfDouble getMeshIndex() const { return _r;}
    
    /// Set the mesh indices.
    /**
     \param r      The mesh indices to set (no update of mesh/frame size -- \b IN.
     */
    virtual void setMeshIndex(const ArrayOfDouble &r  ) { _r=r;}
    
    /// Access to the mesh size parameter delta^k.
    /**
     \param i       Index of the dimension of interest -- \b IN.
     \return        The mesh size parameter delta^k.
     */
    virtual Double getdeltaMeshSize(const size_t i) const = 0;

    /// Access to the mesh size parameter delta^k.
    /**
     \return The mesh size parameter delta^k.
     */
    virtual ArrayOfDouble getdeltaMeshSize() const;

    // Access to the frame size parameter Delta^k.
    /**
     \param i       Index of the dimension of interest -- \b IN.
     \return        The frame size parameter Delta^k.
     */
    virtual Double getDeltaFrameSize(const size_t i) const = 0;

    // Access to the frame size parameter Delta^k.
    /**
     \return The frame size parameter Delta^k.
     */
    virtual ArrayOfDouble getDeltaFrameSize() const;

    // Access to the frame size one shift coarser than the actual frame size.
    /**
     \param i       Index of the dimension of interest -- \b IN.
     \return        The frame size parameter Delta^k.
     */
    virtual Double getDeltaFrameSizeCoarser(const size_t i) const = 0;

    // Access to the frame size one shift coarser than the actual frame size.
    /**
     \return The frame size parameter Delta^k.
     */
    virtual ArrayOfDouble getDeltaFrameSizeCoarser() const;

    /**
     Setting deltaMeshSize and DeltaFrameSize should be done together.
     This is easier and does not seem to be a constraint for
    the general case.
     */
    virtual void setDeltas(const size_t i,
                           const Double &deltaMeshSize,
                           const Double &deltaFrameSize) = 0;

    /**
     Setting deltaMeshSize and DeltaFrameSize should be done together.
     This is easier and does not seem to be a constraint for
     the general case.
     */
    virtual void setDeltas(const ArrayOfDouble &deltaMeshSize,
                           const ArrayOfDouble &deltaFrameSize);

    /// Scale and project the ith component of a vector on the mesh
    /**
     \param i   The vector component number         -- \b IN.
     \param l   The vector component value          -- \b IN.
     \return    The ith component of a vector after mesh scaling and projection.
     */
    virtual Double scaleAndProjectOnMesh(size_t i, const Double &l) const = 0;

    /// Scale and project a direction on the mesh
    /**
     \param dir   The direction to scale and project          -- \b IN.
     \return      The ith component of a vector after mesh scaling and projection.
     */
    virtual ArrayOfDouble scaleAndProjectOnMesh(const Direction &dir) const;

    /// Project the point on the mesh centered on frameCenter. No scaling.
    virtual Point projectOnMesh(const Point& point, const Point& frameCenter) const;

    /// Verify if the point is on the mesh.
    /**
    \return     \c true if the point is on the mesh, false otherwise.
    */
    bool verifyPointIsOnMesh(const Point& point, const Point& frameCenter) const;
    
    /// Return if mesh is finest
    bool isFinest() const {return _isFinest ;}
    
    
    /// Set the limit mesh indices
    /**
    \param limitMinMeshIndex   The limit min mesh index          -- \b IN.
    \param limitMaxMeshIndex   The limit max mesh index          -- \b IN.
    */
    void setLimitMeshIndices(int limitMinMeshIndex, int limitMaxMeshIndex);
    
private:
    /// Helper for constructor.
    void init();

protected:
    /**
     Verify dimension is n. Throw an exception if it is not.
     */
    void verifyDimension(const std::string& arrayName, size_t dim);
};

typedef std::shared_ptr<MeshBase> MeshBasePtr;

///   Display useful values so that a new mesh could be constructed using these values.
DLL_EVAL_API std::ostream& operator<<(std::ostream& os, const MeshBase& mesh);

/// Get the mesh values from stream
DLL_EVAL_API std::istream& operator>>(std::istream& is, MeshBase& mesh);


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_MESHBASE__
