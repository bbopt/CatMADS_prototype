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
const int Ncat=1;
const int Nint=2;
const int Ncon=2;
const int N=Ncat+Nint+Ncon;
const int Lcat=5; 
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ,
                                 NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB};
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
        // Extract variables
    int x_cat1 = static_cast<int>(x[0].todouble()); // Categorical variable: "absolute", "quad", etc.
    int x_int1 = static_cast<int>(x[1].todouble()); // Integer variable 1
    int x_int2 = static_cast<int>(x[2].todouble()); // Integer variable 2
    double x_con1 = x[3].todouble();                // Continuous variable 1
    double x_con2 = x[4].todouble();                // Continuous variable 2

    // Compute penalty function p(x_cat1, x_con)
    auto compute_p = [&](int cat, double x) -> double {
        switch (cat) {
            case 0: // "absolute"
                return std::abs(x - 1);
            case 1: // "quad"
                return (std::pow(x, 2) + 1) / 2.0;
            case 2: // "logsum"
                return std::exp(std::abs(x + 1)) - 1;
            case 3: // "hyperbol"
                return std::pow(x, 2) / (1 + std::abs(x));
            case 4: // "invgauss"
                return 1 - std::exp(-std::pow(x, 2));
            default:
                throw NOMAD::Exception(__FILE__, __LINE__, "Invalid category for penalty function.");
        }
    };

    // Objective function
    double f = 0.0;
    for (int i = 0; i < 2; ++i) {
        double x = (i == 0) ? x_con1 : x_con2;
        f += 0.5 * (std::pow(x, 4) - 16 * std::pow(x, 2) + 5 * ((i == 0) ? x_int1 : x_int2) + 8 * compute_p(x_cat1, x));
    }

    // Constraints
    double g1 = 0.0, g2 = 0.0;
    switch (x_cat1) {
        case 0: // "A"
            g1 = std::exp(x_con1 + x_con2) - 10;
            g2 = std::pow(x_con1 + 2, 3) + std::pow(x_con2 - 1, 2) + 0.1 * std::pow(x_int1 - 1, 2) - 50;
            break;
        case 1: // "B"
            g1 = std::exp(x_con1 + 2 * x_con2) - 18;
            g2 = std::pow(x_con1 - 1, 3) + std::pow(x_con2 + 2, 2) + 0.1 * std::pow(x_int1 + 2, 2) - 40;
            break;
        case 2: // "C"
            g1 = std::exp(2 * x_con1 + x_con2) - 18;
            g2 = std::pow(x_con1, 3) + std::pow(x_con2 - 2, 2) + 0.1 * std::pow(x_int1 - 3, 2) - 45;
            break;
        case 3: // "D"
            g1 = std::exp(x_con1 - x_con2) + std::log(1 + std::abs(x_con2)) - 12;
            g2 = std::sin((x_con1 + x_con2) / 10) + std::pow(x_con1, 2) + 0.2 * std::abs(x_int1 - 1) - 3;
            break;
        case 4: // "E"
            g1 = std::exp(x_con1 - 0.5 * x_con2) + std::log(1 + std::abs(x_con2)) - 11;
            g2 = std::sin((x_con1 + x_con2) / 12.5) + std::pow(x_con1, 2) + 0.2 * std::abs(x_int1 - 2) - 4;
            break;
        default:
            throw NOMAD::Exception(__FILE__, __LINE__, "Invalid category for constraints.");
    }

    // Convert constraints to NOMAD format
    NOMAD::Double F(f);
    NOMAD::Double G1(g1);
    NOMAD::Double G2(g2);

    // Assign constraints and objective function
    x.setBBO(F.tostring() + " " + G1.tostring() + " " + G2.tostring());

    // Mark evaluation as successful
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

    // Bounds for all variables except the first group (categorical variable)
    auto lb = NOMAD::ArrayOfDouble(N, -5.0);
    auto ub = NOMAD::ArrayOfDouble(N, 10.0);
    // Categorical lower bounds
    lb[0] = 0; 
    // Categorical upper bounds
    ub[0] = 4; 
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER,  // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0}; // categorical variables
    NOMAD::VariableGroup vg1 = {1,2, 3,4}; // quantitative variables
    allParams->setAttributeValue("VARIABLE_GROUP", NOMAD::ListOfVariableGroup({vg0,vg1}));
    
    // Primary poll in two subpolls
    NOMAD::DirectionTypeList dtList = {NOMAD::DirectionType::USER_FREE_POLL, NOMAD::DirectionType::ORTHO_2N};
    allParams->setAttributeValue("DIRECTION_TYPE",dtList);
    
    // Secondary poll in two subpolls
    NOMAD::DirectionTypeList dtListSec = {NOMAD::DirectionType::USER_FREE_POLL, NOMAD::DirectionType::DOUBLE};
    allParams->setAttributeValue("DIRECTION_TYPE_SECONDARY_POLL",dtListSec);

    // Set the map of direction types and variable group. This is passed to Mads in the main function
    //myMapDirTypeToVG = {{dtList[0],{vg0}},{dtList[1],{vg1}}}; // Before constraints
    myMapDirTypeToVG = {{dtList[0],{vg0}},{dtList[1],{vg1}},{dtListSec[1],{vg1}}};

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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("styblinskitang_constrained.txt bbe sol obj cons_h"));

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
