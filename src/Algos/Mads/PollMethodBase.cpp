#include "../../Algos/EvcInterface.hpp"
#include "../../Algos/Mads/PollMethodBase.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Type/EvalSortType.hpp"


void NOMAD::PollMethodBase::init()
{
    // A poll method must have a parent
    verifyParentNotNull();

    if (nullptr != _pbParams)
    {
        _n = _pbParams->getAttributeValue<size_t>("DIMENSION");
        _lb = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND");
        _ub = _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND");
    }
}

void NOMAD::PollMethodBase::generateTrialPointsImp()
{

    OUTPUT_INFO_START
    AddOutputInfo("Generate points for " + getName(), true, false);
    OUTPUT_INFO_END

    generateTrialPointsInternal(false);

    OUTPUT_INFO_START
    AddOutputInfo("Generated " + std::to_string(getTrialPointsCount()) + " points");
    AddOutputInfo("Generate points for " + getName(), false, true);
    OUTPUT_INFO_END
}

void NOMAD::PollMethodBase::generate2NDirections(std::list<NOMAD::Direction> &directions, size_t n) const
{
    NOMAD::Direction dirUnit(n, 0.0);
    NOMAD::Direction::computeDirOnUnitSphere(dirUnit, _rng);

    OUTPUT_DEBUG_START
    AddOutputDebug("Unit sphere direction: " + dirUnit.display());
    NOMAD::OutputQueue::Flush();
    OUTPUT_DEBUG_END

    // Householder Matrix
    auto H = new NOMAD::Direction*[2*n];

    // Ordering D_k alternates Hk and -Hk instead of [H_k -H_k]
    for (size_t i = 0; i < n; ++i)
    {
        directions.emplace_back(n, 0.0);
        H[i]   = &(directions.back());
        directions.emplace_back(n, 0.0);
        H[i+n] = &(directions.back());
    }
    // Householder transformations on the 2n directions on a unit n-sphere
    NOMAD::Direction::householder(dirUnit, true, H);
    delete [] H;
}

void NOMAD::PollMethodBase::generateTrialPointsSecondPassImp()
{
    generateTrialPointsInternal(true);
}


// If isSecondPass is true, _trialPoints contains evaluated trial points,
// and the second pass trial points need to be generated.
void NOMAD::PollMethodBase::generateTrialPointsInternal(const bool isSecondPass)
{

    std::list<NOMAD::Direction> directionsFullSpace = generateFullSpaceScaledDirections(isSecondPass);

    OUTPUT_INFO_START
    std::string s = "Generate ";
    s+= (isSecondPass) ? "second pass trial point(s)" : ((_isFreePoll) ? "user free poll trial point(s)" :" first pass trial points");
    s += " for " + getName();
    AddOutputInfo(s, true, false);
    OUTPUT_INFO_END

    OUTPUT_DEBUG_START
    for (const auto& dir : directionsFullSpace)
    {
        AddOutputDebug("Scaled and mesh projected poll direction: " + dir.display());
    }
    NOMAD::OutputQueue::Flush();
    OUTPUT_DEBUG_END

    // We need a frame center to start with.
    if (!_frameCenter->ArrayOfDouble::isDefined() || _frameCenter->size() != _n)
    {
        std::string err = "Invalid frame center: " + _frameCenter->display();
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }

    OUTPUT_DEBUG_START
    AddOutputDebug("Frame center: " + _frameCenter->display());
    OUTPUT_DEBUG_END

    for (auto& it : directionsFullSpace)
    {
        NOMAD::Point pt(_n);

        // pt = frame center + direction
        for (size_t i = 0 ; i < _n ; ++i )
        {
            pt[i] = (*_frameCenter)[i] + it[i];
        }

        auto evalPoint = NOMAD::EvalPoint(pt);
        evalPoint.setPointFrom(_frameCenter, NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this));

        // Snap the points and the corresponding direction to the bounds
        // Not required for free poll
        if (!_isFreePoll)
        {
            snapPointToBoundsAndProjectOnMesh(evalPoint, _lb, _ub);
        }

        if (*evalPoint.getX() != *_frameCenter->getX())
        {
            // New EvalPoint to be evaluated.
            // Add it to the list.
            evalPoint.addGenStep(getStepType());
            bool inserted = insertTrialPoint(evalPoint);

            OUTPUT_INFO_START
            std::string s = "Generated point";
            s += (inserted) ? ": " : " not inserted: ";
            s += evalPoint.display();
            AddOutputInfo(s);
            OUTPUT_INFO_END
        }
        else
        {
            OUTPUT_INFO_START
            std::string s = "Generated point not inserted (equal to frame center): ";
            s += evalPoint.display();
            AddOutputInfo(s);
            OUTPUT_INFO_END
        }
        if (_isFreePoll)
        {
            evalPoint.addGenStep(getStepType());
            bool inserted = insertTrialPoint(evalPoint);

            auto fixedVariable = NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this);
            auto pointFull = evalPoint.getX()->makeFullSpacePointFromFixed(fixedVariable);

            OUTPUT_INFO_START
            std::string s = "Generated point (full space)";
            s += (inserted) ? ": " : " not inserted: ";
            s += pointFull.display();
            AddOutputInfo(s);
            OUTPUT_INFO_END
        }
        if (_isFreePoll)
        {
            evalPoint.addGenStep(getStepType());
            bool inserted = insertTrialPoint(evalPoint);
            
            auto fixedVariable = NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this);
            auto pointFull = evalPoint.getX()->makeFullSpacePointFromFixed(fixedVariable);

            OUTPUT_INFO_START
            std::string s = "Generated point (full space)";
            s += (inserted) ? ": " : " not inserted: ";
            s += pointFull.display();
            AddOutputInfo(s);
            OUTPUT_INFO_END
        }
    }

    OUTPUT_INFO_START
    AddOutputInfo("Generated " + NOMAD::itos(getTrialPointsCount()) + " points");
    std::string s = "Generate ";
    s+= (isSecondPass) ? "second pass trial point(s)" : ((_isFreePoll) ? "user free poll trial point(s)" :" first pass trial points");
    s += " for " + getName();
    AddOutputInfo(s, false, true);
    OUTPUT_INFO_END
}


