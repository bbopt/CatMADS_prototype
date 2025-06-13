
#include "../../Algos/Mads/GMesh.hpp"
#include "../../Algos/Mads/Mads.hpp"
#include "../../Algos/Mads/MadsInitialization.hpp"
#include "../../Algos/Mads/MadsMegaIteration.hpp"
#include "../../Algos/Mads/MadsIteration.hpp"
#include "../../Algos/Mads/MadsUpdate.hpp"
#include "../../Algos/SubproblemManager.hpp"
#include "../../Cache/CacheBase.hpp"
#include "../../Eval/ProgressiveBarrier.hpp"
#include "../../Output/OutputQueue.hpp"
#include "../../Util/fileutils.hpp"
#ifdef TIME_STATS
#include "../../Util/Clock.hpp"
#endif

NOMAD::UserSearchMethodCbFunc NOMAD::Mads::_cbUserSearchMethod = [](const Step& step, EvalPointSet & trialPoints)->bool{ return true;};
NOMAD::UserSearchMethodCbFunc NOMAD::Mads::_cbUserSearchMethod_2 = [](const Step& step, EvalPointSet & trialPoints)->bool{ return true;};
NOMAD::UserMethodEndCbFunc NOMAD::Mads::_cbUserSearchMethodEnd = [](const Step& step)->bool{ return true;};

NOMAD::UserPollMethodCbFunc NOMAD::Mads::_cbUserPollMethod = [](const Step& step, std::list<Direction> & dir , const size_t n)->bool{ return true;};
NOMAD::UserPollMethodCbFunc NOMAD::Mads::_cbUserFreePollMethod = [](const Step& step, std::list<Direction> & dir, const size_t n)->bool{ return true;};
NOMAD::UserMethodEndCbFunc NOMAD::Mads::_cbUserFreePollMethodEnd = [](const Step& step)->bool{ return true;};

void NOMAD::Mads::init(bool barrierInitializedFromCache)
{
    setStepType(NOMAD::StepType::ALGORITHM_MADS);

    // Instantiate Mads initialization class
    _initialization = std::make_unique<NOMAD::MadsInitialization>( this , barrierInitializedFromCache);

    // We can accept Mads with more than one objective when doing a PhaseOneSearch of DMultiMads optimization.
    if (!_runParams->getAttributeValue<bool>("DMULTIMADS_OPTIMIZATION") && NOMAD::Algorithm::getNbObj() > 1)
    {
        throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Mads solves single objective problems. To handle several objectives please use DMultiMads: DMULTIMADS_OPTIMIZATION yes");
    }

}


NOMAD::ArrayOfPoint NOMAD::Mads::suggest()
{

    auto mesh = std::make_shared<NOMAD::GMesh>(_pbParams,_runParams);

    // ChT TODO: compute type must depend on h_norm. Eval type can be surrogate!
    FHComputeTypeS computeType /* default initializer*/;

    auto barrier = std::make_shared<NOMAD::ProgressiveBarrier>(NOMAD::INF,
                                                     NOMAD::SubproblemManager::getInstance()->getSubFixedVariable(this),
                                                     NOMAD::EvalType::BB, computeType,
                                                     std::vector<NOMAD::EvalPoint>(),
                                                     true /* Barrier must be initialized from cache, no x0 provided */);

    NOMAD::MadsMegaIteration megaIteration(this, 1, barrier, mesh, NOMAD::SuccessType::UNDEFINED);

    OUTPUT_INFO_START
    AddOutputInfo("Mega Iteration generated:");
    AddOutputInfo(megaIteration.getName());
    OUTPUT_INFO_END

    return megaIteration.suggest();

}


