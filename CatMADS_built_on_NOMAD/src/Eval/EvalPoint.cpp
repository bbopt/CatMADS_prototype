/**
 \file   EvalPoint.cpp
 \brief  Evaluation point (implementation)
 \author Sebastien Le Digabel and Viviane Rochon Montplaisir
 \date   March 2017
 \see    EvalPoint.hpp
 */
#include <memory>

#include "../Eval/EvalPoint.hpp"
#include "../Output/OutputQueue.hpp"
#include "../Math/MatrixUtils.hpp"

int NOMAD::EvalPoint::_currentTag = -1;

/*---------------------------------------------------------------------*/
/*                            Constructor 1                            */
/*---------------------------------------------------------------------*/
NOMAD::EvalPoint::EvalPoint ()
  : NOMAD::Point(),
    _eval(),
    _tag(-1),
    _threadAlgo(NOMAD::getThreadNum()),
    _numberBBEval(0),
    _evalFromCacheFile(false),
    _pointFrom(nullptr),
    _genSteps(),
    _direction(nullptr),
    _angle(),
    _mesh(nullptr),
    _isRevealing(0),
    _userFailEvalCheck(false)
{
    initEval();
}


/*---------------------------------------------------------------------*/
/*                            Constructor 2                            */
/*---------------------------------------------------------------------*/
NOMAD::EvalPoint::EvalPoint(size_t n)
  : NOMAD::Point(n),
    _eval(),
    _tag(-1),
    _threadAlgo(NOMAD::getThreadNum()),
    _numberBBEval(0),
    _evalFromCacheFile(false),
    _pointFrom(nullptr),
    _genSteps(),
    _direction(nullptr),
    _angle(),
    _mesh(nullptr),
    _isRevealing(0),
    _userFailEvalCheck(false)
{
    initEval();
}


/*---------------------------------------------------------------------*/
/*                            Constructor 3                            */
/*---------------------------------------------------------------------*/
NOMAD::EvalPoint::EvalPoint(const NOMAD::Point &x)
  : NOMAD::Point(x),
    _eval(),
    _tag(-1),
    _threadAlgo(NOMAD::getThreadNum()),
    _numberBBEval(0),
    _evalFromCacheFile(false),
    _pointFrom(nullptr),
    _genSteps(),
    _direction(nullptr),
    _angle(),
    _mesh(nullptr),
    _isRevealing(0),
    _userFailEvalCheck(false)
{
    initEval();
}


/*---------------------------------------------------------------------*/
/*                          Initialization                             */
/*---------------------------------------------------------------------*/
void NOMAD::EvalPoint::initEval()
{
    for (auto& i : _eval)
    {
        i.reset();
    }
}


/*---------------------------------------------------------------------*/
/*                           Copy Constructor                          */
/*---------------------------------------------------------------------*/
NOMAD::EvalPoint::EvalPoint(const NOMAD::EvalPoint &evalPoint)
  : NOMAD::Point(evalPoint)
{
    initEval();
    copyMembers(evalPoint);
}


/*---------------------------------------------------------------------*/
/* Helper for copy constructor                                         */
/* Copy all members except Point.                                      */
/* Useful when converting from full to sub dimension and vice versa.   */
/*---------------------------------------------------------------------*/
void NOMAD::EvalPoint::copyMembers(const NOMAD::EvalPoint &evalPoint)
{
    _tag = evalPoint._tag;
    _threadAlgo = evalPoint._threadAlgo;
    _numberBBEval = evalPoint._numberBBEval;
    _evalFromCacheFile = evalPoint._evalFromCacheFile;

    // Copy evals
    for (size_t i = 0; i < (size_t)NOMAD::EvalType::LAST; i++)
    {
        auto evalType = NOMAD::EvalType(i);
        NOMAD::Eval* eval = evalPoint.getEval(evalType);
        if (nullptr != eval)
        {
            // deep copy.
            _eval[(size_t) evalType] = std::make_unique<NOMAD::Eval>(*eval);
        }
    }

    // shallow copy
    _pointFrom = evalPoint.getPointFrom();
    _genSteps = evalPoint.getGenSteps();
    _direction = evalPoint.getDirection();
    _angle = evalPoint.getAngle();
    if ( nullptr != evalPoint._mesh )
    {
        _mesh = evalPoint._mesh->clone();
    }
    _isRevealing = evalPoint.getRevealingStatus();
    _userFailEvalCheck = evalPoint.getUserFailEvalCheck();
}


