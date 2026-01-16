#include "CatMADS.hpp"
#include "Nomad/nomad.hpp"
#include "Algos/EvcInterface.hpp"
#include "Algos/Mads/Mads.hpp"
#include "Algos/Mads/MadsMegaIteration.hpp"
#include "Algos/Mads/SearchMethodAlgo.hpp"
#include "Algos/Mads/QuadSearchMethod.hpp"
#include "Algos/Mads/SpeculativeSearchMethod.hpp"
#include "Algos/SubproblemManager.hpp"
#include "Algos/SurrogateEvaluation.hpp"
#include "Cache/CacheBase.hpp"
#include "Type/EvalSortType.hpp"
#include "Algos/AlgoStopReasons.hpp"
#include "Util/AllStopReasons.hpp"
#include "Math/MatrixUtils.hpp"
#include "Math/RNG.hpp"

// Setup variables for all problems
std::string basePath = "/home/edhal/CatMADS_prototype_Porifera/CatMADS/";  // MUST BE HARDCODED FOR PROTOTYPE IMPLEMENTATION
std::string pythonEnv = "/home/edhal/gp-catmads-env/bin/python"; // MUST BE HARDCODED FOR PROTOTYPE IMPLEMENTATION
const int nbEvalsPerVariable=10; //250
const int nbEvals = N*nbEvalsPerVariable; // N is initialized in a problem specific folder
const int nbEvalsLHS=static_cast<int>(nbEvals*0.2); //0.2 for GPCatMADS 
int nbCatNeighbors = std::max(2, static_cast<int>(std::sqrt(Lcat)));
const int seedSetup = 0; 


// Paths
std::string fileCache = basePath + "readwrite_files/cachePts.txt";
std::string fileCatDirections = basePath + "readwrite_files/catDirections.txt";
std::string fileParams = basePath + "readwrite_files/params.pkl";

// Python scripts
//std::string simpleCategoricalDist = basePath + "python_scripts/simple_cat_distance.py";
std::string simpleCategoricalDist = basePath + "python_scripts/Porifera_cat_distance.py";
std::string catPoll = basePath + "python_scripts/cat_neighbors.py";


// Comparison with comp function
// Complete trial point information using model on regular poll points only
// Constructor
CustomOrder::CustomOrder()
    : NOMAD::OrderByEval({NOMAD::EvalType::MODEL, NOMAD::defaultFHComputeTypeS})
{
    setName("OrderByModel_BYUSER");
}

// Comparison function
bool CustomOrder::comp(NOMAD::EvalQueuePointPtr& p1, NOMAD::EvalQueuePointPtr& p2) const {


    // Place points in RegularPoll in priority, and the points in RegularPoll are sorted with QUAD models
    bool p1FromUserPoll = (p1->getGenStep() == NOMAD::StepType::POLL_METHOD_USER);
    bool p2FromUserPoll = (p2->getGenStep() == NOMAD::StepType::POLL_METHOD_USER);

    // Case where both points are from UserPoll
    if (p1FromUserPoll && p2FromUserPoll) {
        // We want to prioritize points with smaller tag (generated before)
        // If p1.get() > p2.getTag(), then return true (p1 less interesting) s.t. p1 generated after p2 
        return p1->getTag() > p2->getTag();
    }

    // Case where only one point is from UserPoll
    if (p1FromUserPoll) {
        return true;
    }
    if (p2FromUserPoll) {
        return false;
    }

    auto eval1 = p1->getEval(NOMAD::EvalType::MODEL);
    auto eval2 = p2->getEval(NOMAD::EvalType::MODEL);

    if (nullptr == eval2) {
        return false;
    } else if (nullptr == eval1) {
        return true;
    }

    // Case where both points are from regular Poll
    return NOMAD::OrderByEval::comp(p1, p2);
    
}

// Complete trial points information
void CustomOrder::completeTrialPointsInformation(const NOMAD::Step *step, NOMAD::EvalPointSet & trialPoints) {
    
    
    // Use QUAD models for completing information the points in RegularPoll (quantitative)
    NOMAD::EvalPointSet regularPollTrialPoints, userPollTrialPoints;

    for (auto evalQueuePoint : trialPoints) {
        if (evalQueuePoint.getGenStep() != NOMAD::StepType::POLL_METHOD_USER) {
            regularPollTrialPoints.insert(evalQueuePoint);
        } else {
            userPollTrialPoints.insert(evalQueuePoint);
        }
    }

    NOMAD::SurrogateEvaluation surrogateEvaluation(step, regularPollTrialPoints, NOMAD::EvalType::MODEL);
    surrogateEvaluation.start();
    bool success = surrogateEvaluation.run();
    surrogateEvaluation.end();


    if (success) {
        trialPoints.clear();
        trialPoints = regularPollTrialPoints;
        std::copy(userPollTrialPoints.begin(), userPollTrialPoints.end(), std::inserter(trialPoints, trialPoints.end()));
    }
    
}



