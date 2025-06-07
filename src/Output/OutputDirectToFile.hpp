#ifndef __NOMAD_4_5_OUTPUTDIRECTTOFILE__
#define __NOMAD_4_5_OUTPUTDIRECTTOFILE__

#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif // _OPENMP

#include "../Param/DisplayParameters.hpp"
#include "../Output/OutputInfo.hpp"
#include "../Output/StatsInfo.hpp"

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"

/// Direct output of info to file (history, solution,...).
/**
 The output is a singleton. Some info can be written into files. \n
 The format of output is fixed. The parameters (DisplayParameters) are attributes of the class provided by calling OutputDirectToFile::initParameters. New files to receive output must be  registered in this function.\n
 */
class DLL_UTIL_API OutputDirectToFile
{
private:
    /// Private constructor
    OutputDirectToFile();

public:
    // Destructor
     virtual ~OutputDirectToFile();

    /// Access to singleton
    static std::unique_ptr<OutputDirectToFile>& getInstance();

    /// Initialization of file names using display parameters
    void init(const std::shared_ptr<DisplayParameters>& params);

    /// When history and/or solution files are active, write info in solution and history file according to the flags
    void write(const StatsInfo& outInfo, bool writeInSolutionFile, bool writeInHistoryFile=true, bool appendInSolutionFile = false);

    static void Write(const StatsInfo & outInfo, bool writeInSolutionFile, bool writeInHistoryFile = true, bool appendInSolutionFile = false)
    {
        getInstance()->write(outInfo,writeInSolutionFile,writeInHistoryFile,appendInSolutionFile);
    }

    /// Good to write in history and/or solution files when the file names have been defined.
    bool goodToWrite() const;
    static bool GoodToWrite()
    {
        return getInstance()->goodToWrite();
    }

    void reset() { _hasBeenInitialized = false; } // Allow to reset history file and solution file. Existing files will be overwritten

    /// Not used now
    void setOutputFileFormat(const DisplayStatsTypeList& outputFileFormat)
    {
        _outputFileFormat = outputFileFormat;
    }
    const DisplayStatsTypeList& getOutputFileFormat() const { return _outputFileFormat; }

    /// Used to enable solution file (requires a solution file name)
    void enableSolutionFile() { _enabledSolutionFile = true; }

    /// Disable solution file (temporarily, for example during PhaseOne)
    void disableSolutionFile() { _enabledSolutionFile = false; }

#define OUTPUT_DIRECTTOFILE_START if (OutputDirectToFile::GoodToWrite()) {
#define OUTPUT_DIRECTTOFILE_END }

private:
#ifdef _OPENMP
    // Acquire lock before write.
    // NOTE It does not seem relevant for the lock to be static,
    // because OutputDirectToFile is a singleton anyway. If staticity causes problems,
    // we could remove the static keyword.
    static omp_lock_t  _s_output_lock;
#endif // _OPENMP

    /// Helper for init
    void initHistoryFile();

    static bool        _hasBeenInitialized;    ///< Flag for initialization (initialization cannot be performed more than once).


    static std::unique_ptr<OutputDirectToFile> _single;    ///< The singleton

    size_t                          _outputSize;

    /**
     Format for output in a file.
     Might include some raw strings, do not convert to DisplayStatsType.
     */
    DisplayStatsTypeList            _outputFileFormat;

    std::string                     _solutionFile;
    std::ofstream                   _solutionStream;

    std::string                     _historyFile;
    std::ofstream                   _historyStream;

    /// Even if solution file is provided we can temporarily disable solution file (PhaseOne)
    bool                            _enabledSolutionFile;

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_OUTPUTDIRECTTOFILE__