/*-----------------------------------------------------------*/
/*                     Affectation operator                  */
/*-----------------------------------------------------------*/
NOMAD::EvalPoint & NOMAD::EvalPoint::operator=(const NOMAD::EvalPoint &evalPoint)
{
    if (this == &evalPoint)
    {
        return *this;
    }

    NOMAD::Point::operator=(evalPoint);

    _tag = evalPoint._tag;
    _threadAlgo = evalPoint._threadAlgo;
    _numberBBEval = evalPoint._numberBBEval;
    _evalFromCacheFile = evalPoint._evalFromCacheFile;

    _pointFrom = evalPoint._pointFrom;
    _genSteps = evalPoint._genSteps;
    _direction = evalPoint._direction;
    _angle = evalPoint._angle;
    if (nullptr != evalPoint._mesh)
    {
        _mesh = evalPoint._mesh->clone();
    }
    _isRevealing = evalPoint._isRevealing;
    _userFailEvalCheck = evalPoint._userFailEvalCheck;

    // Do NOT delete _eval. Since it is a smart ptr, it will take care
    // of itself. Releasing the smart ptr here causes a memory leak.

    for (size_t i = 0; i < (size_t)NOMAD::EvalType::LAST; i++)
    {
        auto evalType = NOMAD::EvalType(i);
        if (nullptr == evalPoint.getEval(evalType))
        {
            _eval[(size_t) evalType].reset();
        }
        else
        {
            // deep copy.
            _eval[(size_t) evalType] = std::make_unique<NOMAD::Eval>(*evalPoint.getEval(evalType));
        }
    }

    return *this;
}


/*---------------------------------------------------------------------*/
/*                               Destructor                            */
/*---------------------------------------------------------------------*/
NOMAD::EvalPoint::~EvalPoint () = default;


/*-----------------------------------------------------------*/
/*                           operator ==                     */
/*-----------------------------------------------------------*/
// Note: Considering all eval types in _eval.
bool NOMAD::EvalPoint::operator== (const NOMAD::EvalPoint &evalPoint) const
{
    // First compare Points.
    bool equal = NOMAD::Point::operator==(evalPoint);

    // Ignore tag.
    // Ignore numberEval.
    // Ignore pointFrom and genSteps.

    // Compare Evals for evalTypes BB, MODEL, SURROGATE.
    for (size_t i = 0; (equal && i < (size_t)NOMAD::EvalType::LAST); i++)
    {
        auto evalType = NOMAD::EvalType(i);
        if (equal)
        {
            auto eval = getEval(evalType);
            auto eval2 = evalPoint.getEval(evalType);

            if (nullptr == eval && nullptr == eval2)
            {
                // Both Evals are NULL.
                equal = true;
            }
            else if (nullptr == eval || nullptr == eval2)
            {
                // One Eval is NULL, but not both.
                equal = false;
            }
            else
            {
                // General case
                equal = ( *eval == *(eval2) );
            }
        }
    }

    return equal;
}


/*---------------------*/
/* Other class methods */
/*---------------------*/
bool NOMAD::EvalPoint::isEvalOk(NOMAD::EvalType evalType) const
{
    bool ret = false;

    auto eval = getEval(evalType);
    if (eval)
    {
        ret = (NOMAD::EvalStatusType::EVAL_OK == eval->getEvalStatus());
    }

    return ret;
}


void NOMAD::EvalPoint::setMesh(const std::shared_ptr<MeshBase>& mesh)
{

    if ( nullptr == mesh )
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Cannot clone a null mesh" );
    }

    _mesh = mesh->clone();
}

/*---------*/
/* Get/Set */
/*---------*/
NOMAD::Eval* NOMAD::EvalPoint::getEval(NOMAD::EvalType evalType) const
{
    auto indMap = (size_t) evalType;
    if (indMap >= (size_t) NOMAD::EvalType::LAST)
    {
        return nullptr;
    }
    else
    {
        return _eval[(size_t) evalType].get();
    }
}

NOMAD::EvalType NOMAD::EvalPoint::getSingleEvalType(NOMAD::EvalStatusType evalStatusType) const
{

    auto foundIndMap = (size_t) NOMAD::EvalType::LAST;
    for (size_t indMap = 0 ; indMap < (size_t) NOMAD::EvalType::LAST ; indMap++ )
    {
        if ( nullptr != _eval[indMap] && evalStatusType == _eval[indMap]->getEvalStatus())
        {
            if (foundIndMap != (size_t) NOMAD::EvalType::LAST)
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"There should be only one defined eval in this eval point. ");
            }
            else
            {
                foundIndMap = indMap;
            }
        }
    }
    return static_cast<NOMAD::EvalType>(foundIndMap);
}

void NOMAD::EvalPoint::setEval(const NOMAD::Eval& eval,
                               NOMAD::EvalType evalType)
{
    _eval[(size_t) evalType] = std::make_unique<NOMAD::Eval>(eval);
}


