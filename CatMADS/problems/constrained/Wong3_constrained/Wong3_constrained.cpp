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
const int Nint=10;
const int Ncon=10;
const int N=Ncat+Nint+Ncon;
const int Lcat=13;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ,
                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB};
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
    // Dimension check
    if (x.size() != Ncat + Nint + Ncon)
    {
        throw NOMAD::Exception(__FILE__, __LINE__,
                               "Dimension mismatch: expected Ncat + Nint + Ncon.");
    }

    // Extract categorical variables (A..M encoded as 0..12)
    std::vector<int> x_cat(Ncat);
    for (int i = 0; i < Ncat; ++i)
        x_cat[i] = static_cast<int>(x[i].todouble());

    // Extract integer variables (10 vars: correspond to x_11^int..x_20^int in LaTeX)
    std::vector<int> x_int(Nint);
    for (int i = 0; i < Nint; ++i)
        x_int[i] = static_cast<int>(x[Ncat + i].todouble());

    // Extract continuous variables (10 vars: x_1^con..x_10^con)
    std::vector<double> x_con(Ncon);
    for (int i = 0; i < Ncon; ++i)
        x_con[i] = x[Ncat + Nint + i].todouble();

    // Unpack continuous x1..x10
    const double x1  = x_con[0];
    const double x2  = x_con[1];
    const double x3  = x_con[2];
    const double x4  = x_con[3];
    const double x5  = x_con[4];
    const double x6  = x_con[5];
    const double x7  = x_con[6];
    const double x8  = x_con[7];
    const double x9  = x_con[8];
    const double x10 = x_con[9];

    // Unpack integers: i1..i10 correspond to x11..x20
    const int x11 = x_int[0];
    const int x12 = x_int[1];
    const int x13 = x_int[2];
    const int x14 = x_int[3];
    const int x15 = x_int[4];
    const int x16 = x_int[5];
    const int x17 = x_int[6];
    const int x18 = x_int[7];
    const int x19 = x_int[8];
    const int x20 = x_int[9];

    const int cat = x_cat[0]; // 0..12 for A..M

    // f1(x)
    const double f1 =
        (x1 * x1) + (x2 * x2) + (x1 * x2)
        - 14.0 * x1 - 16.0 * x2
        + std::pow(x3 - 10.0, 2.0)
        + 4.0 * std::pow(x4 - 5.0, 2.0)
        + std::pow(x5 - 3.0, 2.0)
        + 2.0 * std::pow(x6 - 1.0, 2.0)
        + 5.0 * (x7 * x7)
        + 7.0 * std::pow(x8 - 11.0, 2.0)
        + 2.0 * std::pow(x9 - 10.0, 2.0)
        + std::pow(x10 - 7.0, 2.0)
        + std::pow(static_cast<double>(x11) - 9.0, 2.0)
        + 10.0 * std::pow(static_cast<double>(x12) - 1.0, 2.0)
        + 5.0 * std::pow(static_cast<double>(x13) - 7.0, 2.0)
        + 4.0 * std::pow(static_cast<double>(x14) - 14.0, 2.0)
        + 27.0 * std::pow(static_cast<double>(x15) - 1.0, 2.0)
        + std::pow(static_cast<double>(x16), 2.0)
        + std::pow(static_cast<double>(x17) - 2.0, 2.0)
        + 13.0 * std::pow(static_cast<double>(x18) - 2.0, 2.0)
        + std::pow(static_cast<double>(x19) - 3.0, 2.0)
        + std::pow(static_cast<double>(x20), 2.0)
        + 95.0
        + 0.30 * std::sin(x1)
        + 0.25 * std::cos(0.5 * x4)
        + 0.20 * std::sin(0.3 * x9)
        + 0.08 * std::abs(x5 - 3.0) * std::abs(x6 - 1.0)
        + 0.05 * std::abs(static_cast<double>(x13) - 7.0) * std::abs(static_cast<double>(x14) - 14.0)
        + 0.03 * (x7 - 0.5 * x8) * (static_cast<double>(x11) - 9.0)
        + 0.02 * (x3 - 10.0) * (static_cast<double>(x12) - 1.0);

    // s(x^cat, x)  (then multiplied by 10)
    double s_inner = 0.0;
    switch (cat)
    {
        case 0: // A
            s_inner = 3.0 * std::pow(x1 - 2.0, 2.0)
                    + 4.0 * std::pow(x2 - 3.0, 2.0)
                    + 2.0 * std::pow(x3, 2.0)
                    - 7.0 * x4
                    - 120.0;
            break;
        case 1: // B
            s_inner = 5.0 * std::pow(x1, 2.0)
                    + 8.0 * x2
                    + std::pow(x3 - 6.0, 2.0)
                    - 2.0 * x4
                    - 40.0;
            break;
        case 2: // C
            s_inner = 0.5 * std::pow(x1 - 8.0, 2.0)
                    + 2.0 * std::pow(x2 - 4.0, 2.0)
                    + 3.0 * std::pow(x5, 2.0)
                    - x6
                    - 30.0;
            break;
        case 3: // D
            s_inner = std::pow(x1, 2.0)
                    + 2.0 * std::pow(x2 - 2.0, 2.0)
                    - 2.0 * x1 * x2
                    + 14.0 * x5
                    - 6.0 * x6;
            break;
        case 4: // E
            s_inner = -3.0 * x1
                    + 6.0 * x2
                    + 12.0 * std::pow(x8 - 8.0, 2.0)
                    - 7.0 * x10;
            break;
        case 5: // F
            s_inner = std::pow(x1, 2.0)
                    + 5.0 * x1
                    - 8.0 * x2
                    - 28.0;
            break;
        case 6: // G
            s_inner = 4.0 * x1
                    + 9.0 * x2
                    + 5.0 * std::pow(static_cast<double>(x13), 2.0)
                    - 9.0 * static_cast<double>(x14)
                    - 87.0;
            break;
        case 7: // H
            s_inner = 3.0 * x1
                    + 4.0 * x2
                    + 3.0 * std::pow(static_cast<double>(x13) - 6.0, 2.0)
                    - 14.0 * static_cast<double>(x14)
                    - 10.0;
            break;
        case 8: // I
            s_inner = 14.0 * std::pow(static_cast<double>(x12), 2.0)
                    + 35.0 * static_cast<double>(x15)
                    - 79.0 * static_cast<double>(x16)
                    - 92.0;
            break;
        case 9: // J
            s_inner = 15.0 * std::pow(static_cast<double>(x15), 2.0)
                    + 11.0 * static_cast<double>(x15)
                    - 61.0 * static_cast<double>(x16)
                    - 54.0;
            break;
        case 10: // K
            s_inner = 5.0 * std::pow(x1, 2.0)
                    + 2.0 * x2
                    + 9.0 * std::pow(static_cast<double>(x17), 4.0)
                    - static_cast<double>(x18)
                    - 68.0;
            break;
        case 11: // L
            s_inner = std::pow(x1, 2.0)
                    - x9
                    + 19.0 * static_cast<double>(x19)
                    - 20.0 * static_cast<double>(x20)
                    + 19.0;
            break;
        case 12: // M
            s_inner = 12.0 * std::pow(x2, 2.0)
                    + std::pow(x9, 2.0)
                    - 30.0 * static_cast<double>(x20);
            break;
        default:
            s_inner = 0.0;
            break;
    }
    const double s = 10.0 * s_inner;

    // Objective
    const double f = f1 + s;

    // Constraints g_i(x) <= 0
    const double g1 =
        4.0 * x1 + 5.0 * x2 - 3.0 * x7 + 9.0 * x8 - 105.0
        + 0.15 * (x1 * x1) + 0.05 * std::abs(static_cast<double>(x11));

    const double g2 =
        10.0 * x1 - 8.0 * x2 - 17.0 * x7 + 2.0 * x8
        + 0.10 * std::pow(x2 - 1.0, 2.0) + 0.03 * std::pow(static_cast<double>(x12) - 1.0, 2.0);

    const double g3 =
        -8.0 * x1 + 2.0 * x2 + 5.0 * x9 - 2.0 * x10 - 12.0
        + 0.06 * std::pow(x9 - 8.0, 2.0) + 0.02 * std::abs(static_cast<double>(x13) - 6.0);

    const double g4 =
        x1 + x2 + 4.0 * static_cast<double>(x11) - 21.0 * static_cast<double>(x12)
        + 0.08 * std::pow(x3 - 10.0, 2.0) + 0.05 * std::abs(static_cast<double>(x14) - 14.0);

    // Set BBO output: "f g1 g2 g3 g4"
    std::string bbo = NOMAD::Double(f).tostring()
        + " " + NOMAD::Double(g1).tostring()
        + " " + NOMAD::Double(g2).tostring()
        + " " + NOMAD::Double(g3).tostring()
        + " " + NOMAD::Double(g4).tostring();

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

    // Bounds for all variables except the first group (categorical variable)
    auto lb = NOMAD::ArrayOfDouble(N, -20.0);
    auto ub = NOMAD::ArrayOfDouble(N,  20.0);
    // Categorical lower bounds
    lb[0] = 0; 
    // Categorical upper bounds
    ub[0] = 12; 
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    // categorical variables
    NOMAD::BBInputType::INTEGER,  
    // integer variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, 
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, 
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    // continuous variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};

    // Variable group: TODO
    NOMAD::VariableGroup vg0 = {0}; // categorical variables
    NOMAD::VariableGroup vg1 = {1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16,17,18,19,20}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("wong3_constrained.txt bbe sol obj cons_h"));

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
