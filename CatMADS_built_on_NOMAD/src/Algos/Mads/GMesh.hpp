/**
 * \file   GMesh.hpp
 * \brief  Class for Granular Mesh
 * \author Viviane Rochon Montplaisir
 * \date   November 2017
 */

#ifndef __NOMAD_4_5_GMESH__
#define __NOMAD_4_5_GMESH__

#include "../../Eval/MeshBase.hpp"
#include "../../Param/RunParameters.hpp"
#include "../../nomad_nsbegin.hpp"


/// Class for the granular mesh of Mads.
/**
 This class derives from MeshBase.
 The class manages the mesh size (delta) and the frame size (Delta) for the discretization of the variable space used by Mads. Each variable has its own mesh and frame sizes which allows to increase or decrease the anisotropy of the mesh, that is changing the "cell" aspect ratios. The frame size (and mesh size) for each variable can be enlarged or decreased (see GMesh::refineDeltaFrameSize and GMesh::enlargeDeltaFrameSize). A given point can be projected on the the mesh using GMesh::scaleAndProjectOnMesh. \n

 The frame size for each variable is parameterized with two or three attributes: GMesh::_frameSizeExp (exponent), GMesh::_frameSizeMant (mantissa) and GMesh::_granularity (Delta = gran * a * 10^b with b an integer). The last attribute is for variable having a specified minimal granularity (for example, integers have a minimal granularity of 1). The mesh size is delta = 10^(b-|b-b_0|) for variables without granularity and delta = granularity * max(1,delta) for variables with granularity.

 */
class GMesh: public MeshBase
{

    ArrayOfDouble        _initFrameSizeExp;  ///< The initial frame size exponent.
    ArrayOfDouble        _frameSizeMant;  ///< The current frame size mantissa.
    ArrayOfDouble        _frameSizeExp;  ///< The current frame size exponent.
    
    ArrayOfDouble        _finestMeshSize; ///< The current finest mesh size
    
    const ArrayOfDouble  _granularity;  ///< The fixed granularity of the mesh
    bool                 _enforceSanityChecks;   ///< Should we enforce sanity checks?
    
    bool                 _allGranular;  ///< _all_granular is true if all variables are granular; fixed variables are not considered.
    
    Double               _anisotropyFactor;  ///<   Control the development of anisotropy of the mesh (if anisotropicMesh is true).
    bool                 _anisotropicMesh;  ///<    Flag to enable or not anisotropic mesh
    
    size_t               _refineFreq;        ///<  Control the frequency of actual mesh refinement
    
    size_t               _refineCount;       ///< Count mesh refinement call
    
public:

    /// Constructor
    /**
     \param parameters  The problem parameters attributes control the mesh mechanics -- \b IN.
     \param runParams The parameters to set mesh anisotropy factor --  \b IN.
     */
    explicit GMesh(std::shared_ptr<PbParameters> parameters, std::shared_ptr<RunParameters> runParams)
      : MeshBase(parameters,
                 GMESH_LIMIT_MIN_MESH_INDEX, /* Limit Min mesh index */
                 -GMESH_LIMIT_MIN_MESH_INDEX), /* Limit Max mesh index */
                _initFrameSizeExp(ArrayOfDouble()),
                _frameSizeMant(ArrayOfDouble()),
                _frameSizeExp(ArrayOfDouble()),
                _finestMeshSize(ArrayOfDouble()),
                _granularity(parameters->getAttributeValue<ArrayOfDouble>("GRANULARITY")),
                _enforceSanityChecks(true),
                _allGranular(true),
                _anisotropyFactor(runParams->getAttributeValue<NOMAD::Double>("ANISOTROPY_FACTOR")),
                _anisotropicMesh(runParams->getAttributeValue<bool>("ANISOTROPIC_MESH")),
                _refineFreq(runParams->getAttributeValue<size_t>("ORTHO_MESH_REFINE_FREQ")),
                _refineCount(0)
    {
        init();
    }

    // Clone a GMesh and return a pointer to MeshBase
    std::unique_ptr<MeshBase> clone() const override {
      return std::make_unique<GMesh>(*this);
    }
    
    /*-----------*/
    /* Get / Set */
    /*-----------*/
    const ArrayOfDouble& getInitFrameSizeExp() const { return _initFrameSizeExp; }
    const ArrayOfDouble& getFrameSizeMant() const { return _frameSizeMant; }
    const ArrayOfDouble& getFrameSizeExp() const { return _frameSizeExp; }
    const ArrayOfDouble& getGranularity() const { return _granularity; }