void NOMAD::Mads::observe(const std::vector<NOMAD::EvalPoint>& evalPointList)
{

    auto mesh = std::make_shared<NOMAD::GMesh>(_pbParams, _runParams);
    mesh->setEnforceSanityChecks(false);
    mesh->setDeltas(_pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("INITIAL_MESH_SIZE"),
                    _pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("INITIAL_FRAME_SIZE"));
    OUTPUT_DEBUG_START
    AddOutputDebug("Delta frame size: " + mesh->getDeltaFrameSize().display());
    AddOutputDebug("Delta mesh size:  " + mesh->getdeltaMeshSize().display());
    OUTPUT_DEBUG_END
    // Create progressive barrier from current points in cache.
    auto n = _pbParams->getAttributeValue<size_t>("DIMENSION");
    auto hMax = _runParams->getAttributeValue<NOMAD::Double>("H_MAX_0");
    auto hNormType = _runParams->getAttributeValue<NOMAD::HNormType>("H_NORM");

    // ChT. TODO check what happen for DMULTI_COMBINE_F.
    FHComputeTypeS computeType; // Default struct initializer is used
    computeType.hNormType = hNormType;
    std::shared_ptr<NOMAD::ProgressiveBarrier> barrier;
    if (0 == NOMAD::CacheBase::getInstance()->size())
    {
        // No points in cache: Create it solely from evalPointList.
        barrier = std::make_shared<NOMAD::ProgressiveBarrier>(hMax, NOMAD::Point(n),
                                                              NOMAD::EvalType::BB,
                                                              computeType,
                                                              evalPointList);
    }
    else
    {
        // Constructor will create progressive barrier from cache points.
        barrier = std::make_shared<NOMAD::ProgressiveBarrier>(hMax, NOMAD::Point(n),
                                                              NOMAD::EvalType::BB,
                                                              computeType);
    }


    NOMAD::MadsMegaIteration megaIteration(this, 0, barrier, mesh, NOMAD::SuccessType::UNDEFINED);

    OUTPUT_INFO_START
    AddOutputInfo("Mega Iteration generated: ");
    AddOutputInfo(megaIteration.getName());
    OUTPUT_INFO_END

    megaIteration.observe(evalPointList);

    OUTPUT_DEBUG_START
    AddOutputDebug("Delta frame size: " + mesh->getDeltaFrameSize().display());
    AddOutputDebug("Delta mesh size:  " + mesh->getdeltaMeshSize().display());
    OUTPUT_DEBUG_END

    // Mesh has been modified by observe; update mesh parameter.
    _pbParams->setAttributeValue("INITIAL_FRAME_SIZE", mesh->getDeltaFrameSize());
    _pbParams->checkAndComply();
    _runParams->setAttributeValue("H_MAX_0", barrier->getHMax());
    _runParams->checkAndComply(nullptr, _pbParams); // nullptr: We do not have access to EvaluatorControlParameters in this case.
}


bool NOMAD::Mads::runImp()
{
    size_t k = 0;   // Iteration number (incremented at start)

    NOMAD::SuccessType megaIterSuccess = NOMAD::SuccessType::UNDEFINED;

    if (!_termination->terminate(k))
    {
        std::shared_ptr<NOMAD::MeshBase> mesh;
        std::shared_ptr<NOMAD::BarrierBase> barrier;

        if (nullptr != _refMegaIteration)
        {
            // Case hot restart
            k       = _refMegaIteration->getK();
            barrier = _refMegaIteration->getBarrier();

            // Downcast from MegaIteration to MadsMegaIteration
            mesh    = (std::dynamic_pointer_cast<NOMAD::MadsMegaIteration> (_refMegaIteration ))->getMesh();
            megaIterSuccess = _refMegaIteration->getSuccessType();
            _success = megaIterSuccess;
        }
        else
        {
            mesh = dynamic_cast<NOMAD::MadsInitialization*>(_initialization.get())->getMesh();
            barrier = _initialization->getBarrier();
        }

        // Mads member _refMegaIteration is used for hot restart (read and write),
        // as well as to keep values used in Mads::end(), and may be used for _termination.
        // Update it here.
        _refMegaIteration = std::make_shared<NOMAD::MadsMegaIteration>(this, k, barrier, mesh, megaIterSuccess);

        // Create a MegaIteration for looping: manage multiple iterations on different
        // meshes and with different frame centers at the same time.
        NOMAD::MadsMegaIteration megaIteration(this, k, barrier, mesh, megaIterSuccess);
        while (!_termination->terminate(k))
        {

            megaIteration.start();
            megaIteration.run();
            megaIteration.end();

            // Counter is incremented when calling mega iteration end()
            k       = megaIteration.getK();

            if (!_algoSuccessful && megaIteration.getSuccessType() >= NOMAD::SuccessType::FULL_SUCCESS)
            {
                _algoSuccessful = true;
            }

            if (getUserInterrupt())
            {
                hotRestartOnUserInterrupt();
            }
        }
    }

    _termination->start();
    _termination->run();
    _termination->end();

    return _algoSuccessful;
}


