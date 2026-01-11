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
const int Nint=3;
const int Ncon=4;
const int N=Ncat+Nint+Ncon;
const int Lcat=64;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ};
const bool IsConstrained = false;

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
    (void)hMax;

    // Expect: Ncat = 2, Nint = 3, Ncon = 4
    if (x.size() != (Ncat + Nint + Ncon))
    {
        throw NOMAD::Exception(__FILE__, __LINE__,
                               "Dimension mismatch: expected Ncat + Nint + Ncon variables.");
    }

    // Convention habituelle: (cat, int, con)
    // Indices:
    // [0]=x1^cat, [1]=x2^cat,
    // [2]=x1^int, [3]=x2^int, [4]=x3^int,
    // [5]=x1^con, [6]=x2^con, [7]=x3^con, [8]=x4^con

    // ---- Categorical variables (encoded as 0..7 for A..H) ----
    const int x1cat = static_cast<int>(x[0].todouble());
    const int x2cat = static_cast<int>(x[1].todouble());

    // ---- Integer variables ----
    const int x1i = static_cast<int>(x[2].todouble());
    const int x2i = static_cast<int>(x[3].todouble());
    const int x3i = static_cast<int>(x[4].todouble());

    // ---- Continuous variables ----
    const double x1c = x[5].todouble();
    const double x2c = x[6].todouble();
    const double x3c = x[7].todouble();
    const double x4c = x[8].todouble();

    // ---- Helpers ----
    auto sgn = [](double v) -> double { return (v > 0.0) - (v < 0.0); };

    // ---- s1 = s(x1^cat, x1^con, x3^con) ----
    double s1 = 0.0;
    switch (x1cat)
    {
    case 0: // A
        s1 = x1c + 0.25 * std::tanh(x1c) + 0.15 * x3c;
        break;
    case 1: // B
        s1 = x1c + 0.25 * std::tanh(x1c - 0.5) + 0.12 * x3c;
        break;
    case 2: // C
        s1 = x1c + 0.25 * std::tanh(x1c + 0.5) + 0.10 * x3c;
        break;
    case 3: // D
        s1 = x1c - 0.30 * std::tanh(x1c) + 0.10 * x3c;
        break;
    case 4: // E
        s1 = x1c - 0.30 * std::tanh(x1c + 0.5) + 0.08 * x3c;
        break;
    case 5: // F
        s1 = x1c - 0.30 * std::tanh(x1c - 0.5) + 0.09 * x3c;
        break;
    case 6: // G
        s1 = x1c + 0.18 * std::pow(std::abs(x1c), 1.3) - 0.10 * x3c;
        break;
    case 7: // H
        s1 = x1c + 0.18 * std::pow(std::abs(x1c + 0.3), 1.3) - 0.12 * x3c;
        break;
    default:
        // Shouldn't happen if bounds are correct
        s1 = x1c;
        break;
    }

    // ---- s2 = s(x2^cat, x2^con, x4^con) ----
    double s2 = 0.0;
    switch (x2cat)
    {
    case 0: // A
        s2 = x2c + 0.35 * std::atan(x2c) + 0.20 * x4c;
        break;
    case 1: // B
        s2 = x2c + 0.35 * std::atan(x2c - 0.6) + 0.18 * x4c;
        break;
    case 2: // C
        s2 = x2c + 0.35 * std::atan(x2c + 0.6) + 0.16 * x4c;
        break;
    case 3: // D
        s2 = x2c + 0.22 * sgn(x2c) * std::sqrt(std::abs(x2c)) - 0.12 * x4c;
        break;
    case 4: // E
        s2 = x2c + 0.22 * sgn(x2c) * std::sqrt(std::abs(x2c + 0.4)) - 0.10 * x4c;
        break;
    case 5: // F
        s2 = x2c + 0.22 * sgn(x2c) * std::sqrt(std::abs(x2c - 0.4)) - 0.11 * x4c;
        break;
    case 6: // G
        s2 = x2c - 0.28 * std::log(1.0 + x2c * x2c) + 0.15 * x4c;
        break;
    case 7: // H
        s2 = x2c - 0.28 * std::log(1.0 + std::pow(x2c - 0.4, 2.0)) + 0.13 * x4c;
        break;
    default:
        // Shouldn't happen if bounds are correct
        s2 = x2c;
        break;
    }

    // ---- Objective ----
    const double f =
        std::sin(s1 + s2)
        + std::pow(s1 - s2, 2.0)
        - 1.5 * s1
        + 2.5 * s2
        + 1.0
        + 0.06 * std::abs(s1 + static_cast<double>(x1i) / 10.0)
        + 0.04 * std::abs(s2 - static_cast<double>(x2i) / 10.0)
        + 0.03 * std::abs(s1 - s2 + static_cast<double>(x3i) / 10.0);

    // ---- Return to NOMAD ----
    x.setBBO(NOMAD::Double(f).tostring());
    countEval = true;
    return true;
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

    // Bounds for all variables
    auto lb = NOMAD::ArrayOfDouble(N, 0.0);
    auto ub = NOMAD::ArrayOfDouble(N, 0.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 7; 
    ub[1] = 7;
    // Integer lower bounds
    lb[Ncat+0] = -10;
    lb[Ncat+1] = -10; 
    lb[Ncat+2] = -10; 
    // Integer upper bounds
    ub[Ncat+0] = 10;
    ub[Ncat+1] = 10;
    ub[Ncat+2] = 10;
    // Integer lower bounds
    lb[Ncat+Nint+0] = -4;
    lb[Ncat+Nint+1] = -4; 
    lb[Ncat+Nint+2] = -2;
    lb[Ncat+Nint+3] = -2;  
    // Integer upper bounds
    ub[Ncat+Nint+0] = 4;
    ub[Ncat+Nint+1] = 4;
    ub[Ncat+Nint+2] = 2;
    ub[Ncat+Nint+3] = 2;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {2,3,4, 5,6,7,8}; // quantitative variables
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
    // TODO: here file history
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("mccormick.txt bbe sol obj cons_h"));

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
