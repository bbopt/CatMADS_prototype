#include "../Eval/Evaluator.hpp"
#include "../Output/OutputQueue.hpp"
#include "../Util/fileutils.hpp"
#include <cstdio>  // For popen
#include <fstream>  // For ofstream
#ifndef _WIN32
#include <unistd.h> // for getpid
#else
#include <process.h>
#define getpid _getpid
#define popen  _popen
#define pclose _pclose
#endif

// Initialize tmp files. NOTE: static variables for those are not working when building for Windows VS.
/// Vector is used to store one file per thread (thread for eval and algo thread).
std::vector<std::string> _tmpFiles = std::vector<std::string>();
std::vector<std::string> _tmpOutFilesWithoutRedirection = std::vector<std::string>();
std::vector<std::string> _tmpLogFilesWithoutRedirection = std::vector<std::string>();
size_t _tmpFilesForParEval = 1;  // number of tmp files for parallel eval. Total number depends on number of main threads.


// Initialize static var
bool NOMAD::Evaluator::_bbRedirection = true;

namespace {
    // the cleanup of temporary files at program shutdown needs to remain with this
    // translation unit to guarantee the correct destruction order
    struct TmpFilesCleanup {
        ~TmpFilesCleanup() { NOMAD::Evaluator::removeTmpFiles(); }
    } _TmpFilesCleanup;
}

//
// Constructor
//
// NOTE: The full path to BB_EXE is added during check
NOMAD::Evaluator::Evaluator(
                    const std::shared_ptr<NOMAD::EvalParameters> &evalParams,
                    const NOMAD::EvalType evalType,
                    const NOMAD::EvalXDefined evalXDefined)
  : _evalParams(evalParams),
    _evalXDefined(evalXDefined),
    _evalType(evalType),
    _bbOutputTypeList(_evalParams->getAttributeValue<NOMAD::BBOutputTypeList>("BB_OUTPUT_TYPE")),
    _bbEvalFormat(_evalParams->getAttributeValue<NOMAD::ArrayOfDouble>("BB_EVAL_FORMAT"))
{
    init();
}

void NOMAD::Evaluator::init()
{

    if (EvalXDefined::USE_BB_EVAL == _evalXDefined)
    {
        _bbRedirection = _evalParams->getAttributeValue<bool>("BB_REDIRECTION");
        switch (_evalType)
        {
            case NOMAD::EvalType::BB:
                _bbExe = _evalParams->getAttributeValue<std::string>("BB_EXE");
                break;
            case NOMAD::EvalType::SURROGATE:
                _bbExe = _evalParams->getAttributeValue<std::string>("SURROGATE_EXE");
                break;
            default:
                std::string err = "Evaluator: No executable supported for EvalType ";
                err += NOMAD::evalTypeToString(_evalType);
                throw NOMAD::Exception(__FILE__,__LINE__,err);
        }
    }
}

NOMAD::Evaluator::~Evaluator() = default;


void NOMAD::Evaluator::initializeTmpFiles(const std::string& tmpDir, const int & nbThreadsForParallelEval)
{
    // Initialize tmp files for Evaluators
    int nbThreads = 1;
#ifdef _OPENMP
    if (nbThreadsForParallelEval > 1)
    {
        nbThreads = nbThreadsForParallelEval;
    }
#endif
    _tmpFilesForParEval = static_cast<size_t>(nbThreads); // Store the number of threads for parallel eval.

    // Case where no tmp files exist
    if (_tmpFiles.empty())
    {

        std::string tmppath = tmpDir;
        NOMAD::ensureDirPath(tmppath);
        // Use the pid in the file name in case two nomad run at the same time.
        int pid = getpid();
        // Create a temporary file fo blackbox input. One for each thread number,
        // for each nomad pid. Add the file names to _tmpFiles.
        for (auto threadNum = 0; threadNum < nbThreads; threadNum++)
        {
            std::string tmpfilestr = tmppath + "nomadtmp." + std::to_string(pid) + "." + std::to_string(threadNum);
            _tmpFiles.push_back(tmpfilestr);
            if (! _bbRedirection)
            {
                std::string tmpfilestrOut = tmpfilestr + ".output";
                _tmpOutFilesWithoutRedirection.push_back(tmpfilestrOut);
                std::string tmpfilestrLog = tmpfilestr + ".tmplog";
                _tmpLogFilesWithoutRedirection.push_back(tmpfilestrLog);
            }
        }
    }
    else
    {
        // Some tmp files already exist. Use them as template
        size_t posPid = _tmpFiles[0].find_last_of('.');
        size_t len = _tmpFiles[0].length();

        if (len == 0 || posPid==std::string::npos)
        {
            throw NOMAD::Exception(__FILE__, __LINE__, "Evaluator: Tmp files should have been defined.");
        }
        for (auto threadNum = 0; threadNum < nbThreads; threadNum++)
        {
            std::string tmpfilestr = _tmpFiles[0];
            tmpfilestr.replace(posPid+1,len, std::to_string(threadNum+_tmpFiles.size()));
            _tmpFiles.push_back(tmpfilestr);

            if (! _bbRedirection)
            {
                std::string tmpfilestrOut = tmpfilestr + ".output";
                _tmpOutFilesWithoutRedirection.push_back(tmpfilestrOut);
                std::string tmpfilestrLog = tmpfilestr + ".tmplog";
                _tmpLogFilesWithoutRedirection.push_back(tmpfilestrLog);
            }
        }
    }
}