NOMAD::Double NOMAD::EvalPoint::getF(const FHComputeType& computeType) const
{
    auto eval = getEval(computeType.evalType);
    if (nullptr == eval || NOMAD::EvalStatusType::EVAL_OK != eval->getEvalStatus())
    {
        return NOMAD::INF;
        // throw NOMAD::Exception(__FILE__,__LINE__,"EvalPoint::getF() called for an EvalPoint that is not EVAL_OK");
    }

    return eval->getF(computeType.Short());
}


const NOMAD::ArrayOfDouble& NOMAD::EvalPoint::getFs(const FHComputeType& computeType) const
{
    auto eval = getEval(computeType.evalType);
    if (nullptr == eval || NOMAD::EvalStatusType::EVAL_OK != eval->getEvalStatus())
    {
              throw NOMAD::Exception(__FILE__,__LINE__,"EvalPoint::getFs() called for an EvalPoint that is not EVAL_OK");
    }

    return eval->getFs(computeType.Short());
}


NOMAD::Double NOMAD::EvalPoint::getH(const FHComputeType& computeType) const
{
    NOMAD::Double h;

    auto eval = getEval(computeType.evalType);
    if (nullptr == eval || NOMAD::EvalStatusType::EVAL_OK != eval->getEvalStatus())
    {
        return NOMAD::INF;
    }

    return eval->getH(computeType.Short());
}


std::string NOMAD::EvalPoint::getBBO(NOMAD::EvalType evalType) const
{
    std::string bbo;
    auto eval = getEval(evalType);

    if (nullptr != eval)
    {
        bbo = eval->getBBOutput().getBBO();
    }

    return bbo;
}


void NOMAD::EvalPoint::setBBO(const std::string &bbo,
                              const NOMAD::BBOutputTypeList &bbOutputTypeList,
                              NOMAD::EvalType evalType,
                              const bool evalOk)
{
    // The default (unset) eval type is passed (see library mode examples using simple setBBO(bbo) function). This function is used for simplicity BUT we need to check which eval is in progress. It should not be too costly.
    // Quad model evaluator passes the eval type explicitly.
    // Also Evaluator::evalXBBExe passes the eval type explicitly.
    NOMAD::Eval * eval = nullptr;
    if (NOMAD::EvalType::LAST == evalType)
    {
        // Select the single eval in progress
        evalType = getSingleEvalType(NOMAD::EvalStatusType::EVAL_IN_PROGRESS);
    }
    eval = getEval(evalType);

    if (nullptr == eval)
    {
        _eval[(size_t) evalType] = std::make_unique<NOMAD::Eval>(NOMAD::Eval());
        eval = getEval(evalType);
    }

    if (nullptr == eval)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "EvalPoint::setBBO: Could not create new Eval");
    }
    else
    {
        eval->setBBO(bbo, bbOutputTypeList, evalOk);
    }

}


void NOMAD::EvalPoint::setBBO(const std::string &bbo,
                              const std::string &sBBOutputTypes,
                              NOMAD::EvalType evalType,
                              const bool evalOk)
{
    NOMAD::BBOutputTypeList bbOutputTypeList = NOMAD::stringToBBOutputTypeList(sBBOutputTypes);
    setBBO(bbo, bbOutputTypeList, evalType, evalOk);
}


void NOMAD::EvalPoint::setBBOutputType(const NOMAD::BBOutputTypeList& bbOutputType,
                                       const NOMAD::EvalType evalType) const
{
    auto eval = getEval(evalType);
    if (nullptr != eval)
    {
        eval->setBBOutputTypeList(bbOutputType);
    }
}

void NOMAD::EvalPoint::setBBOutputType(const NOMAD::BBOutputTypeList& bbOutputType)
{
    for (const auto& indMap : _eval)
    {
        if (nullptr != indMap)
        {
            indMap->setBBOutputTypeList(bbOutputType);
        }
    }
}

NOMAD::EvalStatusType NOMAD::EvalPoint::getEvalStatus(NOMAD::EvalType evalType) const
{
    NOMAD::EvalStatusType evalStatus = NOMAD::EvalStatusType::EVAL_STATUS_UNDEFINED;

    auto eval = getEval(evalType);
    if (nullptr != eval)
    {
        evalStatus = eval->getEvalStatus();
    }

    return evalStatus;
}

NOMAD::EvalStatusType NOMAD::EvalPoint::getPreEvalStatus(NOMAD::EvalType evalType) const
{

    NOMAD::EvalStatusType evalStatus = NOMAD::EvalStatusType::EVAL_STATUS_UNDEFINED;

    auto eval = getEval(evalType);
    if (nullptr != eval)
    {
        evalStatus = eval->getPreEvalStatus();
    }

    return evalStatus;
}