// --------------------- Callbacks ---------------------- //

// The function to generate user search directions. This is registered as a callback below.
bool userPollMethodCallback(const NOMAD::Step& step, std::list<NOMAD::Direction> & dirs, const size_t &n)
{
    // Reset directions
    dirs.clear();

    // Rrun this script only if nbCatNeighbors is nonzero
    if (nbCatNeighbors == 0) {
        // Skip execution if no neighbors are defined
        return false; 
    }

    // Check if there is a poll
    auto callingPoll = dynamic_cast<const NOMAD::PollMethodBase*>(&step);
    if (nullptr == callingPoll)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"No poll available.");

    }

    // Feasible or infeasible solution might not exist, return false if it is the case: the poll is skipped
    auto frameCenterTmp = callingPoll->getFrameCenter();
    if (nullptr == frameCenterTmp)
    {
        return false;
    }
    
    // Problem information for computing categorical neighbors below
    writeCacheToFile(step, fileCache, nbCatNeighbors);


    // Construct distance
    if (isCatDistanceUpdated){

        // This is constructs the categorical distance 
        // For prototype implementation, it is done once after the DoE (first poll)
        int exitCode = runPythonScript(pythonEnv, simpleCategoricalDist);  
        
        // Stop updating model/distance 
        if (exitCode==1){
            isCatDistanceUpdated = false;
        }
    }

    // Generates a text file with directions for cat directions 
    runPythonScript(pythonEnv, catPoll);   

    // Read categorical neighbors
    auto catDirections = readTextFile(fileCatDirections, FileType::CatDirections);

    // -- Store directions -- //
    for (const auto& direction : catDirections) {
        NOMAD::Direction dir(N, 0.0); // Initialize a direction vector of size n

        for (size_t i = 0; i < direction.size() && i < N; ++i) {
            dir[i] = direction[i]; // Assign values from the current direction vector
        }

        dirs.push_back(dir); // Store the constructed direction
    }
    // -- Store directions -- //

    return true;
}


// Speculative search that is only done if previous iteration had a success on the quantitative variables
bool userSearchMethodCallbackSpeculative(const NOMAD::Step& step, NOMAD::EvalPointSet & trialPoints)
{

    trialPoints.clear();

    if (LastSuccessIsQuantitative)
    {

        NOMAD::SpeculativeSearchMethod speculativeSearch(&step);
        speculativeSearch.generateTrialPoints();

        for ( const auto & tp: speculativeSearch.getTrialPoints())
        {
            if (tp.ArrayOfDouble::isDefined() && nullptr != tp.getPointFrom())
            {
                // Verify the parent step is a poll
                // TODO: avoid harcoded StepTypes
                auto parentStep = tp.getPointFrom()->getGenStep();
                if (parentStep == NOMAD::StepType::POLL_METHOD_ORTHO_2N || parentStep == NOMAD::StepType::POLL_METHOD_DOUBLE) 
                {
                    // Safety check that categorical variables aren't modified
                    auto dir = NOMAD::EvalPoint::vectorize(*(tp.getX()), *(tp.getPointFrom()->getX()));
                    bool allCatDirectionsZeros = true;
                    for (int i = 0; i < Ncat; ++i){
                        if (dir[i] != 0){
                            allCatDirectionsZeros = false;
                            break;
                        }
                    }
                    if (allCatDirectionsZeros)
                    {
                        trialPoints.insert(tp);
                    }
                }
            }

        }

        return true;
    }
    else {
        return false;
    }

}