void NOMAD::Evaluator::removeTmpFiles()
{
    // Remove all temporary files, so that they do not linger around.
    auto nbThreads = _tmpFiles.size();
    for (size_t i = 0; i < nbThreads; i++)
    {
        remove(_tmpFiles[i].c_str());
        if (!_bbRedirection)
        {
            remove(_tmpOutFilesWithoutRedirection[i].c_str());
            remove(_tmpLogFilesWithoutRedirection[i].c_str());
        }
    }
    _tmpFiles.clear();
    _tmpOutFilesWithoutRedirection.clear();
    _tmpLogFilesWithoutRedirection.clear();

}


// Default eval_x: System call to a black box that was provided
// via parameter BB_EXE and set through Evaluator::setBBExe().
bool NOMAD::Evaluator::eval_x(NOMAD::EvalPoint &x,
                              const NOMAD::Double& hMax,
                              bool &countEval) const
{

    if (NOMAD::EvalXDefined::UNDEFINED == _evalXDefined)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Evaluator: this is a fake evaluator with no evaluation capability.");
    }

    // The user might have defined his own eval_x() for NOMAD::EvalPoint.
    // In the NOMAD code, we do not use this method.
    //
    // Implemented to be used by the Runner. In the case of the Runner,
    // eval_x is redefined. When batch mode is used (for instance for
    // Styrene), this eval_x is called. So in fact we really want to
    // use the executable defined by BB_EXE.

    _evalXDefined = NOMAD::EvalXDefined::USE_BB_EVAL;

    // Create a block of one point and evaluate it.
    NOMAD::Block block;
    std::shared_ptr<NOMAD::EvalPoint> epp = std::make_shared<NOMAD::EvalPoint>(x);
    block.push_back(epp);

    std::vector<bool> countEvalVector(1, countEval);
    std::vector<bool> evalOkVector(1, false);

    // Call eval_block
    evalOkVector = eval_block(block, hMax, countEvalVector);

    // Update x and countEval
    x = *epp;
    countEval = countEvalVector[0];

    return evalOkVector[0];
}