void NOMAD::EvalPoint::setEvalStatus(NOMAD::EvalStatusType evalStatus,
                                     NOMAD::EvalType evalType)
{
    if (((size_t) evalType) >= ((size_t) NOMAD::EvalType::LAST))
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "EvalPoint::setEvalStatus: Could not create new Eval");
    }

    auto eval = getEval(evalType);

    if (nullptr == eval)
    {
        _eval[(size_t) evalType] = std::make_unique<NOMAD::Eval>();
        eval = getEval(evalType);
    }

    if (nullptr == eval)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "EvalPoint::setEvalStatus: Could not create new Eval");
    }
    else
    {
        eval->setEvalStatus(evalStatus);
    }
}

void NOMAD::EvalPoint::setPreEvalStatus(NOMAD::EvalStatusType evalStatus,
                                        NOMAD::EvalType evalType)
{
    if (NOMAD::EvalType::BB != evalType)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "EvalPoint::setPreEvalStatus: Eval type should be BB");
    }

    if (((size_t) evalType) >= ((size_t) NOMAD::EvalType::LAST))
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "EvalPoint::setEvalStatus: Could not create new Eval");
    }

    auto eval = getEval(evalType);

    if (nullptr == eval)
    {
        _eval[(size_t) evalType] = std::make_unique<NOMAD::Eval>();
        eval = getEval(evalType);
    }

    if (nullptr == eval)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "EvalPoint::setEvalStatus: Could not create new Eval");
    }
    else
    {
        eval->setPreEvalStatus(evalStatus);
    }
}

NOMAD::Double NOMAD::EvalPoint::getRevealedConstraint() const
{
    // Return value of revealed constraint
    NOMAD::Double constraintValue;

    auto eval = getEval(NOMAD::EvalType::BB);
    {
        if (nullptr != eval)
        {
            auto allBbot = eval->getBBOutputTypeList();

            // Index of revealed constraint
            auto it = std::find(allBbot.begin(),allBbot.end(), NOMAD::BBOutputType::RPB);
            if (it != allBbot.end())
                {
                    // Access value
                    size_t index = it - allBbot.begin();
                    auto bbo = eval->getBBOutput().getBBOAsArrayOfDouble();
                    constraintValue = bbo[index];
                    return constraintValue;
                }
            else
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"getRevealedConstraint(): no revealed constraint.");
            }
        }
        else{
            throw NOMAD::Exception(__FILE__,__LINE__,"getRevealedConstraint(): Eval part of EvalPoint is not defined.");
        }
    }
}

void NOMAD::EvalPoint::setRevealedConstraint(const NOMAD::Double &constraintValue) const
{

    auto eval = getEval(NOMAD::EvalType::BB);
    {
        if (nullptr != eval)
        {
            auto allBbot = eval->getBBOutputTypeList();

            // Index of revealed constraint
            auto it = std::find(allBbot.begin(),allBbot.end(), NOMAD::BBOutputType::RPB);
            if (it != allBbot.end())
                {
                    // Update value
                    size_t index = it - allBbot.begin();
                    auto bbo = eval->getBBOutput().getBBOAsArrayOfDouble();
                    bbo[index]=constraintValue;
                    eval->setBBO(bbo.tostring(), allBbot);
                }
            else
            {
                throw NOMAD::Exception(__FILE__,__LINE__,"setRevealedConstraint(): no revealed constraint.");
            }
        }
        else{
            throw NOMAD::Exception(__FILE__,__LINE__,"setRevealedConstraint(): eval is no defined.");
        }
    }
}


// This method is declared const so we can use it inside a const method.
void NOMAD::EvalPoint::updateTag() const
{
    if (-1 == _tag)
#ifdef OPENMP
#pragma omp critical
#endif
    {
        _currentTag++;
        _tag = _currentTag;
    }
}


void NOMAD::EvalPoint::resetCurrentTag()
{
    _currentTag = -1;
}


std::shared_ptr<NOMAD::EvalPoint> NOMAD::EvalPoint::getPointFrom(const NOMAD::Point& fixedVariable) const
{
    auto pointFrom = _pointFrom;
    if (nullptr != pointFrom)
    {
        pointFrom = std::make_shared<NOMAD::EvalPoint>(pointFrom->projectPointToSubspace(fixedVariable));
    }

    return pointFrom;
}


void NOMAD::EvalPoint::setPointFrom(const std::shared_ptr<NOMAD::EvalPoint>& pointFrom, const NOMAD::Point& fixedVariable)
{
    auto pointFromFull = pointFrom;
    if (pointFromFull->size() < fixedVariable.size())
    {
        // pointFrom must always be in full dimension. Convert if needed.
        pointFromFull = std::make_shared<NOMAD::EvalPoint>(pointFromFull->makeFullSpacePointFromFixed(fixedVariable));
    }

    _pointFrom = pointFromFull;

    // Also set Direction.
    if (nullptr != pointFromFull)
    {
        NOMAD::Point pointFull(*getX());
        if (pointFull.size() < fixedVariable.size())
        {
            pointFull = pointFull.makeFullSpacePointFromFixed(fixedVariable);
        }
        _direction = std::make_shared<NOMAD::Direction>(NOMAD::Point::vectorize(*pointFromFull, pointFull));
    }
}


