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
const int Nint=4;
const int Ncon=4;
const int N=Ncat+Nint+Ncon;
const int Lcat=4;
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
    // Ensure the input dimension matches the expected size
    if (x.size() != Ncat + Nint + Ncon) // n_cat + n_int + n_cont
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Dimension mismatch: Ensure the number of variables matches Ncat + Nint + Ncon.");
    }

    // Extract categorical variable
    int x_cat = static_cast<int>(x[0].todouble());

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

    // Objective function
    double f = -(x_con[0] + x_con[1] + x_con[2]); // - (x_1^con + x_2^con + x_3^con)

    if (x_cat == 1) // "B"
    {
        f -= 1e5 * (833.33252 * x_con[3] / (x_con[0] * x_int[1]) +
                    100 / x_int[1] -
                    83333.333 / (x_con[0] * x_int[1]) -
                    1);
    }
    else if (x_cat == 2) // "C"
    {
        f -= 1e5 * (x_con[3] / x_int[2] +
                    1250 * (x_int[0] - x_con[3]) / (x_con[1] * x_int[2]) -
                    1);
    }
    else if (x_cat == 3) // "D"
    {
        f -= 1e5 * (1250000 / (x_con[2] * x_int[3]) +
                    x_int[0] / x_int[3] -
                    2500 * x_int[0] / (x_con[2] * x_int[3]) -
                    1);
    }

    // Constraints
    double g1 = 0.0025 * (x_int[1] + x_con[3]) - 1;                // g1 = 0.0025 * (x_2^int + x_4^con) - 1
    double g2 = 0.0025 * (x_int[0] + x_int[2] + x_con[3]) - 1;    // g2 = 0.0025 * (x_1^int + x_3^int + x_4^con) - 1
    double g3 = 0.01 * (x_int[3] - x_int[0]) - 1;                // g3 = 0.01 * (x_4^int - x_1^int) - 1

    // Convert constraints and objective to NOMAD format
    NOMAD::Double F(f);
    NOMAD::Double G1(g1);
    NOMAD::Double G2(g2);
    NOMAD::Double G3(g3);

    // Format the output as a string
    std::string bbo = F.tostring() + " " + G1.tostring() + " " + G2.tostring() + " " + G3.tostring();
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
    // Categorical upper bounds
    ub[0] = 3; 
    // Integer lower bounds
    lb[Ncat+0] = 10; 
    lb[Ncat+1] = 10;
    lb[Ncat+2] = 10;
    lb[Ncat+3] = 10;
    // Integer upper bounds
    ub[Ncat+0] = 1000; 
    ub[Ncat+1] = 1000;
    ub[Ncat+2] = 1000; 
    ub[Ncat+3] = 1000;
    // Continuous lower bounds
    lb[Ncat+Nint+0] = 100;
    lb[Ncat+Nint+1] = 1000;
    lb[Ncat+Nint+2] = 1000;
    lb[Ncat+Nint+3] = 10;
    // Continuous upper bounds
    ub[Ncat+Nint+0] = 10000;
    ub[Ncat+Nint+1] = 10000;
    ub[Ncat+Nint+2] = 10000;
    ub[Ncat+Nint+3] = 1000;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER,  // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0}; // categorical variables
    NOMAD::VariableGroup vg1 = {1,2,3,4, 5,6,7,8}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("dembo5_constrained.txt bbe sol obj cons_h"));

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