// Post evaluation callback: keep track of the type of success (either quantitative or categorical)      
void customPostEvalUpdateCB(NOMAD::EvalQueuePointPtr& evaluatedQueuePoint)
{
    // Reset global variables to false 
    LastSuccessIsQuantitative = false;
    LastSuccessIsCategorical = false;

    if (nullptr != evaluatedQueuePoint->getEval(NOMAD::EvalType::BB) && evaluatedQueuePoint->getEvalStatus(NOMAD::EvalType::BB) == NOMAD::EvalStatusType::EVAL_OK )
    {

        if ( nullptr != evaluatedQueuePoint->getPointFrom())
        {
            // Force the compute success. It is normally done after the callback
            auto evc = NOMAD::EvcInterface::getEvaluatorControl();
            evc->computeSuccess(evaluatedQueuePoint, true /* eval is ok */);
            
            if (NOMAD::SuccessType::FULL_SUCCESS == evaluatedQueuePoint->getSuccess())
            {
                // Retrieve successful direction
                auto dir = NOMAD::EvalPoint::vectorize(*(evaluatedQueuePoint->getX()), *(evaluatedQueuePoint->getPointFrom()));                

                // We could access to the variable group here by using step pbParam.
                // Workaround for now is to use a global variable.
                bool allCatDirectionsZeros = true;
                for (int i = 0; i < Ncat; ++i){
                    if (dir[i] != 0){
                        allCatDirectionsZeros = false;
                        break;
                    }
                }
                if (allCatDirectionsZeros){
                    LastSuccessIsQuantitative = true;
                }
                else
                {
                    LastSuccessIsCategorical = true;
                }
            }
            
        }

    }

}
// --------------------- Callbacks ---------------------- //



// --------------------- Utility functions ---------------------- //
// Write NOMAD cache into a text file
void writeCacheToFile(const NOMAD::Step& step, const std::string& filePathWriteCache, int nbCatNeighbors) {
    auto mads = dynamic_cast<const NOMAD::Mads*>(step.getRootAlgorithm());
    if (nullptr == mads) {
        throw NOMAD::Exception(__FILE__, __LINE__, "No Mads available.");
    }

    auto mainPbParams = mads->getParentStep()->getPbParams();
    auto bbInputTypes = mainPbParams->getAttributeValue<NOMAD::BBInputTypeList>("BB_INPUT_TYPE");
    auto lowerBound = mainPbParams->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND");
    auto upperBound = mainPbParams->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND");
    
    // -- Current step information -- //
    std::string stepStr; // use in Cache
    auto callingPoll = dynamic_cast<const NOMAD::PollMethodBase*>(&step);
    auto callingSearch = dynamic_cast<const NOMAD::SearchMethodBase*>(&step);

    std::shared_ptr<NOMAD::EvalPoint> pointToPrint = nullptr;
    std::shared_ptr<NOMAD::EvalPoint> pointToPrint2 = nullptr;
    NOMAD::Double hMax = NOMAD::Double();

    if (nullptr != callingPoll)
    {
        stepStr = "poll";
        auto frameCenterTmp = callingPoll->getFrameCenter();
        if (nullptr == frameCenterTmp)
        {
            throw NOMAD::Exception(__FILE__,__LINE__,"No frame center available.");
        }
        // Duplicate pointToPrint2 to be conform with search that needs the feasible and infeasible solutions
        pointToPrint = std::make_shared<NOMAD::EvalPoint>(*frameCenterTmp);
        pointToPrint2 = std::make_shared<NOMAD::EvalPoint>(*frameCenterTmp);
    }
    else if (nullptr != callingSearch)
    {
        stepStr = "search";
        auto barrier = mads->getMegaIterationBarrier();
        if (nullptr == barrier)
        {
            throw NOMAD::Exception(__FILE__,__LINE__,"No barrier available.");
        }

        pointToPrint = barrier->getCurrentIncumbentFeas();

        if (IsConstrained){
            pointToPrint2 = barrier->getCurrentIncumbentInf();
        }
        else{
            // FIX: if unconstrained, pass the feasible to be compatible with the printing below
            pointToPrint2 = pointToPrint;
        }
        
        hMax = barrier->getHMax();
    }
    else
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"No poll or search method available.");
    }
    // -- Current step information -- //

    // -- Best function value: used for EGO and CatImprovement -- //
    NOMAD::FHComputeType computeType = {NOMAD::EvalType::BB, {NOMAD::ComputeType::STANDARD, NOMAD::HNormType::L2}};
    NOMAD::Double bestFeasVal;
    if (!(pointToPrint==nullptr)){
        bestFeasVal = pointToPrint->getF(computeType);
    }    
    else{
        bestFeasVal = NOMAD::Double(INFINITY);        
    }
    
    NOMAD::Double bestInfVal;
    if (IsConstrained & !(pointToPrint2==nullptr)){
            bestInfVal = pointToPrint2->getF(computeType);
        }
    else{
        bestInfVal = NOMAD::Double(INFINITY);
    }
    // -- Best function value: used for EGO and CatImprovement -- //

    // -- Evaluated points -- //
    std::vector<NOMAD::EvalPoint> evalPointList;
    auto crit0 = [&](const NOMAD::EvalPoint& evalPoint) { return true; };
    NOMAD::CacheBase::getInstance()->find(crit0, evalPointList);
    // -- Evaluated points -- //

    // -- Print pb info state and points -- //
    std::ofstream ptsCache(filePathWriteCache);
    if (!ptsCache.is_open()) {
        std::cerr << "Unable to open cache file for writing." << std::endl;
        return;
    }
    ptsCache << "Variable types: " << bbInputTypes << "\n";
    ptsCache << "Number of cat, int and cont: " << Ncat << " " << Nint << " " << Ncon << "\n";
    ptsCache << "Lower bounds: " << lowerBound << "\n";
    ptsCache << "Upper bounds: " << upperBound << "\n";
    ptsCache << "Current step: " << stepStr << "\n";
    if (!(pointToPrint==nullptr)){
        ptsCache << "Current frame (poll) or feasible (search) : " << *(pointToPrint->getX()) << "\n";
    }
    else{
        ptsCache << "Current frame (poll) or feasible (search) : " << NOMAD::ArrayOfDouble(N) << "\n";
    }
    if (!(pointToPrint2==nullptr))
    {
        ptsCache << "Current frame (poll) or infeasible (search): " << *(pointToPrint2->getX()) << "\n";

    }
    else{
        ptsCache << "Current frame (poll) or infeasible (search): " << NOMAD::ArrayOfDouble(N) << "\n";
    }
    ptsCache << "Best current function values: " << bestFeasVal << " " << bestInfVal << " " << hMax << "\n";
    ptsCache << "Nb of cat neighbors: " << nbCatNeighbors << "\n";
    ptsCache << "Seed: " << seedSetup << "\n";
    ptsCache << "Budget per variables: " << nbEvalsPerVariable << "\n";


    for (const auto& evalPoint : evalPointList) {
        ptsCache << evalPoint << "\n";
    }

    ptsCache.close();
    // -- Print pb info state and points -- //

}



