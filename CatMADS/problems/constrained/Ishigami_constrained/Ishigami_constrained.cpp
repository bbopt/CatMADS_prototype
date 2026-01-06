#include "Nomad/nomad.hpp"
#include "Algos/EvcInterface.hpp"
#include "Algos/Mads/Mads.hpp"
#include "Algos/Mads/MadsMegaIteration.hpp"
#include "Algos/Mads/SearchMethodAlgo.hpp"
#include "Algos/Mads/SpeculativeSearchMethod.hpp"
#include "Algos/Mads/QuadSearchMethod.hpp"
#include "Algos/SubproblemManager.hpp"
#include "Cache/CacheBase.hpp"
#include "Type/EvalSortType.hpp"
#include "Algos/AlgoStopReasons.hpp"
#include "Util/AllStopReasons.hpp"
#include "Math/MatrixUtils.hpp"
#include "Math/RNG.hpp"
#include "CatMADS.hpp"
#include "MyExtendedPoll/MyExtendedPollMethod2.hpp"


// Setup of the problem
const int Ncat=2;
const int Nint=1;
const int Ncon=7;
const int N=Ncat+Nint+Ncon;
const int Lcat=49;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ,
                NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB};
const bool IsConstrained = true;

// Global variables
bool LastSuccessIsQuantitative = false;
bool LastSuccessIsCategorical = false;
bool isCatDistanceUpdated = true;

/*----------------------------------------*/
/*               The problem              */
/*----------------------------------------*/
class My_Evaluator : public NOMAD::Evaluator
{
private:

public:
    My_Evaluator(const std::shared_ptr<NOMAD::EvalParameters>& evalParams)
    : NOMAD::Evaluator(evalParams, NOMAD::EvalType::BB)
    {}

    ~My_Evaluator() {}

    bool eval_x(NOMAD::EvalPoint &x, const NOMAD::Double &hMax, bool &countEval) const override;
};


/*----------------------------------------*/
/*           user-defined eval_x          */
/*----------------------------------------*/
bool My_Evaluator::eval_x(NOMAD::EvalPoint &x,
                          const NOMAD::Double &hMax,
                          bool &countEval) const
{
    // TODO
}


void initAllParams( std::shared_ptr<NOMAD::AllParameters> allParams, std::map<NOMAD::DirectionType,NOMAD::ListOfVariableGroup> & myMapDirTypeToVG, NOMAD::ListOfVariableGroup & myListFixVGForQMS)
{

    // Parameters creation
    allParams->setAttributeValue("DIMENSION", N);
    // Black-box evaluations
    allParams->setAttributeValue("MAX_BB_EVAL", nbEvals);
    // Starting point
    //allParams->setAttributeValue("X0", NOMAD::Point(N, 0.0) );
    // LHS
    std::string budgetLHsFormat = std::to_string(nbEvalsLHS) + " 0";
    allParams->setAttributeValue("LH_SEARCH", NOMAD::LHSearchType(budgetLHsFormat.c_str()));

    // Bounds for all variables except the first group (categorical variable)
    auto lb = NOMAD::ArrayOfDouble(N, -2.0);
    auto ub = NOMAD::ArrayOfDouble(N,  2.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 6; 
    ub[1] = 6;
    // Integer lower bounds
    lb[Ncat+0] = 1; 
    // Integer upper bounds
    ub[Ncat+0] = 20;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, // categorical variables
    NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {2,  3,4,5,6,7,8,9}; // quantitative variables
    allParams->setAttributeValue("VARIABLE_GROUP", NOMAD::ListOfVariableGroup({vg0,vg1}));
    
    // Poll in two subpolls
    NOMAD::DirectionTypeList dtList = {NOMAD::DirectionType::USER_FREE_POLL, NOMAD::DirectionType::ORTHO_2N};
    allParams->setAttributeValue("DIRECTION_TYPE",dtList);
    
    // Set the map of direction types and variable group. This is passed to Mads in the main function
    myMapDirTypeToVG = {{dtList[0],{vg0}},{dtList[1],{vg1}}};
    
    // Constraints and objective
    allParams->setAttributeValue("BB_OUTPUT_TYPE", bbOutputTypeListSetup);

    // Quad search where the first group of variables is fixed
    allParams->setAttributeValue("QUAD_MODEL_SEARCH", true);
    myListFixVGForQMS = {vg0};

    // Default searches that are deactivated 
    allParams->setAttributeValue("NM_SEARCH", false);
    allParams->setAttributeValue("SPECULATIVE_SEARCH", false);
    
    // Enable the user search method
    allParams->setAttributeValue("USER_SEARCH", true);

    // Display
    allParams->setAttributeValue("DISPLAY_DEGREE", 2);
    allParams->setAttributeValue("DISPLAY_STATS", NOMAD::ArrayOfString("bbe ( sol ) obj cons_h"));
    allParams->setAttributeValue("DISPLAY_ALL_EVAL", true);

    // Fix seed for duplicity of results
    allParams->setAttributeValue("SEED", seedSetup);
    allParams->setAttributeValue("RNG_ALT_SEEDING", true);

    // File history for convergence plots and profiles
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("ishigami.txt bbe sol obj cons_h"));

    // Parameters validation
    allParams->checkAndComply();
    
}


