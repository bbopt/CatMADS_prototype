/**
 \file   fileutils.cpp
 \brief  Utility functions for files
 \author Viviane Rochon Montplaisir
 \date   June 2017
 \see    fileutils.hpp
 */
#include "../Util/Exception.hpp"
#include "../Util/fileutils.hpp"
#include "../Util/utils.hpp"

#ifdef _WIN32
#include <direct.h>     // for getcwd
#include <cctype>       // for isalpha
#define getcwd _getcwd
#endif

/*-----------------------------------------------------------------*/
/*              check if a file exists and is executable           */
/*-----------------------------------------------------------------*/
bool NOMAD::checkExeFile(const std::string &filename)
{
#ifdef WINDOWS
    // don't check on Windows:
    return true;
#else
    return (access ( filename.c_str() , X_OK ) == 0);
#endif
}

/*-----------------------------------------------------------------*/
/*              check if a file exists and is readable             */
/*-----------------------------------------------------------------*/
bool NOMAD::checkReadFile(const std::string &filename)
{
#ifdef _MSC_VER
    return (_access ( filename.c_str() , 4 ) == 0);
#else
    return (access ( filename.c_str() , R_OK ) == 0);
#endif
}

/*-----------------------------------------------------------------*/
/*              check if a file exists and is writable             */
/*-----------------------------------------------------------------*/
bool NOMAD::checkWriteFile(const std::string &filename)
{
#ifdef _MSC_VER
    return (_access (filename.c_str() , 6 ) == 0);
#else
    return (access (filename.c_str() , W_OK ) == 0);
#endif
}


// Get current directory.
std::string NOMAD::curdir()
{
    char dirbuff[1024];
    if (nullptr == getcwd(dirbuff, 1024))
    {
        std::cout << "Warning: Could not get current directory" << std::endl;
    }
    std::string dir(dirbuff);

    return dir;
}


// Extract directory from the given filename.
// If there is no directory, return current directory using getcwd().
std::string NOMAD::dirname(const std::string &filename)
{
    std::string dir;

    size_t sepIndex = filename.find_last_of(NOMAD::DIR_SEP);
    if (sepIndex < filename.size())
    {
        dir = filename.substr(0, sepIndex) + NOMAD::DIR_SEP;
    }
    else
    {
        dir = std::string(".") + NOMAD::DIR_SEP;
    }

    return dir;
}


// Extract file root from the given filename.
// Ex. "/path/toto.txt" returns "toto"
std::string NOMAD::rootname(const std::string &filename)
{
    std::string root;

    size_t sepIndex = filename.find_last_of(NOMAD::DIR_SEP);
    size_t pointIndex = filename.find_last_of('.');
    // For cleaner comparisons
    if (std::string::npos == sepIndex)
    {
        sepIndex = filename.size();
    }
    if (std::string::npos == pointIndex)
    {
        pointIndex = filename.size();
    }

    if (sepIndex < pointIndex)
    {
        root = filename.substr(sepIndex + 1, pointIndex - sepIndex - 1);
    }
    else if (sepIndex < filename.size())
    {
        root = filename.substr(sepIndex + 1, filename.size() - sepIndex - 1);
    }
    else if (pointIndex < filename.size())
    {
        root = filename.substr(0, pointIndex);
    }
    else
    {
        // No DIR_SEP and no point
        root = filename;
    }

    return root;
}


// Extract extension from the given filename.
// Ex. "/path/toto.txt" returns ".txt"
std::string NOMAD::extension(const std::string &filename)
{
    std::string ext;

    size_t sepIndex = filename.find_last_of(NOMAD::DIR_SEP);
    size_t pointIndex = filename.find_last_of('.');

    if (std::string::npos != pointIndex)
    {
        if ((std::string::npos == sepIndex) || (sepIndex < pointIndex))
        {
            ext = filename.substr(pointIndex, filename.size() - pointIndex);
        }
    }

    return ext;
}


// If filename has a path, leave it.
// If it doesn't, add dirname() to it.
std::string NOMAD::fullpath(const std::string &filename)
{
    std::string full;

    size_t k = filename.find_last_of(NOMAD::DIR_SEP);
    if (k < filename.size())
    {
        full = filename;
    }
    else
    {
        full = NOMAD::dirname(filename) + filename;
    }

    return full;
}