// Run python script with given Python virtual environment and script
int runPythonScript(const std::string& pythonEnv, const std::string& scriptPath) {
    std::string command = pythonEnv + " " + scriptPath;

    // Execute the command and capture the exit code
    int exitCode = system(command.c_str());

    // Handle normal termination
    if (WIFEXITED(exitCode)) {
        int statusCode = WEXITSTATUS(exitCode);

        if (statusCode == 0 || statusCode == 1) {
            // statusCode == 0: standard exit 
            // statusCode == 1: used to stop constructing the categorical distance
            return statusCode;
        } else {
            std::cerr << "Unexpected exit code: " << statusCode << std::endl;
            throw std::runtime_error("Unexpected Python script exit code.");
        }
    } else {
        // Handle abnormal termination
        std::cerr << "Python script terminated abnormally." << std::endl;
        throw std::runtime_error("Python script terminated abnormally.");
    }
}


// Read text files generated by Python scripts
std::vector<std::vector<double>> readTextFile(const std::string& filePath, FileType fileType) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    std::vector<std::vector<double>> result;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<double> values;
        double value;
        char ch;

        while (iss >> ch && (ch < '0' || ch > '9') && ch != '-' && ch != '.') {}
        iss.putback(ch);

        while (iss >> value) {
            values.push_back(value);
        }

        if (!values.empty()) {
            result.push_back(values);
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }

    if (result.empty()) {
        throw std::runtime_error("No valid data found in file: " + filePath);
    }

    return result;
}

// Delete specific files
void deleteFiles(const std::vector<std::string>& files) {
    for (const auto& file : files) {
        if (std::ifstream(file)) {
            if (std::remove(file.c_str()) == 0) {
                std::cout << "Deleted file: " << file << std::endl;
            } else {
                std::cerr << "Error deleting file: " << file << std::endl;
            }
        }
    }
}

// --------------------- Utility functions ---------------------- //
