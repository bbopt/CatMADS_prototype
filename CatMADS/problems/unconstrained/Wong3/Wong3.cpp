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
const int Lcat=18;
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

    // Expect: Ncat = 1, Nint = 10, Ncon = 10
    if (x.size() != (Ncat + Nint + Ncon))
    {
        throw NOMAD::Exception(__FILE__, __LINE__,
                               "Dimension mismatch: expected Ncat + Nint + Ncon variables.");
    }

    // ---- Categorical variable (encoded as 0..17 for A..R) ----
    const int x_cat = static_cast<int>(x[0].todouble());

    // ---- Integer variables (10) ----
    std::vector<int> xi(Nint);
    for (int i = 0; i < Nint; ++i)
    {
        xi[i] = static_cast<int>(x[Ncat + i].todouble()); // starts at 1
    }

    // ---- Continuous variables (10) ----
    std::vector<double> xc(Ncon);
    for (int j = 0; j < Ncon; ++j)
    {
        xc[j] = x[Ncat + Nint + j].todouble(); // starts at 11
    }

    // Shorthand accessors (1-based in LaTeX)
    auto I = [&](int idx1) -> double { return static_cast<double>(xi[idx1 - 1]); };
    auto C = [&](int idx1) -> double { return xc[idx1 - 1]; };

    // ---- g(x^int, x^con) ----
    double g = 0.0;

    g += std::abs(I(1)) + std::abs(C(1)) + I(1) * C(1) - 14.0 * I(1) - 16.0 * C(1);
    g += std::pow(I(2) - 10.0, 2) + 4.0 * std::pow(C(2) - 5.0, 2);

    g += std::abs(I(3) - 3.0) + 2.0 * std::abs(C(3) - 1.0);
    g += 5.0 * std::pow(I(4), 2) + 7.0 * std::pow(C(4) - 11.0, 2);

    g += 2.0 * std::abs(I(5) - 10.0) + std::abs(C(5) - 7.0);
    g += std::pow(I(6) - 9.0, 2) + 10.0 * std::pow(C(6) - 1.0, 2);

    g += 5.0 * std::abs(I(7) - 7.0) + 4.0 * std::abs(C(7) - 14.0);
    g += 27.0 * std::pow(I(8) - 1.0, 2) + std::pow(C(8), 4);

    g += std::abs(I(9) - 2.0) + 13.0 * std::abs(C(9) - 2.0);
    g += std::pow(I(10) - 3.0, 2) + std::pow(C(10), 2);

    g += 95.0;

    // ---- s(x^cat, x^int, x^con) ----
    double s_case = 0.0;

    switch (x_cat)
    {
    case 0: // A
        s_case = 0.0;
        break;

    case 1: // B
        s_case =
            3.0 * std::pow(I(1) - 2.0, 2)
            + 4.0 * std::pow(C(1) - 3.0, 2)
            + 2.0 * std::pow(I(2), 2)
            - 7.0 * C(2)
            - 120.0;
        break;

    case 2: // C
        s_case =
            5.0 * std::pow(I(1), 2)
            + 8.0 * C(1)
            + std::pow(I(2) - 6.0, 2)
            - 2.0 * C(2)
            - 40.0;
        break;

    case 3: // D
        s_case =
            0.5 * std::pow(I(1) - 8.0, 2)
            + 2.0 * std::pow(C(1) - 4.0, 2)
            + 3.0 * std::pow(I(3), 2)
            - C(3)
            - 30.0;
        break;

    case 4: // E
        s_case =
            std::pow(I(1), 2)
            + 2.0 * std::pow(C(1) - 2.0, 2)
            - 2.0 * I(1) * C(1)
            + 14.0 * I(3)
            - 6.0 * C(3);
        break;

    case 5: // F
        s_case =
            4.0 * std::pow(I(2), 2)
            + 5.0 * C(2)
            - 3.0 * I(4)
            + 9.0 * C(4)
            - 105.0;
        break;

    case 6: // G
        s_case =
            10.0 * I(1)
            - 8.0 * C(1)
            - 17.0 * I(4)
            + 2.0 * C(4);
        break;

    case 7: // H
        s_case =
            -3.0 * I(1)
            + 6.0 * C(1)
            + 12.0 * std::pow(I(5) - 8.0, 2)
            - 7.0 * C(5);
        break;

    case 8: // I
        s_case =
            -8.0 * I(1)
            + 2.0 * C(1)
            + 5.0 * I(5)
            - 2.0 * C(5)
            - 12.0;
        break;

    case 9: // J
        s_case =
            I(1)
            + C(1)
            + 4.0 * I(6)
            - 21.0 * C(6);
        break;

    case 10: // K
        s_case =
            std::pow(I(1), 2)
            + 5.0 * I(6)
            - 8.0 * C(6)
            - 28.0;
        break;

    case 11: // L
        s_case =
            4.0 * I(1)
            + 9.0 * C(1)
            + 5.0 * std::pow(I(7), 2)
            - 9.0 * C(7)
            - 87.0;
        break;

    case 12: // M
        s_case =
            3.0 * I(1)
            + 4.0 * C(1)
            + 3.0 * std::pow(I(7) - 6.0, 2)
            - 14.0 * C(7)
            - 10.0;
        break;

    case 13: // N
        s_case =
            14.0 * std::pow(I(1), 2)
            + 35.0 * I(8)
            - 79.0 * C(8)
            - 92.0;
        break;

    case 14: // O
        s_case =
            15.0 * std::pow(C(1), 2)
            + 11.0 * I(8)
            - 61.0 * C(8)
            - 54.0;
        break;

    case 15: // P
        s_case =
            5.0 * std::pow(I(1), 2)
            + 2.0 * C(1)
            + 9.0 * std::pow(I(9), 4)
            - C(9)
            - 68.0;
        break;

    case 16: // Q
        s_case =
            std::pow(I(1), 2)
            - C(1)
            + 19.0 * I(10)
            - 20.0 * C(10)
            + 19.0;
        break;

    case 17: // R
        s_case =
            7.0 * std::pow(I(1), 2)
            + 5.0 * std::pow(C(1), 2)
            + std::pow(I(10), 2)
            - 30.0 * C(10);
        break;

    default:
        // Should not happen if bounds/types are correct
        s_case = 0.0;
        break;
    }

    const double s = 10.0 * s_case;

    // ---- Objective ----
    const double f = g + s;

    // ---- Return to NOMAD ----
    NOMAD::Double F(f);
    x.setBBO(F.tostring());
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
    auto lb = NOMAD::ArrayOfDouble(N, -50.0);
    auto ub = NOMAD::ArrayOfDouble(N,  50.0);
    // Categorical lower bounds
    lb[0] = 0; 
    // Categorical upper bounds
    ub[0] = 17; 
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    // categorical variables
    NOMAD::BBInputType::INTEGER,
    // integer variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    // continuous variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group: TODO
    NOMAD::VariableGroup vg0 = {0}; // categorical variables
    NOMAD::VariableGroup vg1 = {1,2,3,4,5,6,7,8,9,10,  11,12,13,14,15,16,17,18,19,20}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("wong3.txt bbe sol obj cons_h"));

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
