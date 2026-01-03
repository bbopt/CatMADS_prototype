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
#include "../../CatMADS.hpp"
#include "../../MyExtendedPoll/MyExtendedPollMethod2.hpp"


// Setup of the problem
const int Ncat=1;
const int Nint=4;
const int Ncon=6;
const int N=Ncat+Nint+Ncon;
const int Lcat=6;
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
        // Extract categorical variables
    int x_cat1 = static_cast<int>(x[0].todouble()); // First categorical variable

    // Extract integer variables
    std::vector<int> x_int(Nint);
    for (int i = 0; i < Nint; ++i) {
        x_int[i] = static_cast<int>(x[Ncat + i].todouble());
    }

    // Extract continuous variables
    std::vector<double> x_con(Ncon);
    for (int i = 0; i < Ncon; ++i) {
        x_con[i] = x[Ncat + Nint + i].todouble();
    }

    // Compute penalty function p
    double p_val = std::pow(x_con[0], 2) + std::pow(x_int[0], 2) + x_con[0] * x_int[0]
                   - 14 * x_con[0] - 16 * x_int[0]
                   + std::pow(x_con[1] - 10, 2) + 4 * std::pow(x_int[1] - 5, 2)
                   + std::pow(x_con[2] - 3, 2) + 2 * std::pow(x_int[2] - 1, 2)
                   + 5 * x_con[3] + 7 * std::pow(x_int[3] - 11, 2)
                   + 2 * std::pow(x_con[4] - 10, 2) + std::pow(x_con[5] - 7, 2) + 45;

    // Compute objective function
    double f = -p_val;
    switch (x_cat1) {
        case 0: // A
            break;
        case 1: // B
            f += 10 * (3 * std::pow(x_con[0] - 2, 2) + 4 * std::pow(x_int[0] - 3, 2)
                       + 2 * std::pow(x_con[1], 2) - 7 * std::pow(x_int[1], 2) - 120);
            break;
        case 2: // C
            f += 10 * (5 * std::pow(x_con[0], 2) + 8 * x_int[0] + 6 * std::pow(x_con[1] - 6, 2)
                       - 2 * x_int[1] - 40);
            break;
        case 3: // D
            f += 10 * (0.5 * std::pow(x_con[0] - 8, 2) + 2 * std::pow(x_int[0] - 4, 2)
                       + 3 * std::pow(x_con[2], 2) - x_int[2] - 30);
            break;
        case 4: // E
            f += 10 * (std::pow(x_con[0], 2) + 2 * std::pow(x_int[0] - 2, 2)
                       - 2 * x_con[0] * x_int[0] + 14 * x_con[2] - 6 * x_int[2]);
            break;
        case 5: // F
            f += 10 * (-3 * x_con[0] + 6 * x_int[0] + 12 * std::pow(x_con[4] - 8, 2)
                       - 7 * x_con[5]);
            break;
        default:
            throw std::invalid_argument("Invalid category index for x_cat1.");
    }

    // Compute constraints
    double g1, g2, g3;
    switch (x_cat1) {
        case 0: // A
            g1 = 4 * x_con[0] + 5 * x_int[0] - 3 * x_con[3] + 9 * x_int[3] - 105;
            g2 = 10 * x_con[0] - 8 * x_int[0] - 17 * x_con[3] + 2 * x_int[3];
            g3 = -8 * x_con[0] + 2 * x_int[0] + 5 * x_con[4] - 2 * x_con[5];
            break;
        case 1: // B
            g1 = 3 * x_con[0] + 6 * x_int[0] - 3 * x_con[3] + 9 * x_int[3] - 105;
            g2 = 8 * x_con[0] - 6 * x_int[0] - 17 * x_con[3] + 2 * x_int[3];
            g3 = -4 * x_con[0] + 4 * x_int[0] + 5 * x_con[4] - 2 * x_con[5];
            break;
        case 2: // C
            g1 = 4 * x_con[0] + 4 * x_int[0] - 2 * x_con[3] + 9 * x_int[3] - 105;
            g2 = 10 * x_con[0] - 10 * x_int[0] - 15 * x_con[3] + 2 * x_int[3];
            g3 = -8 * x_con[0] + 1 * x_int[0] + 10 * x_con[4] - 2 * x_con[5];
            break;
        case 3: // D
            g1 = 4 * x_con[0] + 5 * x_int[0] - 4 * x_con[3] + 10 * x_int[3] - 105;
            g2 = 10 * x_con[0] - 8 * x_int[0] - 19 * x_con[3] + 4 * x_int[3];
            g3 = -8 * x_con[0] + 2 * x_int[0] + 2.5 * x_con[4] - 4 * x_con[5];
            break;
        case 4: // E
            g1 = 5 * x_con[0] + 5 * x_int[0] - 3 * x_con[3] + 8 * x_int[3] - 105;
            g2 = 12 * x_con[0] - 8 * x_int[0] - 19 * x_con[3] + 1 * x_int[3];
            g3 = -16 * x_con[0] + 2 * x_int[0] + 5 * x_con[4] - 1 * x_con[5];
            break;
        case 5: // F
            g1 = 3 * x_con[0] + 4 * x_int[0] - 1 * x_con[3] + 8 * x_int[3] - 95;
            g2 = 9 * x_con[0] - 9 * x_int[0] - 18 * x_con[3] + 1 * x_int[3] + 10;
            g3 = -4 * x_con[0] + 1 * x_int[0] + 10 * x_con[4] - 4 * x_con[5] + 10;
            break;
        default:
            throw std::invalid_argument("Invalid category index for x_cat1.");
    }

    // Convert constraints to NOMAD format
    NOMAD::Double F(f);
    NOMAD::Double G1(g1);
    NOMAD::Double G2(g2);
    NOMAD::Double G3(g3);

    // Assign constraints and objective function
    x.setBBO(F.tostring() + " " + G1.tostring() + " " + G2.tostring() + " " + G3.tostring());

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
    auto lb = NOMAD::ArrayOfDouble(N, 0);
    auto ub = NOMAD::ArrayOfDouble(N, 10);
    // Categorical lower bounds
    lb[0] = 0; 
    // Categorical upper bounds
    ub[0] = 5; 
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER,  // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0}; // categorical variables
    NOMAD::VariableGroup vg1 = {1,2,3,4, 5,6,7,8,9,10}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("wong2_constrained.txt bbe sol obj cons_h"));

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
