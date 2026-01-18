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



bool My_Evaluator::eval_x(NOMAD::EvalPoint &x,
                          const NOMAD::Double &hMax,
                          bool &countEval) const
{
    if (x.size() != Ncat + Nint + Ncon)
    {
        throw NOMAD::Exception(__FILE__, __LINE__,
                               "Dimension mismatch: expected Ncat + Nint + Ncon.");
    }

    // Categorical: x^{cat} in {A..M} encoded as 0..12
    std::vector<int> x_cat(Ncat);
    for (int i = 0; i < Ncat; ++i)
        x_cat[i] = static_cast<int>(x[i].todouble());

    // Integer: x_1^{int}..x_10^{int}
    std::vector<int> x_int(Nint);
    for (int i = 0; i < Nint; ++i)
        x_int[i] = static_cast<int>(x[Ncat + i].todouble());

    // Continuous: x_1^{con}..x_10^{con}
    std::vector<double> x_con(Ncon);
    for (int i = 0; i < Ncon; ++i)
        x_con[i] = x[Ncat + Nint + i].todouble();

    // Unpack continuous variables (LaTeX: x_1^{continuous}..x_10^{continuous})
    const double xc1  = x_con[0];
    const double xc2  = x_con[1];
    const double xc3  = x_con[2];
    const double xc4  = x_con[3];
    const double xc5  = x_con[4];
    const double xc6  = x_con[5];
    const double xc7  = x_con[6];
    const double xc8  = x_con[7];
    const double xc9  = x_con[8];
    const double xc10 = x_con[9];

    // Unpack integer variables (LaTeX: x_1^{integer}..x_10^{integer})
    const int xi1  = x_int[0];
    const int xi2  = x_int[1];
    const int xi3  = x_int[2];
    const int xi4  = x_int[3];
    const int xi5  = x_int[4];
    const int xi6  = x_int[5];
    const int xi7  = x_int[6];
    const int xi8  = x_int[7];
    const int xi9  = x_int[8];
    const int xi10 = x_int[9];

    const int cat = x_cat[0]; // 0..12 for A..M

    // f1(x)  (same as LaTeX)
    const double f1 =
        (xc1 * xc1) + (xc2 * xc2) + (xc1 * xc2)
        - 14.0 * xc1 - 16.0 * xc2
        + std::pow(xc3 - 10.0, 2.0)
        + 4.0 * std::pow(xc4 - 5.0, 2.0)
        + std::pow(xc5 - 3.0, 2.0)
        + 2.0 * std::pow(xc6 - 1.0, 2.0)
        + 5.0 * (xc7 * xc7)
        + 7.0 * std::pow(xc8 - 11.0, 2.0)
        + 2.0 * std::pow(xc9 - 10.0, 2.0)
        + std::pow(xc10 - 7.0, 2.0)
        + std::pow(static_cast<double>(xi1) - 9.0, 2.0)
        + 10.0 * std::pow(static_cast<double>(xi2) - 1.0, 2.0)
        + 5.0 * std::pow(static_cast<double>(xi3) - 7.0, 2.0)
        + 4.0 * std::pow(static_cast<double>(xi4) - 14.0, 2.0)
        + 27.0 * std::pow(static_cast<double>(xi5) - 1.0, 2.0)
        + std::pow(static_cast<double>(xi6), 2.0)
        + std::pow(static_cast<double>(xi7) - 2.0, 2.0)
        + 13.0 * std::pow(static_cast<double>(xi8) - 2.0, 2.0)
        + std::pow(static_cast<double>(xi9) - 3.0, 2.0)
        + std::pow(static_cast<double>(xi10), 2.0)
        + 95.0
        + 0.30 * std::sin(xc1)
        + 0.25 * std::cos(0.5 * xc4)
        + 0.20 * std::sin(0.3 * xc9)
        + 0.08 * std::abs(xc5 - 3.0) * std::abs(xc6 - 1.0)
        + 0.05 * std::abs(static_cast<double>(xi3) - 7.0) * std::abs(static_cast<double>(xi4) - 14.0)
        + 0.03 * (xc7 - 0.5 * xc8) * (static_cast<double>(xi1) - 9.0)
        + 0.02 * (xc3 - 10.0) * (static_cast<double>(xi2) - 1.0);

    // s(x^{cat}, x) (same as LaTeX): s = 10 * s_inner
    double s_inner = 0.0;
    switch (cat)
    {
        case 0: // A
            s_inner = 3.0 * std::pow(xc1 - 2.0, 2.0)
                    + 4.0 * std::pow(xc2 - 3.0, 2.0)
                    + 2.0 * std::pow(xc3, 2.0)
                    - 7.0 * xc4
                    - 120.0;
            break;
        case 1: // B
            s_inner = 5.0 * std::pow(xc1, 2.0)
                    + 8.0 * xc2
                    + std::pow(xc3 - 6.0, 2.0)
                    - 2.0 * xc4
                    - 40.0;
            break;
        case 2: // C
            s_inner = 0.5 * std::pow(xc1 - 8.0, 2.0)
                    + 2.0 * std::pow(xc2 - 4.0, 2.0)
                    + 3.0 * std::pow(xc5, 2.0)
                    - xc6
                    - 30.0;
            break;
        case 3: // D
            s_inner = std::pow(xc1, 2.0)
                    + 2.0 * std::pow(xc2 - 2.0, 2.0)
                    - 2.0 * xc1 * xc2
                    + 14.0 * xc5
                    - 6.0 * xc6;
            break;
        case 4: // E
            s_inner = -3.0 * xc1
                    + 6.0 * xc2
                    + 12.0 * std::pow(xc8 - 8.0, 2.0)
                    - 7.0 * xc10;
            break;
        case 5: // F
            s_inner = std::pow(xc1, 2.0)
                    + 5.0 * xc1
                    - 8.0 * xc2
                    - 28.0;
            break;
        case 6: // G
            s_inner = 4.0 * xc1
                    + 9.0 * xc2
                    + 5.0 * std::pow(static_cast<double>(xi3), 2.0)
                    - 9.0 * static_cast<double>(xi4)
                    - 87.0;
            break;
        case 7: // H
            s_inner = 3.0 * xc1
                    + 4.0 * xc2
                    + 3.0 * std::pow(static_cast<double>(xi3) - 6.0, 2.0)
                    - 14.0 * static_cast<double>(xi4)
                    - 10.0;
            break;
        case 8: // I
            s_inner = 14.0 * std::pow(static_cast<double>(xi2), 2.0)
                    + 35.0 * static_cast<double>(xi5)
                    - 79.0 * static_cast<double>(xi6)
                    - 92.0;
            break;
        case 9: // J
            s_inner = 15.0 * std::pow(static_cast<double>(xi5), 2.0)
                    + 11.0 * static_cast<double>(xi5)
                    - 61.0 * static_cast<double>(xi6)
                    - 54.0;
            break;
        case 10: // K
            s_inner = 5.0 * std::pow(xc1, 2.0)
                    + 2.0 * xc2
                    + 9.0 * std::pow(static_cast<double>(xi7), 4.0)
                    - static_cast<double>(xi8)
                    - 68.0;
            break;
        case 11: // L
            s_inner = std::pow(xc1, 2.0)
                    - xc9
                    + 19.0 * static_cast<double>(xi9)
                    - 20.0 * static_cast<double>(xi10)
                    + 19.0;
            break;
        case 12: // M
            s_inner = 12.0 * std::pow(xc2, 2.0)
                    + std::pow(xc9, 2.0)
                    - 30.0 * static_cast<double>(xi10);
            break;
        default:
            s_inner = 0.0;
            break;
    }
    const double s = 10.0 * s_inner;

    const double f = f1 + s;

    // Constraints (match LaTeX exactly)
    const double g1 =
        4.0 * xc1 + 5.0 * xc2 - 3.0 * xc7 + 9.0 * xc8 - 105.0
        + 0.15 * (xc1 * xc1) + 0.05 * std::abs(static_cast<double>(xi1));

    const double g2 =
        10.0 * xc1 - 8.0 * xc2 - 17.0 * xc7 + 2.0 * xc8
        + 0.10 * std::pow(xc2 - 1.0, 2.0) + 0.03 * std::pow(static_cast<double>(xi2) - 1.0, 2.0);

    const double g3 =
        -8.0 * xc1 + 2.0 * xc2 + 5.0 * xc9 - 2.0 * xc10 - 12.0
        + 0.06 * std::pow(xc9 - 8.0, 2.0) + 0.02 * std::abs(static_cast<double>(xi3) - 6.0);

    const double g4 =
        xc1 + xc2 + 4.0 * static_cast<double>(xi1) - 21.0 * static_cast<double>(xi2)
        + 0.08 * std::pow(xc3 - 10.0, 2.0) + 0.05 * std::abs(static_cast<double>(xi4) - 14.0);

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
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);


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