void NOMAD::EvalPoint::addGenStep(const NOMAD::StepType& genStep, bool inherit)
{
    if (!inherit)
    {
        _genSteps.clear();
    }

    // Do not add duplicates.
    size_t nbSteps = _genSteps.size();
    if (nbSteps >= 1 && _genSteps[nbSteps-1] == genStep)
    {
        return;
    }
    _genSteps.push_back(genStep);
}


NOMAD::StepType NOMAD::EvalPoint::getGenStep() const
{
    if (_genSteps.empty())
    {
        return NOMAD::StepType::UNDEFINED;
    }
    return *_genSteps.begin();
}


const NOMAD::StepTypeList& NOMAD::EvalPoint::getGenSteps() const
{
    return _genSteps;
}


void NOMAD::EvalPoint::setGenSteps(const NOMAD::StepTypeList& genSteps)
{
    _genSteps = genSteps;
}


bool NOMAD::EvalPoint::getGenByPhaseOne() const
{
    for (auto stepType : _genSteps)
    {
        if (NOMAD::StepType::ALGORITHM_PHASE_ONE == stepType)
        {
            return true;
        }
    }
    return false;
}


std::string NOMAD::EvalPoint::getComment() const
{
    if (getGenByPhaseOne())
    {
        return "(Phase One)";
    }

    return "";
}


bool NOMAD::EvalPoint::isFeasible(const FHComputeType& computeType) const
{
    auto eval = getEval(computeType.evalType);
    if (nullptr == eval || NOMAD::EvalStatusType::EVAL_OK != eval->getEvalStatus())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"EvalPoint::isFeasible: Needs eval to be defined and status to be EVAL_OK.");
    }

    return eval->isFeasible(computeType.Short());
}

bool NOMAD::EvalPoint::isEBOk(NOMAD::EvalType evalType) const
{
    bool isEBOk = true;
    auto eval = getEval(evalType);
    if (nullptr == eval || NOMAD::EvalStatusType::EVAL_OK != eval->getEvalStatus())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"EvalPoint::isEBOk: Needs eval to be defined and status to be EVAL_OK.");
    }

    // Infeasibility measure accounting for EB constraints only is computed with phase one compute type
    const FHComputeTypeS computeType = {NOMAD::ComputeType::PHASE_ONE, NOMAD::HNormType::L2, NOMAD::defaultEmptySingleOutputCompute /* not used*/};
    NOMAD::Double EBconstraintsInfeasibility = eval->getF(computeType);

    if(EBconstraintsInfeasibility>0)
    {
        isEBOk = false;
    }
    return isEBOk;
}



NOMAD::EvalPoint NOMAD::EvalPoint::makeFullSpacePointFromFixed(const NOMAD::Point &fixedVariable) const
{
    NOMAD::EvalPoint fullSpaceEvalPoint(getX()->makeFullSpacePointFromFixed(fixedVariable));
    fullSpaceEvalPoint.copyMembers(*this);

    return fullSpaceEvalPoint;
}


NOMAD::EvalPoint NOMAD::EvalPoint::makeSubSpacePointFromFixed(const NOMAD::Point &fixedVariable) const
{
    NOMAD::EvalPoint subSpaceEvalPoint(getX()->makeSubSpacePointFromFixed(fixedVariable));
    subSpaceEvalPoint.copyMembers(*this);

    return subSpaceEvalPoint;
}


// Should we evaluate (possibly re-evaluate) this point?
bool NOMAD::EvalPoint::toEval(short maxPointBBEval, NOMAD::EvalType evalType) const
{

    bool reEval = false;

    auto eval = getEval(evalType);
    if (nullptr == eval)
    {
        // No eval, return true.
        reEval = true;
    }
    else if (NOMAD::EvalType::MODEL == evalType || NOMAD::EvalType::SURROGATE == evalType)
    {
        // If using model, or static surrogate, never allow re-evaluation.
        reEval = false;
    }
    else if (_numberBBEval >= maxPointBBEval)
    {
        // Too many evaluations, return false.
        reEval = false;
    }
    else if (_numberBBEval >= 1 && NOMAD::EvalStatusType::EVAL_OK == eval->getEvalStatus())
    {
        // For now, we will not re-evaluate an EvalPoint that is EVAL_OK.
        reEval = false;
    }
    else
    {
        reEval = eval->canBeReEvaluated();
    }

    return reEval;
}


