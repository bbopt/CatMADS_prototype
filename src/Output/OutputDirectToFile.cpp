
#include <fstream>
#include "../Output/OutputDirectToFile.hpp"
#include "../Util/Exception.hpp"

// Static members initialization
#ifdef _OPENMP
omp_lock_t NOMAD::OutputDirectToFile::_s_output_lock;
#endif // _OPENMP

std::unique_ptr<NOMAD::OutputDirectToFile> NOMAD::OutputDirectToFile::_single(nullptr);

bool NOMAD::OutputDirectToFile::_hasBeenInitialized = false;

// Private constructor
NOMAD::OutputDirectToFile::OutputDirectToFile()
  : _outputSize(0),
    _outputFileFormat(DisplayStatsTypeList("SOL BBO")),
    _solutionFile(),
    _historyFile(),
    _enabledSolutionFile(true)
{
}


// Destructor
NOMAD::OutputDirectToFile::~OutputDirectToFile()
{

#ifdef _OPENMP
    omp_destroy_lock(&_s_output_lock);
#endif // _OPENMP
    if (!_historyFile.empty())
    {
        _historyStream.close();
    }
    if (!_solutionFile.empty())
    {
        _solutionStream.close();
    }

}


// Access to singleton
std::unique_ptr<NOMAD::OutputDirectToFile>& NOMAD::OutputDirectToFile::getInstance()
{
#ifdef _OPENMP
    #pragma omp critical(initODTFLock)
    {
#endif // _OPENMP
        if (nullptr == _single)
        {
#ifdef _OPENMP
            omp_init_lock(&_s_output_lock);
#endif // _OPENMP
            _single = std::unique_ptr<OutputDirectToFile> (new OutputDirectToFile());
#ifdef _OPENMP
        }
#endif // _OPENMP
    } // end of critical section

    return _single;
}

// Initialize output parameters.
void NOMAD::OutputDirectToFile::init(const std::shared_ptr<NOMAD::DisplayParameters>& params)
{

    if (nullptr == params)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "OutputDirectToFile::init: Display Parameters are NULL");
    }

    // Read from the current DisplayParameters
    std::string historyFileTmp = params->getAttributeValue<std::string>("HISTORY_FILE");

    // Check conflict with previous history file if this has already been initialized
    if(_hasBeenInitialized && ! _historyFile.empty() && ! historyFileTmp.empty() && historyFileTmp == _historyFile )
    {
       throw NOMAD::Exception(__FILE__, __LINE__, "OutputQueue::initParameters: Initialization cannot be performed more than once with the same history_file. The history file will be overwritten! Call OutputDirectToFile::getInstance()->reset() to allow this.");
    }
    _historyFile = historyFileTmp;
    _solutionFile = params->getAttributeValue<std::string>("SOLUTION_FILE");
    _outputSize = params->getAttributeValue<NOMAD::ArrayOfDouble>("SOL_FORMAT").size();

    initHistoryFile();

    _hasBeenInitialized = true;
}


bool NOMAD::OutputDirectToFile::goodToWrite() const
{
    return ( !_historyFile.empty() || !_solutionFile.empty());
}

void NOMAD::OutputDirectToFile::initHistoryFile()
{
    if (!_historyFile.empty())
    {
        // Open history file and clear it (trunc)
        _historyStream.close();
        _historyStream.open(_historyFile.c_str(), std::ofstream::out | std::ios::trunc);
        if (_historyStream.fail())
        {
            std::cout << "Warning: could not open history file " << _historyFile << std::endl;
        }
        _historyStream.setf(std::ios::fixed);
        // Set full precision on history file.
        _historyStream.precision(NOMAD::DISPLAY_PRECISION_FULL);
    }
}


void NOMAD::OutputDirectToFile::write(const NOMAD::StatsInfo &info, bool writeInSolutionFile, bool writeInHistoryFile, bool appendInSolutionFile )
{
    // Early out
    if (_historyFile.empty() && _solutionFile.empty())
    {
        return;
    }

    if (0==_outputSize)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "OutputDirectToFile: output size is null");
    }

#ifdef _OPENMP
    // Lock queue before writing in file
    omp_set_lock(&_s_output_lock);
#endif

    NOMAD::ArrayOfDouble solFormatStats(_outputSize, NOMAD::DISPLAY_PRECISION_FULL);

    // Add information in history file.
    if (writeInHistoryFile)
    {
        _historyStream << info.display(_outputFileFormat, solFormatStats, 0, 0, false, false) << std::endl;
    }

    // Add information in solution file
    if (writeInSolutionFile && _enabledSolutionFile && !_solutionFile.empty())
    {
        // Open solution file and clear it if needed
        _solutionStream.close();
        
        if (appendInSolutionFile)
        {
            _solutionStream.open(_solutionFile.c_str(), std::ofstream::out | std::ios::app);
        }
        else
        {
            _solutionStream.open(_solutionFile.c_str(), std::ofstream::out | std::ios::trunc);
        }
        if (_solutionStream.fail())
        {
            std::cout << "Warning: could not open solution file " << _solutionFile << std::endl;
        }
        _solutionStream.setf(std::ios::fixed);
        // Set full precision on solution file.
        _solutionStream.precision(NOMAD::DISPLAY_PRECISION_FULL);

        _solutionStream << info.display(_outputFileFormat, solFormatStats, 0, 0, false, false) << std::endl;
        _solutionStream.close();
    }

#ifdef _OPENMP
    omp_unset_lock(&_s_output_lock);
#endif // _OPENMP

}



