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
const int Lcat=8;
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
    // Extract variables
    int x_cat = static_cast<int>(x[0].todouble()); // Categorical variable
    double x_int1 = 0.0625 * static_cast<int>(x[1].todouble());
    double x_int2 = 0.0625 * static_cast<int>(x[2].todouble());
    double x_con1 = x[3].todouble();
    double x_con2 = x[4].todouble();

    // Helper function for p
    auto compute_p = [&](int x_cat, double x_con2) -> double {
        switch (x_cat) {
            case 0: return 20 + 0.8 * x_con2 + 15 * std::sin(0.02 * x_con2);
            case 1: return 50 + 0.85 * x_con2 + 18 * std::sin(0.025 * x_con2 + 0.3);
            case 2: return 10 + 0.75 * x_con2 + 12 * std::sin(0.018 * x_con2 - 0.4);
            case 3: return 40 + 0.82 * x_con2 + 20 * std::sin(0.022 * x_con2 + 0.5);
            case 4: return 200 - 0.8 * x_con2 - 15 * std::sin(0.02 * x_con2);
            case 5: return 160 - 0.85 * x_con2 - 18 * std::sin(0.025 * x_con2 + 0.3);
            case 6: return 220 - 0.75 * x_con2 - 12 * std::sin(0.018 * x_con2 - 0.4);
            case 7: return 180 - 0.82 * x_con2 - 20 * std::sin(0.022 * x_con2 + 0.5);
            default: throw std::invalid_argument("Invalid categorical variable value.");
        }
    };

    double p_val = compute_p(x_cat, x_con2);

    // Objective function
    double f = 0.6224 * x_int1 * x_con1 * p_val +
               1.7781 * x_int2 * std::pow(p_val, 2) +
               3.1661 * std::pow(x_int1, 2) * x_con1 +
               19.84 * std::pow(x_int1, 2) * p_val;

    // Constraints
    double g1 = -x_int1 + 0.0193 * p_val;
    double g2 = -x_int2 + 0.00954 * p_val;
    double g3 = -M_PI * x_con1 * std::pow(p_val, 2) - (4 * M_PI / 3) * std::pow(p_val, 3) + 1296000;

    // Convert constraints and objective function to NOMAD format
    NOMAD::Double F(f);
    NOMAD::Double G1(g1);
    NOMAD::Double G2(g2);
    NOMAD::Double G3(g3);

    // Assign constraints and objective function
    x.setBBO(F.tostring() + " " + G1.tostring() + " " + G2.tostring() + " " + G3.tostring());

    // Mark evaluation as successful
    countEval = true;
    return true;
};

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
    auto lb = NOMAD::ArrayOfDouble(N, 10);
    auto ub = NOMAD::ArrayOfDouble(N, 200);
    // Categorical lower bounds
    lb[0] = 0; 
    // Categorical upper bounds
    ub[0] = 7; 
    // Integer lower bounds
    lb[Ncat+0] = 1; 
    lb[Ncat+1] = 1;
    // Integer upper bounds
    ub[Ncat+0] = 99; 
    ub[Ncat+1] = 99;
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("pressurevessel_constrained.txt bbe sol obj cons_h"));

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