// Displaying only bb eval
std::string NOMAD::EvalPoint::display(const NOMAD::FHComputeTypeS& computeType,
                                      const NOMAD::ArrayOfDouble &pointFormat,
                                      const int &solFormat,
                                      const bool surrogateAsBB) const
{
    std::string s;
    if (_tag >= 0)
    {
        s = "#" + std::to_string(_tag) + " ";
    }
    s += NOMAD::Point::display(pointFormat);
    auto eval = (surrogateAsBB) ? getEval(NOMAD::EvalType::SURROGATE)
                                : getEval(NOMAD::EvalType::BB);
    if (nullptr != eval)
    {
        s += "\t";
        s += eval->display(computeType, solFormat);
    }
    return s;
}


std::string NOMAD::EvalPoint::display(const NOMAD::ArrayOfDouble &pointFormat,
                                      const int &solFormat) const
{
    return display(defaultFHComputeTypeS, pointFormat, solFormat);
}

// Display only BB and SURROGATE. Model is not displayed
std::string NOMAD::EvalPoint::displayForCache(const NOMAD::ArrayOfDouble &pointFormat) const
{
    // Example:
    // ( 1.7 2.99 -2.42 2.09 -36 2.33 ) EVAL_FAILED ( NaN 0 -20 )
    std::string s;

    NOMAD::Point p = *(getX());
    s = p.display(pointFormat);

    std::ostringstream oss;
    for (size_t indMap = 0 ; indMap <= (size_t) NOMAD::EvalType::SURROGATE ; indMap++ )
    {
        if (nullptr != _eval[indMap])
        {
            oss << " " << NOMAD::evalTypeToString(NOMAD::EvalType(indMap)) << "_" << _eval[indMap]->getEvalStatus();     // Raw, ex. "BB_EVAL_OK"
            oss << " " << NOMAD::BBOutput::bboStart << " " << _eval[indMap]->getBBO();
            oss << " " << NOMAD::BBOutput::bboEnd;
        }
        else
        {
            oss << " " << NOMAD::evalTypeToString(NOMAD::EvalType(indMap)) << "_" << NOMAD::EvalStatusType::EVAL_NOT_STARTED;
            oss << " " << NOMAD::BBOutput::bboStart << " " << NOMAD::BBOutput::bboEnd;
        }
    }
    s += oss.str();

    return s;
}


// Show all evals. For debugging purposes.
std::string NOMAD::EvalPoint::displayAll(const NOMAD::FHComputeTypeS & computeType) const
{
    std::string s;
    if (_tag >= 0)
    {
        s = "#" + std::to_string(_tag) + " ";
    }
    s += NOMAD::Point::display();
    for (size_t i = 0; i < (size_t)NOMAD::EvalType::LAST; i++)
    {
        auto evalType = NOMAD::EvalType(i);
        auto eval = getEval(evalType);
        if (nullptr != eval)
        {
            s += "\t";
            s += "(" + NOMAD::evalTypeToString(evalType) + " - ";
            s += eval->display(computeType);
            s += ")";
        }
    }
    return s;
}


// Determine if an eval point has a bb (regular) eval.
bool NOMAD::EvalPoint::hasBbEval(const NOMAD::EvalPoint& evalPoint)
{
    return (nullptr != evalPoint.getEval(NOMAD::EvalType::BB));
}


// Determine if an evalpoint has a model eval.
bool NOMAD::EvalPoint::hasModelEval(const NOMAD::EvalPoint& evalPoint)
{
    return (nullptr != evalPoint.getEval(NOMAD::EvalType::MODEL));
}


// Determine if an eval point has a static surrogate eval.
bool NOMAD::EvalPoint::hasSurrogateEval(const NOMAD::EvalPoint& evalPoint)
{
    return (nullptr != evalPoint.getEval(NOMAD::EvalType::SURROGATE));
}


bool NOMAD::EvalPoint::isPhaseOneSolution(const NOMAD::EvalPoint& evalPoint, const NOMAD::FHComputeType & completeComputeType)
{
    bool issol = false;

    auto eval = evalPoint.getEval(completeComputeType.evalType);
    auto computeType = completeComputeType.Short();
    if (nullptr != eval && NOMAD::EvalStatusType::EVAL_OK == eval->getEvalStatus())
    {
        issol = (0.0 == eval->getF(computeType).todouble());
    }

    return issol;
}


