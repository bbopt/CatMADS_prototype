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
const int Nint=1;
const int Ncon=7;
const int N=Ncat+Nint+Ncon;
const int Lcat=100;
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

    if (x.size() != (Ncat + Nint + Ncon))
    {
        throw NOMAD::Exception(__FILE__, __LINE__,
                               "Dimension mismatch: expected Ncat + Nint + Ncon variables.");
    }

    // Categorical (0..9 for A..J)
    const int x_cat1 = static_cast<int>(x[0].todouble());
    const int x_cat2 = static_cast<int>(x[1].todouble());

    // Integer (0..17)
    const int x_int = static_cast<int>(x[2].todouble());

    // Continuous (7 vars)
    std::vector<double> xc(Ncon);
    for (int i = 0; i < Ncon; ++i)
        xc[i] = x[Ncat + Nint + i].todouble();

    const double x1 = xc[0], x2 = xc[1], x3 = xc[2], x4 = xc[3], x5 = xc[4], x6 = xc[5], x7 = xc[6];

    static const double alpha[4] = {1.0, 1.2, 3.0, 3.2};

    static const double A[4][6] = {
        {10.0, 3.0, 17.0, 3.5, 1.7, 8.0},
        {0.05, 10.0, 17.0, 0.1, 8.0, 14.0},
        {3.0, 3.5, 1.7, 10.0, 17.0, 8.0},
        {17.0, 8.0, 0.05, 10.0, 0.1, 14.0}};

    static const double P[4][6] = {
        {0.1312, 0.1696, 0.5569, 0.0124, 0.8283, 0.5886},
        {0.2329, 0.4135, 0.8307, 0.3736, 0.1004, 0.9991},
        {0.2348, 0.1415, 0.3522, 0.2883, 0.3047, 0.6650},
        {0.4047, 0.8828, 0.8732, 0.5743, 0.1091, 0.0381}};

    // s-table: rows = x2^{cat}, cols = x1^{cat}
    static const double s_table[10][10] = {
        {0.41, 0.52, 0.59, 0.70, 0.76, 0.79, 0.73, 0.68, 0.61, 0.47},
        {0.50, 0.61, 0.71, 0.81, 0.88, 0.92, 0.87, 0.80, 0.69, 0.57},
        {0.57, 0.69, 0.81, 0.93, 1.01, 1.05, 0.99, 0.91, 0.79, 0.65},
        {0.64, 0.77, 0.89, 1.04, 1.13, 1.18, 1.11, 1.03, 0.91, 0.73},
        {0.71, 0.86, 1.01, 1.16, 1.26, 1.31, 1.24, 1.14, 0.99, 0.81},
        {0.78, 0.95, 1.11, 1.27, 1.38, 1.44, 1.37, 1.26, 1.09, 0.89},
        {0.85, 1.03, 1.21, 1.39, 1.51, 1.57, 1.49, 1.37, 1.21, 0.97},
        {0.92, 1.12, 1.31, 1.50, 1.63, 1.70, 1.62, 1.49, 1.29, 1.05},
        {0.99, 1.20, 1.41, 1.62, 1.76, 1.83, 1.74, 1.60, 1.39, 1.13},
        {1.06, 1.29, 1.51, 1.73, 1.88, 1.96, 1.87, 1.72, 1.49, 1.21}};

    const double s = s_table[x_cat2][x_cat1];

    const double xvec6[6] = {x1, x2, x3, x4, x5, x6};

    double hartmann = 0.0;
    for (int i = 0; i < 4; ++i)
    {
        double inner = 0.0;
        for (int j = 0; j < 6; ++j)
        {
            const double d = xvec6[j] - P[i][j];
            inner += A[i][j] * d * d;
        }
        hartmann += alpha[i] * std::exp(-inner);
    }

    const double main_bracket = -hartmann;
    const double scale = s * (1.0 + 0.02 * static_cast<double>(x_int));

    const double pi = M_PI;
    const double phase = 4.0 * pi * x7 + (2.0 * pi * static_cast<double>(x_int)) / 17.0;
    const double term2 = 0.1 * s * std::sin(phase);

    const double f = scale * main_bracket + term2;

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
    auto lb = NOMAD::ArrayOfDouble(N, 0.0);
    auto ub = NOMAD::ArrayOfDouble(N, 1.5);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 9; 
    ub[1] = 9;
    // Integer lower bounds
    lb[Ncat+0] = 0; 
    // Integer upper bounds
    ub[Ncat+0] = 17;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, // categorical variables
    NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {2,  3,4,5,6,7,8,9}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("hartmann.txt bbe sol obj cons_h"));

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