std::list<NOMAD::Direction> NOMAD::PollMethodBase::generateFullSpaceScaledDirections(bool isSecondPass, const NOMAD::MeshBasePtr& mesh)
{

    std::list<NOMAD::Direction> directionsSubSpace, directionsFullSpace;

    if (_varGroups.empty())
    {
        if (!isSecondPass)
        {
            // Creation of the poll directions in the full space
            generateUnitPollDirections(directionsFullSpace,_n);
        }
        else
        {
            // Creation of the directions for the second pass. For example, the N+1th direction in the full space for Ortho N+1 methods
            generateSecondPassDirections(directionsFullSpace);

            // The trial points were needed to generate the second pass direction. We clear the first trial points before adding the second pass trial points.
            _trialPoints.clear();
        }
    }
    else
    {
        // Can be a single varGroup for a selected poll method
        for (const auto& vg : _varGroups)
        {
            directionsSubSpace.clear();

            size_t nVG = vg.size();

            // -------------------------------------------- //
            // TODO : EHH REMOVE
            //std::cout << "\n=== Variable Group Sanity ===\n";
            //std::cout << "_n (current DIM) = " << _n << "\n";
            //std::cout << "vg.size() = " << vg.size() << "\n";
            //std::cout << "_subsetListVG = " << (_subsetListVG ? "true" : "false") << "\n";

            //std::cout << "vg indices: ";
            //for (auto idx : vg) std::cout << idx << " ";
            //std::cout << "\n";

            //std::cout << "directionsSubSpace dimension (first dir): ";
            //if (!directionsSubSpace.empty())
            //    std::cout << directionsSubSpace.front().size() << "\n";
            //else
            //    std::cout << "(empty)\n";
            //std::cout << "============================\n" << std::flush;
            // TODO : EHH REMOVE ABOVE //
            // -------------------------------------------- //


            if (!isSecondPass)
            {
                // Creation of the poll directions in the subspace of the variable group
                generateUnitPollDirections(directionsSubSpace, nVG);
            }
            else
            {
                // Creation of the second pass poll direction in the subspace of the variable group
                generateSecondPassDirections(directionsSubSpace);
                // We do not want to re-evaluate points generated in the first pass.
                _trialPoints.clear();
            }

            // ----------------------------- //
            // TODO : remove EHH 
            //std::cout << "\n=== Variable Group Sanity (AFTER GEN) ===\n";
            //std::cout << "_n (current DIM) = " << _n << "\n";
            //std::cout << "vg.size() = " << vg.size() << " (nVG=" << nVG << ")\n";
            //std::cout << "_subsetListVG = " << (_subsetListVG ? "true" : "false") << "\n";

            //std::cout << "vg indices: ";
            //for (auto idx : vg) std::cout << idx << " ";
            //std::cout << "\n";

            //std::cout << "directionsSubSpace.size() = " << directionsSubSpace.size() << "\n";
            //std::cout << "directionsSubSpace.front().size() = "
            //        << (directionsSubSpace.empty() ? 0 : directionsSubSpace.front().size())
            //        << "\n";
            //std::cout << "========================================\n" << std::flush;
            // --------------------------- //


            // Convert subspace (in a group of variable) directions to full space directions (all variables -> _n)
            // TODO : EHH CHECK with CT if there's a problem here
            if (_subsetListVG || vg.size() != _n)
            //if (_subsetListVG) // OLD LINE
            {
                size_t vgIndex = 0; // For Output debug only
                for (auto& it : directionsSubSpace)
                {
                    // In full space, the direction for an index outside the group of variables is null
                    NOMAD::Direction fullSpaceDirection(_n,0.0);

                    // Copy the subspace direction elements to full space
                    size_t i = 0;
                    for (auto index: vg)
                    {
                        fullSpaceDirection[index] = it[i++];
                    }
                    directionsFullSpace.push_back(fullSpaceDirection);
                    OUTPUT_DEBUG_START
                    AddOutputDebug("Unit poll direction for Variable Group " + std::to_string(vgIndex) + ": "+ fullSpaceDirection.display());
                    OUTPUT_DEBUG_END
                    vgIndex++;
                }
            }
            else
            {
                directionsFullSpace = directionsSubSpace;

                OUTPUT_DEBUG_START
                for (const auto& dir : directionsFullSpace)
                {
                    AddOutputDebug("Unit poll direction: " + dir.display());
                }
                OUTPUT_DEBUG_END

            }
        }
    }

    // Scale and project directions on the mesh
    // Always do it for first pass
    // For second pass only do it when the flag is on. For the moment, the flag is off only for Ortho n+1 quad.
    // Directions from user free poll method are not scaled or projected on mesh.
    if (!_isFreePoll)
    {
        if (!isSecondPass || _scaleAndProjectSecondPassDirectionOnMesh )
        {
            scaleAndProjectOnMesh(directionsFullSpace, mesh);
        }
    }

    return directionsFullSpace;

}


