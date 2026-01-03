/**
 \file   fileutils.hpp
 \brief  Utility functions about files (headers)
 \author Viviane Rochon Montplaisir
 \date   June 2017
 \see    fileutils.cpp
 */
#ifndef __NOMAD_4_5_FILEUTILS__
#define __NOMAD_4_5_FILEUTILS__

// use of 'access' or '_access', and getpid() or _getpid():
#ifdef _MSC_VER
#include <io.h>
//#include <process.h>
#else
#include <unistd.h>
#endif

#include <fstream>

#include "../nomad_platform.hpp"
#include "../Util/defines.hpp"

#include "../nomad_nsbegin.hpp"

// Copied from NOMAD_3.
/// Check if a file exists and is executable.
/**
 \param filename  A string corresponding to a file name -- \b IN.
 \return          A boolean equal to \c true if the file is executable.
 */
DLL_UTIL_API bool checkExeFile(const std::string &filename);


/// Check if a file exists and is readable.
/**
 \param filename  A string corresponding to a file name -- \b IN.
 \return          A boolean equal to \c true if the file exists and is readable.
 */
DLL_UTIL_API bool checkReadFile(const std::string &filename);


/// Check if a file exists and is writable.
/**
 \param filename  A string corresponding to a file name -- \b IN.
 \return          A boolean equal to \c true if the file exists and is writable.
 */
DLL_UTIL_API bool checkWriteFile(const std::string &filename);


// Get current directory
DLL_UTIL_API std::string curdir();


// Extract directory from the given filename.
// If there is no directory, return ".".
DLL_UTIL_API std::string dirname(const std::string &filename);

// Extract file root from the given filename.
// Ex. "/path/toto.txt" returns "toto"
DLL_UTIL_API std::string rootname(const std::string &filename);

// Extract extension from the given filename.
// Ex. "/path/toto.txt" returns ".txt"
DLL_UTIL_API std::string extension(const std::string &filename);

// If filename has a path, leave it.
// If it doesn't, add dirname() to it.
DLL_UTIL_API std::string fullpath(const std::string &filename);


// Return true if a filename is absolute, i.e., starts with '/' (DIR_SEP).
// Return false otherwise (filename is relative, or filename is file only).
DLL_UTIL_API bool isAbsolute(const std::string &filename);


// Add a '/' (DIR_SEP) at the end of the dirname if it does not end with one.
DLL_UTIL_API void ensureDirPath(std::string &dirname);


// Input a line (from a parameters file).
// Remove comments starting with '#'.
// Replace tabs by spaces.
// Trim extra spaces.
DLL_UTIL_API void removeComments(std::string &line);

// Add problemDir to filename.
// Add seed if desired.
DLL_UTIL_API void completeFileName(std::string &filename,
                      const std::string &problemDir,
                      bool addSeed = false,
                      int seed = 0);

DLL_UTIL_API void addSeedToFileName(size_t nSeed,
                       const std::string& sSeed,
                       std::string& filename);


// Write to file
// Use T::operator<< (examples of T: CacheSet, Mads)
// Will break if T::operator<< is not defined.
template<typename T>
bool write(const T &info, const std::string &filename)
{
    bool writeSuccess = true;
    std::ofstream fout;

    if (filename.empty())
    {
        std::cout << "Warning: " << typeid(T).name() << ": Cannot write to file: file name is not defined.";
        writeSuccess = false;
    }

    if (writeSuccess)
    {
        fout.open(filename.c_str(), std::ofstream::out);
        if (fout.fail())
        {
            std::cout << "Warning: " << typeid(T).name() << ": Cannot write to file " + filename << std::endl;
            writeSuccess = false;
            fout.close();
        }
    }

    if (writeSuccess)
    {
        fout.clear();
        fout << info;
        fout.close();
    }


    return writeSuccess;
}


// Read from file
// Use T::operator>> (examples of T: CacheSet, Mads)
// Will break if T::operator>> is not defined.
template<typename T>
bool read(T &info, const std::string &filename)
{
    bool readSuccess = true;
    std::ifstream fin;

    if (filename.empty())
    {
        std::cout << "Warning: " << typeid(T).name() << ": Cannot read file: file name is not defined.";
        readSuccess = false;
    }

    if (readSuccess)
    {
        if (!checkReadFile(filename))
        {
            std::cout << "Warning: " << typeid(T).name() << ": File does not exist or cannot be read: " + filename << std::endl;
            readSuccess = false;
        }
    }

    if (readSuccess)
    {
        fin.open(filename.c_str(), std::ifstream::out);
        if (fin.fail())
        {
            std::cout << "Warning: " << typeid(T).name() << ": Cannot read from file " + filename << std::endl;
            readSuccess = false;
            fin.close();
        }
    }

    if (readSuccess)
    {
        fin >> info;
    }

    fin.close();

    return readSuccess;
}


// Read all file and store it in string info.
DLL_UTIL_API bool readAllFile(std::string &info, const std::string &filename);


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_FILEUTILS__