/*------------------------------------------*/
/*            NOMAD main function           */
/*------------------------------------------*/
int main ( int argc , char ** argv )
{

    
    // List of files to clear
        std::vector<std::string> filesToClear = {
            fileCache,
            fileCatDirections,
            fileParams
        };

    // Clear the files at the start
    deleteFiles(filesToClear);


    NOMAD::MainStep TheMainStep;

    // Set parameters
    auto params = std::make_shared<NOMAD::AllParameters>();
    
    // Map to associate a direction type to a group of variable.
    std::map<NOMAD::DirectionType,NOMAD::ListOfVariableGroup> myMapDirTypeToVG;
    
    // List of fix variable group for Quad model search
    NOMAD::ListOfVariableGroup myListFixVGForQMS;

    initAllParams(params, myMapDirTypeToVG, myListFixVGForQMS);
    TheMainStep.setAllParameters(params);

    // Custom Evaluator
    //std::unique_ptr<My_Evaluator> ev(new My_Evaluator(params->getEvalParams())); //before ExtendedPoll
    std::shared_ptr<NOMAD::Evaluator> ev(new My_Evaluator(params->getEvalParams())); //with ExtendedPoll
    TheMainStep.setEvaluator(std::move(ev));
    
    // Main step start initializes Mads (default algorithm)
    TheMainStep.start();
       
    // Define new sort function and sort according to that function
    auto customOrder = std::make_shared<CustomOrder>();
    NOMAD::EvcInterface::getEvaluatorControl()->setUserCompMethod(customOrder);

    // Define post eval callback
    NOMAD::EvalCallbackFunc<NOMAD::CallbackType::POST_EVAL_UPDATE> cbPostEvalUpdate = customPostEvalUpdateCB;
    NOMAD::EvcInterface::getEvaluatorControl()->addEvalCallback<NOMAD::CallbackType::POST_EVAL_UPDATE>(cbPostEvalUpdate);

    // Registering the callback functions
    auto mads = std::dynamic_pointer_cast<NOMAD::Mads>(TheMainStep.getAlgo(NOMAD::StepType::ALGORITHM_MADS));
    if (nullptr == mads)
    {
        throw NOMAD::Exception(__FILE__,__LINE__,"Cannot access to Mads algorithm");
    }    
    
    // Callbacks for search
    mads->addCallback(NOMAD::CallbackType::USER_METHOD_SEARCH, userSearchMethodCallbackSpeculative);
    //mads->addCallback(NOMAD::CallbackType::USER_METHOD_SEARCH_2, userSearchMethodCallbackGP);
    
    // Default quad model search (QMS) must not consider categorical variable.
    // Give access to the group of categorical variables.
    // Their values are fixed during QMS
    params->getRunParams()->setListFixVGForQuadModelSearch(params->getPbParams(), myListFixVGForQMS );


    // Callback to generate Mads user poll trial points
    // Add a custom poll method on a variable group.
    mads->addCallback(NOMAD::CallbackType::USER_METHOD_FREE_POLL, userPollMethodCallback);
    // Associate direction type and variable groups
    params->getRunParams()->setMapDirTypeToVG(params->getPbParams(), myMapDirTypeToVG);
    
    // Set user extended poll method
    std::unique_ptr<NOMAD::ExtendedPollMethod> extendedPollMethod = std::make_unique<MyExtendedPollMethod2>(mads, ev);
    mads->setExtendedPollMethod(std::move(extendedPollMethod));

    TheMainStep.run();
    TheMainStep.end();

    return 0;
}