void NOMAD::PollMethodBase::scaleAndProjectOnMesh(std::list<Direction> & dirs, std::shared_ptr<NOMAD::MeshBase> mesh)
{
    // Scale the directions and project on the mesh
    if (nullptr == mesh)
    {
        mesh = getIterationMesh();
    }
    if (nullptr == mesh)
    {
        std::string err("Iteration or Mesh not found.");
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }

    std::list<NOMAD::Direction>::iterator itDir;
    for (itDir = dirs.begin(); itDir != dirs.end(); ++itDir)
    {
        OUTPUT_DEBUG_START
        AddOutputDebug("Poll direction before scaling and projection on mesh: " + itDir->display());
        OUTPUT_DEBUG_END

        Direction scaledDir(_n,0.0);

        // Compute infinite norm for direction pointed by itDir.
        NOMAD::Double infiniteNorm = (*itDir).infiniteNorm();
        if (0 == infiniteNorm)
        {
            std::string err("Cannot handle an infinite norm of zero");
            throw NOMAD::Exception(__FILE__, __LINE__, err);
        }

        for (size_t i = 0; i < _n; ++i)
        {
            // Scaling and projection on the mesh
            scaledDir[i] = mesh->scaleAndProjectOnMesh(i, (*itDir)[i] / infiniteNorm);
        }

        *itDir = scaledDir;
    }
    OUTPUT_DEBUG_START
    NOMAD::OutputQueue::Flush();
    OUTPUT_DEBUG_END
}


void NOMAD::PollMethodBase::setListVariableGroups(const ListOfVariableGroup & varGroups)
{
    if (varGroups.empty())
    {
        return;
    }

    _varGroups = varGroups;
    size_t dimLVG = std::accumulate(varGroups.begin(), varGroups.end(), size_t{0}, [](size_t s,const NOMAD::VariableGroup & vg){ return s+vg.size();});
    if (dimLVG < _n)
    {
        _subsetListVG = true;
    }
}