std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::EvalPoint &evalPoint)
{
    // Example:
    // ( 1.7 2.99 -2.42 2.09 -36 2.33 ) EVAL_FAILED ( NaN 0 -20 )

    NOMAD::Point p = *(evalPoint.getX());
    // Since this operator is used to write cache, we need full precision on point.
    os << p.display(NOMAD::ArrayOfDouble(evalPoint.size(), NOMAD::DISPLAY_PRECISION_FULL));

    // Never use model eval in input/output stream operators
    const NOMAD::Eval* eval = evalPoint.getEval(NOMAD::EvalType::BB);
    if (nullptr != eval)
    {
        os << " BB_" << eval->getEvalStatus();     // Raw, ex. "EVAL_OK"
        os << " " << NOMAD::BBOutput::bboStart << " " << eval->getBBO();
        os << " " << NOMAD::BBOutput::bboEnd ;
    }
    else
    {
        os << " BB_" << NOMAD::EvalStatusType::EVAL_NOT_STARTED;
        os << " " << NOMAD::BBOutput::bboStart << " " ;
        os << " " << NOMAD::BBOutput::bboEnd;
    }
    const NOMAD::Eval* evalSur = evalPoint.getEval(NOMAD::EvalType::SURROGATE);
    if (nullptr != evalSur)
    {
        os << " SURROGATE_" << evalSur->getEvalStatus();     // Raw, ex. "EVAL_OK"
        os << " " << NOMAD::BBOutput::bboStart << " " << evalSur->getBBO();
        os << " " << NOMAD::BBOutput::bboEnd;
    }
    else
    {
        os << " SURROGATE_" << NOMAD::EvalStatusType::EVAL_NOT_STARTED;
        os << " " << NOMAD::BBOutput::bboStart << " " ;
        os << " " << NOMAD::BBOutput::bboEnd;
    }


    return os;
}

// Operator for BB and SURROGATE eval type. Model is not output
std::istream& NOMAD::operator>>(std::istream& is, NOMAD::EvalPoint &evalPoint)
{
    // Set up structures to gather member info
    NOMAD::Point point;
    NOMAD::EvalStatusType evalStatus = NOMAD::EvalStatusType::EVAL_STATUS_UNDEFINED;
    bool skip = false;

    std::string s;
    is >> s;

    if (s.empty() || !is.good() || is.eof())
    {
        skip = true;
    }

    if (!skip && NOMAD::ArrayOfDouble::pStart == s)
    {
        // Found start of point.
        is.unget();
        is >> point;

        evalPoint = NOMAD::EvalPoint(point);

        for (size_t indMap = 0 ; indMap <= (size_t) NOMAD::EvalType::SURROGATE ; indMap++ )
        {
            // Read Eval - if following field is an EvalStatus.
            is >> evalStatus;
            if (NOMAD::EvalStatusType::EVAL_STATUS_UNDEFINED != evalStatus)
            {

                // Read BBOutput. EvalType is BB. Bb output types is determined later.
                NOMAD::BBOutput bbo("");
                is >> bbo;

                // No need to set eval if eval not started (no evaluation performed)
                if (NOMAD::EvalStatusType::EVAL_NOT_STARTED == evalStatus)
                    continue;

                // Never use model eval in input/output stream operators
                evalPoint.setEvalStatus(evalStatus, NOMAD::EvalType(indMap));

                evalPoint.setBBO(bbo.getBBO(), NOMAD::BBOutputTypeList(), NOMAD::EvalType(indMap));

                // For now, set numEval to 1 if Eval exists. Currently,
                // only 1 Eval is correctly supported.
                evalPoint.setNumberBBEval(1);
            }
        }
    }
    else if (!skip)
    {
        is.setstate(std::ios::failbit);
        std::string err = "Expecting \"" + NOMAD::ArrayOfDouble::pStart + "\", got \"" + s + "\"";
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }


    return is;
}


bool NOMAD::findInList(const NOMAD::Point& point,
                       const std::vector<NOMAD::EvalPoint>& evalPointList,
                       NOMAD::EvalPoint& foundEvalPoint)
{
    bool found = false;

    for (const auto & evalPoint : evalPointList)
    {
        if (point == *evalPoint.getX())
        {
            foundEvalPoint = evalPoint;
            found = true;
            break;
        }
    }

    return found;
}


void NOMAD::convertPointListToSub(std::vector<NOMAD::EvalPoint> &evalPointList, const NOMAD::Point& fixedVariable)
{
    if (fixedVariable.isEmpty())
    {
        std::string s = "Error: Fixed variable of dimension 0";
        throw NOMAD::Exception(__FILE__,__LINE__,s);
    }
    for (auto & evalPoint: evalPointList)
    {
        if (evalPoint.size() == fixedVariable.size())
        {
            evalPoint = evalPoint.makeSubSpacePointFromFixed(fixedVariable);
        }
    }
}


void NOMAD::convertPointListToFull(std::vector<NOMAD::EvalPoint> &evalPointList, const NOMAD::Point& fixedVariable)
{
    for (auto & evalPoint: evalPointList)
    {
        if (evalPoint.size() == fixedVariable.size() - fixedVariable.nbDefined())
        {
            evalPoint = evalPoint.makeFullSpacePointFromFixed(fixedVariable);
        }
    }
}


