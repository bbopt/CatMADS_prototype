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
const int Ncon=4;
const int N=Ncat+Nint+Ncon;
const int Lcat=9;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB};
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
    // Ensure the input dimension matches the expected size
    if (x.size() != Ncat + Nint + Ncon) // n_cat + n_int + n_cont
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Dimension mismatch: Ensure the number of variables matches Ncat + Nint + Ncon.");
    }

    // Extract categorical variables
    std::vector<int> x_cat(Ncat);
    for (int i = 0; i < Ncat; ++i)
    {
        x_cat[i] = static_cast<int>(x[i].todouble());
    }

    // Extract integer variables
    std::vector<int> x_int(Nint);
    for (int i = 0; i < Nint; ++i)
    {
        x_int[i] = static_cast<int>(x[Ncat + i].todouble());
    }

    // Extract continuous variables
    std::vector<double> x_con(Ncon);
    for (int i = 0; i < Ncon; ++i)
    {
        x_con[i] = x[Ncat + Nint + i].todouble();
    }

    // Helper functions: p and h
    auto compute_p = [&](int cat1, double x2_con, double x3_con) -> double {
        switch (cat1)
        {
        case 0: return std::sqrt(std::abs(x2_con + x3_con) + 2.0); // Case A
        case 1: return std::abs(x2_con + x3_con);                  // Case B
        case 2: return (std::pow(x2_con + x3_con, 2) / 1.25) + 1.0; // Case C
        default: return 0.0;
        }
    };

    auto compute_h = [&](int cat2, int x1_int, double x1_con, double x4_con) -> double {
        switch (cat2)
        {
        case 0: return std::sqrt(std::abs(x1_int + x1_con + x4_con) + 1.5); // Case A
        case 1: return std::abs(x1_int + x1_con + x4_con);                  // Case B
        case 2: return ((x1_int + std::pow(x1_con + x4_con, 2)) / 1.25) + 1.0; // Case C
        default: return 0.0;
        }
    };

    // Compute the objective function
    double p_val = compute_p(x_cat[0], x_con[1], x_con[2]);
    double h_val = compute_h(x_cat[1], x_int[0], x_con[0], x_con[3]);
    double f = 100 * std::sqrt(std::abs(p_val - 0.01 * h_val)) + 0.01 * std::abs(h_val + x_int[1]);

    // Compute constraints g1 and g2
    double g1, g2;
    if (x_cat[0] == 0) { // Case A for x_cat[0]
        g1 = 1.5 * sin((x_con[0] + x_con[3]) / 5.0) + 0.5 * x_int[0] - 0.1 * std::pow(x_con[0], 2) - 2.0;
    } else if (x_cat[0] == 1) { // Case B for x_cat[0]
        g1 = 1.5 * sin((x_con[0] - x_con[3]) / 5.0) + 0.3 * x_int[0] - 0.1 * std::pow(x_con[3], 2) - 2.5;
    } else { // Case C for x_cat[0]
        g1 = std::exp(0.2 * x_con[0] + 0.1 * x_con[3]) + 0.1 * x_int[0] - 4.0;
    }

    if (x_cat[1] == 0) { // Case A for x_cat[1]
        g2 = 1.5 * cos((x_con[1] + x_con[2]) / 5.0) + 0.4 * x_int[1] - 0.2 * std::pow(x_con[1], 2) - 1.5;
    } else if (x_cat[1] == 1) { // Case B for x_cat[1]
        g2 = 1.5 * cos((x_con[1] - x_con[2]) / 5.0) + 0.6 * x_int[1] - 0.2 * std::pow(x_con[2], 2) - 2.0;
    } else { // Case C for x_cat[1]
        g2 = std::log(1.0 + std::pow(x_con[1] + x_con[2], 2)) + 0.2 * x_int[1] - 3.0;
    }

    // Convert to NOMAD format
    NOMAD::Double F(f);
    NOMAD::Double G1(g1);
    NOMAD::Double G2(g2);

    // Format the output as a string
    std::string bbo = F.tostring() + " " + G1.tostring() + " " + G2.tostring();
    x.setBBO(bbo);

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
    auto lb = NOMAD::ArrayOfDouble(N, 0.0);
    auto ub = NOMAD::ArrayOfDouble(N, 0.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 2; 
    ub[1] = 2;
    // Integer lower bounds
    lb[Ncat+0] = -5; 
    lb[Ncat+1] = -5;
    // Integer upper bounds
    ub[Ncat+0] = 5; 
    ub[Ncat+1] = 5;
    // Continuous lower bounds
    lb[Ncat+Nint+0] = -15.0;
    lb[Ncat+Nint+1] = -3;
    lb[Ncat+Nint+2] = -15;
    lb[Ncat+Nint+3] = -3;
    // Continuous upper bounds
    ub[Ncat+Nint+0] = 5.0;
    ub[Ncat+Nint+1] = 3;
    ub[Ncat+Nint+2] = 5;
    ub[Ncat+Nint+3] = 3;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {2,3, 4,5,6,7}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("bukin6_constrained.txt bbe sol obj cons_h"));

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