    /**
     Performing enforced sanity checks consists in checking GMesh::checkFrameSizeIntegrity and GMesh::checkDeltasGranularity. It is requested when delta/granularity==1.
     */
    void setEnforceSanityChecks(const bool doubleCheck) { _enforceSanityChecks = doubleCheck; }

    /*----------------------*/
    /* Other Class Methods */
    /*----------------------*/


    void checkMeshForStopping( std::shared_ptr<AllStopReasons> algoStopReason ) const override;

    void updatedeltaMeshSize() override;

    /**
     \copydoc MeshBase::enlargeDeltaFrameSize
     \note This implementation relies on GMesh::_frameSizeExp, GMesh::_frameSizeMant and GMesh::_granularity.
     */
    bool enlargeDeltaFrameSize(const Direction& direction) override;

    /**
     \copydoc MeshBase::refineDeltaFrameSize
     \note This implementation relies on GMesh::_frameSizeExp and GMesh::_frameSizeMant. Frame size is updated and mesh size can be impacted too if _frameSizeExp is decreased. Upon failure, the frame sizes for all dimensions are refined in the same way (no need for direction and anisotropy factor).
     */
    void refineDeltaFrameSize() override;

private:

    /// Helper for refineDeltaFrameSize().
    /**
     \param frameSizeMant    The frame size mantissa to update -- \b IN/OUT.
     \param frameSizeExp     The frame size exponent to update -- \b IN/OUT.
     \param granularity      The granularity of the mesh -- \b IN.
     */
    void refineDeltaFrameSize(Double &frameSizeMant,
                              Double &frameSizeExp,
                              const Double &granularity) const;
public:
    Double getRho(const size_t i) const override;
    ArrayOfDouble getRho() const override { return MeshBase::getRho(); }

    ArrayOfDouble getdeltaMeshSize() const override;
    Double getdeltaMeshSize(const size_t i) const override;
    
private:

    /// Helper function
    Double getdeltaMeshSize(const Double& frameSizeExp,
                            const Double& initFrameSizeExp,
                            const Double& granularity) const;
public:

    //
    // The documentation of overriden function is provided in the base class.
    //

    ArrayOfDouble getDeltaFrameSize() const override;
    Double getDeltaFrameSize(const size_t i) const override;
    ArrayOfDouble getDeltaFrameSizeCoarser() const override;
    Double getDeltaFrameSizeCoarser(const size_t i) const override;

private:
    /// Helper function
    Double getDeltaFrameSize(const Double& granularity, const Double& frameSizeMant, const Double& frameSizeExp) const;

public:

    void setDeltas(const ArrayOfDouble &deltaMeshSize,
                   const ArrayOfDouble &deltaFrameSize) override;

    void setDeltas(const size_t i,
                   const Double &deltaMeshSize,
                   const Double &deltaFrameSize) override;

private:
    /// helper function to verify values are correct.
    void checkDeltasGranularity(const size_t i,
                                const Double &deltaMeshSize,
                                const Double &deltaFrameSize) const;

    /// helper function to verify values are correct.
    void checkFrameSizeIntegrity(const Double &frameSizeExp,
                                const Double &frameSizeMant) const;

    /// helper function to verify values are correct.
    void checkSetDeltas(const size_t i,
                        const Double &deltaMeshSize,
                        const Double &deltaFrameSize) const;

public:

    // Add some documentation in addition to the base class
    /**
     \copydoc MeshBase::scaleAndProjectOnMesh
     \note This implementation relies on GMesh::_frameSizeExp and GMesh::_frameSizeMant.
     */
    Double scaleAndProjectOnMesh(size_t i, const Double &l) const override;

    ArrayOfDouble scaleAndProjectOnMesh(const Direction &dir) const override;

    /**
     * Project the point on the mesh centered on frameCenter. No scaling.
     */
    Point projectOnMesh(const Point& point, const Point& frameCenter) const override;


private:
    /// Helper for constructor.
    void init();

    /// Initialization of granular frame size mantissa and exponent
    /**
     \param contInitFrameSize        continuous initial frame size   -- \b IN.
    */
    void initFrameSizeGranular(const ArrayOfDouble &contInitFrameSize);

    /// Round frame size exponent to an int.
    /**
     \param exp     The frame size exponent -- \b IN.
     \return        The frame size exponent as an int
     */
    int roundFrameSizeExp(const Double &exp);

    /// Round frame size mantissa to an int.
    /**
     \param mant     The frame size mantissa -- \b IN.
     \return         The frame size mantissa as an int
     */
    int roundFrameSizeMant(const Double &mant);

    /// Helper for enlargeDeltaFrameSize and getDeltaFrameSizeCoarser.
    void getLargerMantExp(Double &frameSizeMant, Double &frameSizeExp) const;
};


#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_GMESH__