void NOMAD::Mads::hotRestartOnUserInterrupt()
{
    if (_stopReasons->checkTerminate())
    {
        return;
    }
#ifdef TIME_STATS
    if (isRootAlgo())
    {
        _totalCPUAlgoTime += NOMAD::Clock::getCPUTime() - _startTime;
    }
#endif // TIME_STATS
    hotRestartBeginHelper();

    // Reset mesh because parameters have changed.
    std::stringstream ss;
    const NOMAD::Iteration* iteration = getParentOfType<NOMAD::Iteration*>();
    if (nullptr != iteration)
    {
        auto mesh = getIterationMesh();
        ss << *mesh;
        // Reset pointer
        mesh.reset();

        mesh = std::make_shared<NOMAD::GMesh>(iteration->getPbParams(),iteration->getRunParams());
        // Get old mesh values
        ss >> *mesh;
    }

    hotRestartEndHelper();
#ifdef TIME_STATS
    if (isRootAlgo())
    {
        _startTime = NOMAD::Clock::getCPUTime();
    }
#endif // TIME_STATS
}


void NOMAD::Mads::readInformationForHotRestart()
{
    // Restart from where we were before.
    // For this, we need to read some files.
    // Note: Cache file is treated independently of hot restart file.

    if (_runParams->getAttributeValue<bool>("HOT_RESTART_READ_FILES"))
    {
        // Verify the files exist and are readable.
        std::string hotRestartFile = _runParams->getAttributeValue<std::string>("HOT_RESTART_FILE");
        if (NOMAD::checkReadFile(hotRestartFile))
        {
            std::string s = "Read hot restart file " + hotRestartFile;
            NOMAD::OutputQueue::Add(s, NOMAD::OutputLevel::LEVEL_NORMAL);

            // Create a GMesh and an MadsMegaIteration with default values, to be filled
            // by istream is.
            // NOTE: Working in full dimension
            auto barrier = std::make_shared<NOMAD::ProgressiveBarrier>(NOMAD::INF, NOMAD::Point(_pbParams->getAttributeValue<size_t>("DIMENSION")), NOMAD::EvalType::BB);

            std::shared_ptr<NOMAD::MeshBase> mesh = std::make_shared<NOMAD::GMesh>(_pbParams,_runParams);

            _refMegaIteration = std::make_shared<NOMAD::MadsMegaIteration>(this, 0, barrier, mesh, NOMAD::SuccessType::UNDEFINED);

            // Here we use Algorithm::operator>>
            NOMAD::read<NOMAD::Mads>(*this, hotRestartFile);
        }
    }
}

void NOMAD::Mads::addCallback(const NOMAD::CallbackType& callbackType,
                              const NOMAD::UserMethodEndCbFunc& userMethodCbFunc) const
{
    switch (callbackType)
    {
        case NOMAD::CallbackType::USER_METHOD_SEARCH_END:
            if (!_hasUserSearchMethod)
            {
                throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Calling to add a user search callback for post evaluation fails. A NOMAD::CallbackType::USER_METHOD_SEARCH callback must be added first.");
            }
            _cbUserSearchMethodEnd = userMethodCbFunc;
            break;
        case NOMAD::CallbackType::USER_METHOD_FREE_POLL_END:
            if (!_hasUserFreePollMethod)
            {
                throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Calling to add a free user poll callback post eval has failed. A NOMAD::CallbackType::USER_METHOD_FREE_POLL callback must be added first.");
            }
            _cbUserFreePollMethodEnd = userMethodCbFunc;
            break;
        default:
            throw NOMAD::Exception(__FILE__,__LINE__,"Callback type not supported.");
            break;
    }

}

void NOMAD::Mads::addCallback(const NOMAD::CallbackType& callbackType,
                              const NOMAD::UserSearchMethodCbFunc& userMethodCbFunc)
{

    auto us = _runParams->getAttributeValue<bool>("USER_SEARCH");
    switch (callbackType)
    {
        case NOMAD::CallbackType::USER_METHOD_SEARCH:
            if (!us)
            {
                throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Calling to add a user search method callback fails because USER_SEARCH parameter has not been set to True.");
            }
            _cbUserSearchMethod = userMethodCbFunc;
            _hasUserSearchMethod = true;  // This flag is used to enable user search method
            break;
        case NOMAD::CallbackType::USER_METHOD_SEARCH_2:
            if (!us)
            {
                throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Calling to add a user search (2) method callback fails because USER_SEARCH parameter has not been set to True.");
            }
            _cbUserSearchMethod_2 = userMethodCbFunc;
            _hasUserSearchMethod = true;  // This flag is used to enable user search method
            break;
        case NOMAD::CallbackType::USER_METHOD_POLL:
        case NOMAD::CallbackType::USER_METHOD_FREE_POLL:
            throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Calling to add user search method callback but callback type is for USER_POLL.");
            break;
        default:
            throw NOMAD::Exception(__FILE__,__LINE__,"Callback type not supported.");
            break;

    }
}

