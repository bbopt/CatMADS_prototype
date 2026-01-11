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
const int Nint=2;
const int Ncon=2;
const int N=Ncat+Nint+Ncon;
const int Lcat=36;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ,
                    NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
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
    // Expected ordering (as in previous problems):
    // [cat vars][int vars][cont vars]
    //
    // Here:
    //   n^cat = 2 : x1^cat, x2^cat in {A..F} encoded 0..5
    //   n^int = 2 : x1^int, x2^int in {-2,-1,0,1,2}
    //   n^con = 2 : x1^con, x2^con in [-5,5]

    if (x.size() != Ncat + Nint + Ncon)
        throw NOMAD::Exception(__FILE__, __LINE__, "Dimension mismatch in eval_x.");

    // --- Decode variables ---
    const int x1_cat = static_cast<int>(x[0].todouble());           // 0..5
    const int x2_cat = static_cast<int>(x[1].todouble());           // 0..5
    const int x1_int = static_cast<int>(x[Ncat + 0].todouble());    // -2..2
    const int x2_int = static_cast<int>(x[Ncat + 1].todouble());    // -2..2
    const double x1_con = x[Ncat + Nint + 0].todouble();            // [-5,5]
    const double x2_con = x[Ncat + Nint + 1].todouble();            // [-5,5]

    // --- s1 = s(x1^cat, x1^con) ---
    auto s1_map = [&](int cat, double z) -> double {
        switch (cat)
        {
        case 0: // A
            return z + 0.12 * std::pow(z - 1.0, 2.0) + 0.15 * std::abs(z);
        case 1: // B
            return z + 0.12 * std::pow(z - 1.0, 2.0) + 0.10 * std::abs(z - 0.5);
        case 2: // C
            return z + 0.12 * std::pow(z - 1.0, 2.0) + 0.08 * std::abs(z + 0.5);
        case 3: // D
            return z - 0.10 * std::pow(z + 1.0, 2.0) - 0.12 * std::abs(z);
        case 4: // E
            return z - 0.10 * std::pow(z + 1.0, 2.0) - 0.09 * std::abs(z - 0.5);
        case 5: // F
            return z - 0.10 * std::pow(z + 1.0, 2.0) - 0.07 * std::abs(z + 0.5);
        default:
            return 0.0;
        }
    };

    // --- s2 = s(x2^cat, x2^con) ---
    auto s2_map = [&](int cat, double z) -> double {
        switch (cat)
        {
        case 0: // A
            return 0.9 * std::exp(0.35 * z) + 0.05 * std::abs(z);
        case 1: // B
            return 0.6 * std::exp(0.45 * z) + 0.04 * std::abs(z);
        case 2: // C
            return 0.9 * std::exp(0.35 * z) + 0.06 * std::abs(z - 0.4);
        case 3: // D
            return 0.6 * std::exp(0.45 * z) + 0.05 * std::abs(z - 0.4);
        case 4: // E
            return 0.9 * std::exp(0.35 * z) + 0.06 * std::abs(z + 0.4);
        case 5: // F
            return 0.6 * std::exp(0.45 * z) + 0.05 * std::abs(z + 0.4);
        default:
            return 0.0;
        }
    };

    const double s1 = s1_map(x1_cat, x1_con);
    const double s2 = s2_map(x2_cat, x2_con);

    // --- Objective ---
    const double f =
        2.0 * std::pow(s1, 2.0)
        - 1.05 * std::pow(s1, 4.0)
        + std::pow(s1, 6.0) / 6.0
        + s1 * s2
        + std::pow(s2, 2.0)
        + 0.08 * std::abs(s1)
        + 0.05 * std::abs(s2 - s1);

    // --- Constraints ---
    const double g1 =
        std::pow(x1_con - 1.0, 2.0) / 0.10
        + std::pow(x2_con + 0.6 * s2 - 0.54, 2.0) / 0.06
        + std::pow((static_cast<double>(x1_int) + 2.0 - 2.0 * std::abs(s1)), 2.0) / 9.0
        - 1.0;

    const double g2 =
        (s2 - s1) / 0.10
        + std::abs(static_cast<double>(x1_int)) / 2.0
        + std::abs(static_cast<double>(x2_int)) / 2.0
        - 1.0;

    const double g3 =
        std::pow(s1 - 1.15, 2.0) / 0.05
        + std::pow(s2 - 0.90, 2.0) / 0.05
        + std::abs(static_cast<double>(x1_int + x2_int)) / 6.0
        - 1.0;

    const double sig = 1.0 / (1.0 + std::exp(-1.2 * (x1_con - x2_con)));
    const double g4 =
        std::pow(sig - 0.73, 2.0)
        + std::pow(std::abs(s1) / 3.0, 2.0)
        + std::pow(std::abs(static_cast<double>(x2_int)) / 10.0, 2.0)
        - 0.02;

    const double g5 =
        std::abs(x1_con - x2_con / 10.0 - 1.0)
        + 0.45 * std::abs(s2 - 0.90)
        + 0.10 * std::abs(s1 - 1.15)
        - 0.12;

    const double g6 =
        std::pow(x2_con - 0.40 * s1, 2.0) / 0.20
        + std::pow(x1_con + 0.30 * s2 - 1.27, 2.0) / 0.20
        + 0.015 * std::abs(static_cast<double>(x1_int))
        + 0.015 * std::abs(static_cast<double>(x2_int))
        - 1.0;

    // Set BBO output: "f g1 g2 g3 g4 g5 g6"
    std::string bbo = NOMAD::Double(f).tostring()
        + " " + NOMAD::Double(g1).tostring()
        + " " + NOMAD::Double(g2).tostring()
        + " " + NOMAD::Double(g3).tostring()
        + " " + NOMAD::Double(g4).tostring()
        + " " + NOMAD::Double(g5).tostring()
        + " " + NOMAD::Double(g6).tostring();

    x.setBBO(bbo);

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
    auto lb = NOMAD::ArrayOfDouble(N, -5.0);
    auto ub = NOMAD::ArrayOfDouble(N,  5.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 5; 
    ub[1] = 5;
    // Integer lower bounds
    lb[Ncat+0] = -2; 
    lb[Ncat+1] = -2;
    // Integer upper bounds
    ub[Ncat+0] = 2; 
    ub[Ncat+1] = 2;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {2,3, 4,5}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("three_humps_constrained.txt bbe sol obj cons_h"));

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
