#include "../Eval/MeshBase.hpp"
#include "../Type/BBInputType.hpp"

NOMAD::MeshBase::MeshBase(const std::shared_ptr<NOMAD::PbParameters>& pbParams, int limitMinMeshIndex, int limitMaxMeshIndex)
      : _n(pbParams->getAttributeValue<size_t>("DIMENSION")),
        _pbParams(pbParams),
        _initialMeshSize(pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("INITIAL_MESH_SIZE")),
        _minMeshSize(pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("MIN_MESH_SIZE")),
        _initialFrameSize(pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("INITIAL_FRAME_SIZE")),
        _minFrameSize(pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("MIN_FRAME_SIZE")),
        _lowerBound(pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND")),
        _upperBound(pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND")),
        _isFinest(true),
        _r(NOMAD::ArrayOfDouble(_n,0)),
        _rMin(NOMAD::ArrayOfDouble(_n,0)),
        _rMax(NOMAD::ArrayOfDouble(_n,0)),
        _limitMinMeshIndex(limitMinMeshIndex),
        _limitMaxMeshIndex(limitMaxMeshIndex)
{
    init();
}


void NOMAD::MeshBase::init()
{
    if ( _pbParams->toBeChecked() )
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Parameters::checkAndComply() needs to be called before constructing a mesh.");
    }
}

NOMAD::ArrayOfDouble NOMAD::MeshBase::getRho() const
{
    NOMAD::ArrayOfDouble rho(_n);
    for (size_t i = 0; i < _n; i++)
    {
        rho[i] = getRho(i);
    }
    return rho;
}


NOMAD::ArrayOfDouble NOMAD::MeshBase::getdeltaMeshSize() const
{
    NOMAD::ArrayOfDouble delta(_n);
    for (size_t i = 0; i < _n; i++)
    {
        delta[i] = getdeltaMeshSize(i);
    }
    return delta;
}


NOMAD::ArrayOfDouble NOMAD::MeshBase::getDeltaFrameSize() const
{
    NOMAD::ArrayOfDouble Delta(_n);
    for (size_t i = 0; i < _n; i++)
    {
        Delta[i] = getDeltaFrameSize(i);
    }
    return Delta;
}


NOMAD::ArrayOfDouble NOMAD::MeshBase::getDeltaFrameSizeCoarser() const
{
    NOMAD::ArrayOfDouble Delta(_n);
    for (size_t i = 0; i < _n; i++)
    {
        Delta[i] = getDeltaFrameSizeCoarser(i);
    }
    return Delta;
}


void NOMAD::MeshBase::setDeltas(const NOMAD::ArrayOfDouble &deltaMeshSize,
                                const NOMAD::ArrayOfDouble &deltaFrameSize)
{
    for (size_t i = 0; i < _n; i++)
    {
        setDeltas(i, deltaMeshSize[i], deltaFrameSize[i]);
    }
}


/*-----------------------------------------------------------*/
/*              scale and project on the mesh                */
/*-----------------------------------------------------------*/
NOMAD::Double NOMAD::MeshBase::scaleAndProjectOnMesh(size_t i, const NOMAD::Double &l) const
{
    // Not defined for MeshBase.
    throw NOMAD::Exception(__FILE__, __LINE__, "scaleAndProjectOnMesh() not defined for MeshBase.");
}


NOMAD::ArrayOfDouble NOMAD::MeshBase::scaleAndProjectOnMesh(
    const NOMAD::Direction &dir) const
{
    // Not defined for MeshBase.
    throw NOMAD::Exception(__FILE__, __LINE__, "scaleAndProjectOnMesh() not defined for MeshBase.");
}


NOMAD::Point NOMAD::MeshBase::projectOnMesh(const NOMAD::Point& point,
                                            const NOMAD::Point& frameCenter) const
{
    // Not defined for MeshBase.
    throw NOMAD::Exception(__FILE__, __LINE__, "projectOnMesh() not defined for MeshBase.");
}


bool NOMAD::MeshBase::verifyPointIsOnMesh(const NOMAD::Point& point, const NOMAD::Point& center) const
{
    bool isOnMesh = true;

    for (size_t i = 0; i < point.size(); i++)
    {
        NOMAD::Double pointRebaseI = point[i];
        const NOMAD::Double& centerI = center[i];
        NOMAD::Double deltaI = getdeltaMeshSize(i);

        if (   (_lowerBound[i].isDefined() && _lowerBound[i] == pointRebaseI)
            || (_upperBound[i].isDefined() && _upperBound[i] == pointRebaseI))
        {
            isOnMesh = true;
        }
        else
        {
            if (!centerI.isMultipleOf(deltaI))
            {
                // Rebase point on the mesh centered on center Point
                pointRebaseI -= centerI;
            }
            if (!pointRebaseI.isMultipleOf(deltaI))
            {
                isOnMesh = false;
                break;
            }
        }
    }

    return isOnMesh;
}


std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::MeshBase& mesh)
{
    os << "DELTA_MESH_SIZE " << mesh.getdeltaMeshSize() << std::endl;
    os << "DELTA_FRAME_SIZE " << mesh.getDeltaFrameSize() << std::endl;

    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::MeshBase& mesh)
{
    size_t n = mesh.getSize();

    // Read line by line
    std::string name;

    NOMAD::ArrayOfDouble deltaMeshSize(n), deltaFrameSize(n);
    while (is >> name && is.good() && !is.eof())
    {
        if ("DELTA_MESH_SIZE" == name)
        {
            is >> deltaMeshSize;
        }
        else if ("DELTA_FRAME_SIZE" == name)
        {
            is >> deltaFrameSize;
        }
        else
        {
            // Put back name to istream. Maybe there is a simpler way.
            for (unsigned i = 0; i < name.size(); i++)
            {
                is.unget();
            }
            break;
        }
    }

    mesh.setDeltas(deltaMeshSize, deltaFrameSize);

    return is;

}


void NOMAD::MeshBase::setLimitMeshIndices(int limitMinMeshIndex, int limitMaxMeshIndex)
{
    _limitMaxMeshIndex = limitMaxMeshIndex;
    _limitMinMeshIndex = limitMinMeshIndex;
    
    // TODO Update the mesh size according the mesh indices
    // This is not trivial. Depends on the type of mesh.
    
    
    
    // Not yet defined for MeshBase.
    throw NOMAD::Exception(__FILE__, __LINE__, "setLimitMeshIndices not yet implemented.");
    
}