void NOMAD::Mads::addCallback(const NOMAD::CallbackType& callbackType,
                              const NOMAD::UserPollMethodCbFunc& userMethodCbFunc)
{
    auto dt = _runParams->getAttributeValue<NOMAD::DirectionTypeList>("DIRECTION_TYPE");
    switch (callbackType)
    {
        case NOMAD::CallbackType::USER_METHOD_SEARCH:
        case NOMAD::CallbackType::USER_METHOD_SEARCH_2:
            throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Calling to add user poll method callback but callback type is for USER_SEARCH.");
            break;
        case NOMAD::CallbackType::USER_METHOD_POLL:
            _cbUserPollMethod = userMethodCbFunc;
            if ( std::find(dt.begin(),dt.end(),NOMAD::DirectionType::USER_POLL) == dt.end() )
            {
                throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Calling to add user poll method callback but DIRECTION_TYPE USER_POLL has not been set.");
            }
            _hasUserPollMethod = true; // This flag is used to enable user poll method
            break;
        case NOMAD::CallbackType::USER_METHOD_FREE_POLL:
            _cbUserFreePollMethod = userMethodCbFunc;
            if ( std::find(dt.begin(),dt.end(),NOMAD::DirectionType::USER_FREE_POLL) == dt.end() )
            {
                throw NOMAD::InvalidParameter(__FILE__,__LINE__,"Calling to add user poll method callback but DIRECTION_TYPE USER_FREE_POLL has not been set.");
            }
            _hasUserFreePollMethod = true; // This flag is used to enable user free poll method
            break;
        default:
            throw NOMAD::Exception(__FILE__,__LINE__,"Callback type not supported.");
            break;
    }
}

bool NOMAD::Mads::runCallback(const NOMAD::CallbackType & callbackType,
                              const NOMAD::Step& step,
                              std::list<Direction> & dirs,
                              const size_t n) const
{

    switch(callbackType)
    {
        case NOMAD::CallbackType::USER_METHOD_POLL:
            return _cbUserPollMethod(step, dirs, n);
            break;
        case NOMAD::CallbackType::USER_METHOD_FREE_POLL:
            return _cbUserFreePollMethod(step, dirs, n);
            break;
        case NOMAD::CallbackType::USER_METHOD_SEARCH:
        case NOMAD::CallbackType::USER_METHOD_SEARCH_2:
            throw NOMAD::Exception(__FILE__,__LINE__,"Cannot run user search callback type to get directions.");
            break;
        default:
            return false;
    }
}

bool NOMAD::Mads::runCallback(const NOMAD::CallbackType & callbackType,
                              const NOMAD::Step& step,
                              NOMAD::EvalPointSet & trialPoints) const
{

    switch(callbackType)
    {
        case NOMAD::CallbackType::USER_METHOD_POLL:
        case NOMAD::CallbackType::USER_METHOD_FREE_POLL:
            throw NOMAD::Exception(__FILE__,__LINE__,"Cannot run user poll callback type to get trial points.");
            break;
        case NOMAD::CallbackType::USER_METHOD_SEARCH:
            return _cbUserSearchMethod(step, trialPoints);
            break;
        case NOMAD::CallbackType::USER_METHOD_SEARCH_2:
            return _cbUserSearchMethod_2(step, trialPoints);
            break;
        default:
            return false;
    }
}


bool NOMAD::Mads::runCallback(const NOMAD::CallbackType & callbackType,
                              const NOMAD::Step& step) const
{

    switch(callbackType)
    {
        case NOMAD::CallbackType::USER_METHOD_FREE_POLL_END:
            return _cbUserFreePollMethodEnd(step);
            break;
        case NOMAD::CallbackType::USER_METHOD_SEARCH_END:
            return _cbUserSearchMethodEnd(step);
            break;
        default:
            return false;
    }
}