// Return true if a filename is absolute.
// On Linux/macOS, check if the filename start with '/'.
// On Windows, naively check if the filename starts with a letter
// followed by a colon.
// Return false otherwise (filename is relative, or filename is file only).
bool NOMAD::isAbsolute(const std::string &filename)
{
#ifdef WINDOWS
    if (filename.size() < 2)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"isAbsolute: File name is too small");
    }
    return (std::isalpha(filename[0]) && ':' == filename[1]);
#else
    if (filename.empty())
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"isAbsolute: Empty file name");
    }
    return (filename[0] == NOMAD::DIR_SEP);
#endif
}


// Add a '/' (DIR_SEP) at the end of the dirname if it does not end with one.
void NOMAD::ensureDirPath(std::string &dirname)
{
    if (dirname.empty())
    {
        dirname = std::string(".");
    }

    if (dirname[dirname.size()-1] != NOMAD::DIR_SEP)
    {
        dirname += NOMAD::DIR_SEP;
    }

}


// Input a line (from a parameters file).
// Remove comments starting with '#'.
// Replace tabs by spaces.
// Trim extra spaces.
void NOMAD::removeComments(std::string &line)
{
    // Remove comments
    size_t comment_start = line.find('#');
    size_t line_size = line.size();
    if (comment_start < line_size)
    {
        line.replace(comment_start, line_size-comment_start, "");
    }
    line_size = line.size(); // new line size

    // Remove tabs
    size_t tab_index = line.find('\t');
    while (tab_index != std::string::npos)
    {
        line.replace(tab_index, 1, " ");
        tab_index = line.find('\t');
    }

    // Trim extra spaces at the beginning
    size_t space_index = line.find(' ');
    while (0 == space_index && line_size > 0)
    {
        line.replace(0, 1, "");
        space_index = line.find(' ');
        line_size = line.size();
    }

    // Trim trailing '\r'
    size_t r_index = line.find('\r');
    if (line_size-1 == r_index && line_size > 0)
    {
        line.replace(r_index, 1, "");
        line_size = line.size();
    }

    // Trim extra spaces at the end
    size_t space_rindex = line.rfind(' ');
    while (line_size-1 == space_rindex && line_size > 0)
    {
        line.replace(space_rindex, 1, "");
        space_rindex = line.rfind(' ');
        line_size = line.size();
    }

    // Trim extra spaces in the middle
    size_t two_space_index = line.find("  ");
    while (two_space_index != std::string::npos)
    {
        line.replace(two_space_index, 2, " ");
        two_space_index = line.find("  ");
    }

}


void NOMAD::completeFileName(std::string &filename,
                                        const std::string &problemDir,
                                        bool addSeed,
                                        int seed)
{
    if (filename.empty()
        || NOMAD::isAbsolute(filename))
    {
        return;
    }

    if (NOMAD::isAbsolute(problemDir))
    {
        filename = problemDir + filename;
    }
    else
    {
        filename = NOMAD::curdir() + NOMAD::DIR_SEP + problemDir + filename;
    }

    // Set stats file name relative to problem dir.
    if (addSeed)
    {
        std::string sSeed = NOMAD::itos(seed);
        size_t nSeed = sSeed.size();

        addSeedToFileName(nSeed, sSeed, filename);
    }
}


/*---------------------------------------------------------------*/
/*  add seed to a file name: filename.ext --> filename.seed.ext  */
/*---------------------------------------------------------------*/
void NOMAD::addSeedToFileName(size_t nSeed,
                              const std::string& sSeed,
                              std::string& filename)
{
    size_t filenameSize = filename.size();

    if (0 == filenameSize)
    {
        return;
    }

    size_t lastPoint = filename.find_last_of('.');
    std::string ext;
    std::string fic = filename;

    if (lastPoint < filenameSize)
    {
        fic = filename.substr(0, lastPoint);
        ext = filename.substr(lastPoint, filenameSize - lastPoint);
        filenameSize = lastPoint;
    }

    if (filenameSize <= nSeed + 1 ||
        fic.substr(filenameSize - nSeed, filenameSize - 1) != sSeed)
    {
        filename = fic + "." + sSeed + ext;
    }
}


bool NOMAD::readAllFile(std::string &info, const std::string &filename)
{
    std::ifstream infile { filename };
    info = std::string(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>());
    return !(info.empty());
}