// Default eval_block: for block
// This is used even for blocks of 1 point.
// If we never go through this eval_block(),
// it means that eval_block was redefined by the user,
// using library mode.
std::vector<bool> NOMAD::Evaluator::eval_block(NOMAD::Block &block,
                                               const NOMAD::Double &hMax,
                                               std::vector<bool> &countEval) const
{
    std::vector<bool> evalOk(block.size(), false);
    countEval.resize(block.size(), false);

    // Verify there is at least one point to evaluate
    if (block.empty())
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Evaluator: eval_block called with an empty block");
    }

    if (NOMAD::EvalXDefined::UNDEFINED == _evalXDefined)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Evaluator: this is a fake evaluator with no evaluation capability.");
    }

    // Verify all points are completely defined
    for (auto& it : block)
    {
        if (!it->isComplete())
        {
            throw NOMAD::Exception(__FILE__, __LINE__, "Evaluator: Incomplete point " + it->display());
        }
    }

    // Start evaluation
    for (auto& it : block)
    {
        // Debugging. EVAL should already be IN_PROGRESS or EVAL_WAIT.
        // EVAL_WAIT -> no need to evaluate, it is already in progress by another thread.
        if (NOMAD::EvalStatusType::EVAL_IN_PROGRESS != it->getEvalStatus(_evalType) &&
            NOMAD::EvalStatusType::EVAL_WAIT != it->getEvalStatus(_evalType))
        {
#ifdef _OPENMP
            #pragma omp critical(warningEvalX)
#endif
            {
                throw NOMAD::Exception(__FILE__, __LINE__, "EVAL should already be IN_PROGRESS for point" + it->display());
            }
        }
    }

    if (NOMAD::EvalXDefined::EVAL_BLOCK_DEFINED_BY_USER == _evalXDefined)
    {
        // First time that eval_block() is called.
        // Obviously, eval_block() was not redefined by user.
        // If the blackbox is external, USE_BB_EVAL is already set by the
        // constructor. Hence, eval_x() is defined by user.
        _evalXDefined = NOMAD::EvalXDefined::EVAL_X_DEFINED_BY_USER;
    }

    if (NOMAD::EvalXDefined::USE_BB_EVAL == _evalXDefined)
    {
        // EVAL_WAIT is managed inside the function
        evalOk = evalXBBExe(block, hMax, countEval);
    }
    else if (NOMAD::EvalXDefined::EVAL_X_DEFINED_BY_USER == _evalXDefined)
    {
        for (size_t index = 0; index < block.size(); index++)
        {
            bool countEval1 = false;

            // EVAL_WAIT -> no need to evaluate, it is already in progress by another thread.
            if ( NOMAD::EvalStatusType::EVAL_WAIT == block[index]->getEvalStatus(_evalType) )
            {
                evalOk[index] = true;
            }
            else
            {
                evalOk[index] = eval_x(*block[index], hMax, countEval1);
            }
            countEval[index] = countEval1;
        }
    }
    else
    {
        std::string s = "Error: This value of EvalXDefined is not processed: ";
        s += std::to_string((int)_evalXDefined);
        throw NOMAD::Exception(__FILE__, __LINE__, s);
    }

    return evalOk;
}


