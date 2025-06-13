#ifndef CATMADS_HPP
#define CATMADS_HPP

#include "Nomad/nomad.hpp"
#include <vector>
#include <list>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// Global setup variables fix or same formula for all problems: initialized in GPCatMADS.cpp
extern std::string basePath;
extern std::string pythonEnv;
extern const int nbEvalsPerVariable;
extern const int nbEvals; // formula based on nbEvalsPerVariable in GPCatMADS.cpp
extern const int nbEvalsLHS; // formula based on nbEvals in GPCatMADS.cpp
extern int nbCatNeighbors; // formula based on the number of categories Lcat
extern const int seedSetup; // to automize the optimization runs on the seed-based instances

// Global variables specific for each problem: initialized in problem file
extern const int Ncat;
extern const int Nint;
extern const int Ncon;
extern const int N;
extern const int Lcat; // total nb of categories 
extern bool LastSuccessIsQuantitative;
extern bool LastSuccessIsCategorical;
extern bool isCatDistanceUpdated;
extern const NOMAD::BBOutputTypeList bbOutputTypeListSetup;
extern const bool IsConstrained;



// Path variables
extern std::string fileCache;
extern std::string fileCatDirections;
extern std::string fileParams;

// Python scripts paths
extern std::string simpleCategoricalDist;
extern std::string catPoll;


// Callback for CAT-MADS
bool userPollMethodCallback(const NOMAD::Step& step, std::list<NOMAD::Direction>& dirs, const size_t& n);
bool userSearchMethodCallbackSpeculative(const NOMAD::Step& step, NOMAD::EvalPointSet & trialPoints);
void customPostEvalUpdateCB(NOMAD::EvalQueuePointPtr& evaluatedQueuePoint);


class CustomOrder : public NOMAD::OrderByEval {
public:
    /// Constructor
    explicit CustomOrder();

    bool comp(NOMAD::EvalQueuePointPtr& p1, NOMAD::EvalQueuePointPtr& p2) const override;

    void completeTrialPointsInformation(const NOMAD::Step *step, NOMAD::EvalPointSet & trialPoints) override;
};




// Utility functions
void writeCacheToFile(const NOMAD::Step& step, const std::string& filePathWriteCache, int nbCatNeighbors);
int runPythonScript(const std::string& pythonEnv, const std::string& scriptPath);
enum class FileType { CatDirections, EGODirection, PollPtsSurrogateEval, CatImprovement };
std::vector<std::vector<double>> readTextFile(const std::string& filePath, FileType fileType);
void deleteFiles(const std::vector<std::string>& files);


#endif