#ifdef USE_UNORDEREDSET
/// Used for unordered set.
/// Template specialization of std::hash<class T> to std::hash<NOMAD@::EvalPoint>
size_t std::hash<NOMAD::EvalPoint>::operator()(const NOMAD::EvalPoint& evalPoint) const
{
    double eps = NOMAD::Double::getEpsilon();
    double sizeMax = SIZE_MAX;
    size_t hashKey = 0;
    double hashKeyDouble = hashKey; // Compute hashKey in double.

    for (size_t i = 0; i < evalPoint.size(); i++)
    {
        hashKeyDouble *= 10.0;
        double t = std::trunc(std::fabs(evalPoint[i].todouble() / eps));
        hashKeyDouble += t;

        // avoid size_t overflow, which would set hashKey to 0.
        while (hashKeyDouble > sizeMax)
        {
            hashKeyDouble -= sizeMax;
        }
    }

    // convert to size_t
    hashKey = size_t(hashKeyDouble);

    return hashKey;
}


/// Used for unordered set.
/// Template specialization of std::equal_to<class T> to equal_to<NOMAD::EvalPoint>
bool std::equal_to<NOMAD::EvalPoint>::operator()(const NOMAD::EvalPoint& lhs, const NOMAD::EvalPoint& rhs) const
{

    return (lhs == rhs);
}
#endif // USE_UNORDEREDSET


bool NOMAD::EvalPoint::dominates(const NOMAD::EvalPoint &ep,
                                 const FHComputeType& completeComputeType) const
{
    bool dom = false;
    auto evalType = completeComputeType.evalType;
    if (this != &ep && nullptr != getEval(evalType) && nullptr != ep.getEval(evalType))
    {
        dom = getEval(evalType)->dominates(*ep.getEval(evalType), completeComputeType.Short());
    }

    return dom;
}

NOMAD::CompareType NOMAD::EvalPoint::compMO(const EvalPoint &ep,
                                            const NOMAD::FHComputeType& completeComputeType,
                                            bool onlyfvalues) const
{
    NOMAD::CompareType compareFlag = NOMAD::CompareType::UNDEFINED;
    auto evalType = completeComputeType.evalType;
    if (this != &ep && nullptr != getEval(evalType) && nullptr != ep.getEval(evalType))
    {
        compareFlag = getEval(evalType)->compMO(*ep.getEval(evalType), completeComputeType.Short(), onlyfvalues);
    }

    return compareFlag;
}


// Get the rank of the generating directions of a list of eval points
size_t NOMAD::EvalPoint::getRank(const std::vector<NOMAD::EvalPoint> & vectEvalPoints )
{

    if ( vectEvalPoints.empty() )
        throw NOMAD::Exception(__FILE__, __LINE__, "There is no generating directions "); // Maybe replace exception by return 0;

    // The dimension of DV (k)
    size_t k = vectEvalPoints.size() ;

    if (k == 1)
    {
        if ((*(vectEvalPoints[0].getDirection())).norm() > 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    const size_t n = vectEvalPoints[0].size();

    // DV : vector generating direction (2D array)
    auto DV = new double *[k];
    for (size_t i = 0 ; i < k ; ++i )
        DV[i]=new double [n];

    // For debugging
    std::ostringstream outDbg;
    outDbg << "The rank of DV=[";

    // Create DZ
    size_t j=0;
    while ( j < k )
    {
        outDbg << " (" ;

        for ( size_t i = 0; i < n ; i++ )
        {
            DV[j][i] = (*(vectEvalPoints[j].getDirection()))[i].todouble();
            outDbg << DV[j][i] << " ";
        }
        j++;
        outDbg << ")" ;

    }

    // Get the rank
    int rank= NOMAD::getRank(DV , k , n , NOMAD::DEFAULT_EPSILON );

    OUTPUT_DEBUGDEBUG_START
    outDbg << " ] equals " << rank;

    NOMAD::OutputQueue::Add(outDbg.str(), NOMAD::OutputLevel::LEVEL_DEBUGDEBUG);
    OUTPUT_DEBUGDEBUG_END

    for (size_t i=0 ; i < k ; ++i)
        delete [] DV[i];
    delete [] DV;

    return rank;
}


void NOMAD::EvalPoint::resetDMultiCombineFValue()
{
    for (const auto& i : _eval)
    {
        if (i != nullptr)
        {
            i->resetDMultiCombineFValue();
        }
    }
}

void NOMAD::EvalPoint::resetFValues()
{
    for (const auto& evalType : _eval)
    {
        if (evalType != nullptr)
        {
            evalType->resetFValues();
        }
    }
}