// eval_x() called in batch. Use system command and use the blackbox executable
// provided by parameter BB_EXE.
std::vector<bool> NOMAD::Evaluator::evalXBBExe(NOMAD::Block &block,
                                               const NOMAD::Double &hMax,
                                               std::vector<bool> &countEval) const
{
    std::vector<bool> evalOk(block.size(), false);

    // At this point, we are for sure in batch mode.
    // Verify blackbox/surrogate executable is available and executable.
    if (_bbExe.empty())
    {
        std::string err = "Evaluator: No ";
        err += (_evalType == NOMAD::EvalType::BB) ? "blackbox ": "surrogate ";
        err += "executable defined.";
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }

    const int mainThreadNum = block[0]->getThreadAlgo();

    const size_t indexTmpFile = mainThreadNum*_tmpFilesForParEval + NOMAD::getThreadNum();

    // Write a temp file for x0 and give that file as argument to bbExe.
    if (indexTmpFile >= _tmpFiles.size())
    {
        std::cout << "Error: Evaluator: No enough temp file available." << std::endl;
        // Ugly early return
        return evalOk;
    }
    const std::string& tmpfile = _tmpFiles[indexTmpFile];
    std::string tmpoutfile, tmplogfile;
    if (! _bbRedirection)
    {
        tmpoutfile = _tmpOutFilesWithoutRedirection[indexTmpFile];
        tmplogfile = _tmpLogFilesWithoutRedirection[indexTmpFile];
    }



    // System command
    std::ofstream xfile;
    // Open xfile and clear it (trunc)
    xfile.open(tmpfile.c_str(), std::ofstream::trunc);
    if (xfile.fail())
    {
        for (auto& it : block)
        {
            it->setEvalStatus(NOMAD::EvalStatusType::EVAL_ERROR, _evalType);
            std::cout << "Error writing point " << it->display() << " to temporary file \"" << tmpfile << "\"" << std::endl;
        }
        // Ugly early return
        return evalOk;
    }

    bool flagHasEvalInProgress = false;
    for (auto& it : block)
    {
        const std::shared_ptr<NOMAD::EvalPoint>& x = it;
        for (size_t i = 0; i < x->size(); i++)
        {
            if ( x->getEvalStatus(_evalType) == NOMAD::EvalStatusType::EVAL_IN_PROGRESS)
            {
                if (i != 0)
                {
                    xfile << " ";
                }
                xfile << (*x)[i].display(static_cast<int>(_bbEvalFormat[i].todouble()));
                flagHasEvalInProgress = true;
            }
        }
        xfile << std::endl;
    }
    xfile.close();

    // No need to launch bb command in that case.
    if (!flagHasEvalInProgress)
    {
        std::fill(countEval.begin(), countEval.end(), false); // no eval should be counted
        return std::vector<bool>(countEval.size(),true); // no eval is ok
    }

    std::string cmd = _bbExe + " " + tmpfile;
    std::string s;
    OUTPUT_DEBUG_START
    s = "System command: " + cmd;
    NOMAD::OutputQueue::Add(s, NOMAD::OutputLevel::LEVEL_DEBUGDEBUG);
    OUTPUT_DEBUG_END

    // Stream for output file when bb manages output files (no redirection)
    std::ifstream finWithoutRedirection;

    FILE *fresult = popen(cmd.c_str(), "r");
    if (!fresult)
    {
        // Something went wrong with the evaluation.
        // Point could be re-submitted.
        for (auto& it : block)
        {
            it->setEvalStatus(NOMAD::EvalStatusType::EVAL_ERROR, _evalType);
#ifdef _OPENMP
            #pragma omp critical(warningEvalX)
#endif
            {
                std::cout << "Warning: Evaluation error with point " << it->display() << std::endl;
            }
        }
    }
    else
    {

        if (!_bbRedirection)
        {
            char buffer[BUFFER_SIZE];
            char *outputline = nullptr;

            outputline = fgets(buffer, sizeof(buffer), fresult);
            if ( nullptr != outputline) // Something to log
            {

                // BB log file
                std::ofstream foutLogWithoutRedirection;
                foutLogWithoutRedirection.open(tmplogfile.c_str(), std::ofstream::app );
                if (foutLogWithoutRedirection.fail())
                {
                    std::cout << "Warning: fail to create log file for blackbox output (no redirection mode) \"" << tmplogfile << "\"" << std::endl;
                }
                else
                {
                    // Get blackbox standard and error outputs (not yet bb_output) and write into log file.
                    // If something is output then do at least once with the base message. Otherwise, no file is created.
                    foutLogWithoutRedirection << "####### Blackbox evaluation output log (no redirection)  ######## " <<std::endl;
                    std::string out;
                    do
                    {
                        out = outputline;
                        // delete trailing '\n'
                        out.erase(out.size() - 1);
                        foutLogWithoutRedirection << out << std::endl;
                        outputline = fgets(buffer, sizeof(buffer), fresult);
                    }
                    while (nullptr != outputline);
                }
                foutLogWithoutRedirection.close();
            }

            // Test bb outputs file
            finWithoutRedirection.open( tmpoutfile.c_str() );
            if (!finWithoutRedirection.is_open())
            {
                for (auto& it : block)
                {
                    it->setEvalStatus(NOMAD::EvalStatusType::EVAL_ERROR, _evalType);
#ifdef _OPENMP
#pragma omp critical(warningEvalX)
#endif
                    {
                        std::cout << "Warning: Cannot open output file " << tmpoutfile << " for point " << it->display() << std::endl;
                    }
                }
            }

            // Read bb outputs in file
            for (size_t index = 0; index < block.size(); index++)
            {
                const std::shared_ptr<NOMAD::EvalPoint>& x = block[index];

                // Only EVAL_IN_PROGRESS are managed.
                // EVAL_WAIT points are not evaluated
                if (x->getEvalStatus(_evalType) != NOMAD::EvalStatusType::EVAL_WAIT)
                {

                    std::string bbo;
                    std::getline(finWithoutRedirection, bbo);

                    // When several points are evaluated, an empty line is a failed evaluation
                    if (!bbo.empty())
                    {
                        // Process blackbox output
                        x->setBBO(bbo, _bbOutputTypeList, _evalType);
                        auto bbOutput = x->getEval(_evalType)->getBBOutput();

                        evalOk[index] = bbOutput.getEvalOk();
                        countEval[index] = bbOutput.getCountEval(_bbOutputTypeList);
                    }
                }
            }
            finWithoutRedirection.close();
        }
        else  // Nomad is handling bb redirection
        {
            for (size_t index = 0; index < block.size(); index++)
            {
                const std::shared_ptr<NOMAD::EvalPoint>& x = block[index];

                // Only EVAL_IN_PROGRESS are managed.
                // EVAL_WAIT points are not evaluated
                if (x->getEvalStatus(_evalType) != NOMAD::EvalStatusType::EVAL_WAIT)
                {

                    char buffer[BUFFER_SIZE];
                    char *outputLine = nullptr;

                    size_t nbTries=0;
                    while (nbTries < 20)
                    {
                        nbTries++;

                        outputLine = fgets(buffer, sizeof(buffer), fresult);

                        if( feof(fresult) )
                        { // c-stream eof detected. Output is empty, break the loop
                            x->setEvalStatus(NOMAD::EvalStatusType::EVAL_ERROR, _evalType);
                            s = "Warning: Evaluation error with point " + x->display();
                            s += ": output is empty. Let's count eval anyway (I).";
                            NOMAD::OutputQueue::Add(s, NOMAD::OutputLevel::LEVEL_WARNING);
                            countEval[index] = true;
                            break;
                        }

                        if (nullptr != outputLine)
                        {
                            // Evaluation succeeded. Get and process blackbox output.
                            std::string bbo(outputLine);
                            // delete trailing '\n'
                            bbo.erase(bbo.size() - 1);

                            // Process blackbox output
                            x->setBBO(bbo, _bbOutputTypeList, _evalType);
                            auto bbOutput = x->getEval(_evalType)->getBBOutput();

                            evalOk[index] = bbOutput.getEvalOk();
                            countEval[index] = bbOutput.getCountEval(_bbOutputTypeList);

                            break;
                        }
                    }
                    if( ! feof(fresult) && nullptr == outputLine )
                    {
                        // Something went wrong with the evaluation.
                        // Point could be re-submitted.
                        x->setEvalStatus(NOMAD::EvalStatusType::EVAL_ERROR, _evalType);
                        s = "Warning: Evaluation error with point " + x->display();
                        s += ": output is empty. Let's count eval anyway (II). Nb Tries to read output: " + std::to_string(nbTries) + ". You may consider increasing the number of tries in the code and rebuild." ;
                        NOMAD::OutputQueue::Add(s, NOMAD::OutputLevel::LEVEL_WARNING);
                        countEval[index] = true;
                    }
                }
            }
        }

        // Get exit status of the bb.exe. If it is not 0, there was an error.
        int exitStatus = pclose(fresult);

        size_t index = 0;   // used to update evalOk
        for (auto& it : block)
        {

            const std::shared_ptr<NOMAD::EvalPoint>& x = it;

            // Points with EVAL_WAIT status have been managed before. Their status should remain EVAL_WAIT.
            if ( NOMAD::EvalStatusType::EVAL_WAIT != x->getEvalStatus(_evalType) )
            {

                if (exitStatus)
                {
                    evalOk[index] = false;
                    // Test eval status to prevent multiple error display (see above)
                    if (x->getEvalStatus(_evalType) != NOMAD::EvalStatusType::EVAL_ERROR)
                    {
                        x->setEvalStatus(NOMAD::EvalStatusType::EVAL_ERROR, _evalType);
                        s = "Warning: Evaluator returned exit status ";
                        s += std::to_string(exitStatus);
                        s += " for point: " + x->getX()->NOMAD::Point::display();
                        NOMAD::OutputQueue::Add(s, NOMAD::OutputLevel::LEVEL_WARNING);
                    }
                }
                else if (!evalOk[index])
                {
                    x->setEvalStatus(NOMAD::EvalStatusType::EVAL_FAILED, _evalType);
                }
                else
                {
                    x->setEvalStatus(NOMAD::EvalStatusType::EVAL_OK, _evalType);
                }
            }

            index++;
        }
    }

    return evalOk;
}
