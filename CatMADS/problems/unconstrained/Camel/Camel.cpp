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
const int Ncat=3;
const int Nint=2;
const int Ncon=4;
const int N=Ncat+Nint+Ncon;
const int Lcat=80;
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
    (void)hMax; // unused (no constraints handled here)

    // Expect: Ncat = 3, Nint = 2, Ncon = 4
    if (x.size() != (Ncat + Nint + Ncon))
    {
        throw NOMAD::Exception(__FILE__, __LINE__,
                               "Dimension mismatch: expected Ncat + Nint + Ncon variables.");
    }

    // ---- Extract categorical variables ----
    // x1^{cat}, x2^{cat} encoded as 0..3 for A..D
    // x3^{cat} encoded as 0..4 for A..E
    const int x_cat1 = static_cast<int>(x[0].todouble());
    const int x_cat2 = static_cast<int>(x[1].todouble());
    const int x_cat3 = static_cast<int>(x[2].todouble());

    // ---- Extract integer variables ----
    const int x_int1 = static_cast<int>(x[3].todouble()); // x1^{integer}
    const int x_int2 = static_cast<int>(x[4].todouble()); // x2^{integer}

    // ---- Extract continuous variables ----
    std::vector<double> xc(Ncon);
    for (int i = 0; i < Ncon; ++i)
    {
        xc[i] = x[Ncat + Nint + i].todouble(); // starts at index 5
    }

    const double x1 = xc[0];
    const double x2 = xc[1];
    const double x3 = xc[2];
    const double x4 = xc[3];

    // ---- s1(x1^{cat}, x2^{cat}) from Table (rows = x2^{cat}, cols = x1^{cat}) ----
    static const double s1_table[4][4] = {
        /* x2=A */ {0.80, 1.10, 0.95, 1.20},
        /* x2=B */ {1.05, 0.85, 1.25, 0.90},
        /* x2=C */ {0.92, 1.18, 1.00, 0.88},
        /* x2=D */ {1.15, 0.98, 0.87, 1.30}};

    const double s1 = s1_table[x_cat2][x_cat1];

    // ---- Integer scaling factor ----
    const double scale = 1.0
                       + 0.03 * (static_cast<double>(x_int1) - 5.0)
                       + 0.02 * (static_cast<double>(x_int2) - 4.0);

    // ---- Six-hump camel-like bracket ----
    // [ (4 - 2.1 x1^2 + (1/3) x1^4) x1^2 + x1 x2 + (-4 + 4 x2^2) x2^2 ]
    const double x1_2 = x1 * x1;
    const double x1_4 = x1_2 * x1_2;
    const double x2_2 = x2 * x2;

    const double camel =
        (4.0 - 2.1 * x1_2 + (1.0 / 3.0) * x1_4) * x1_2
        + x1 * x2
        + (-4.0 + 4.0 * x2_2) * x2_2;

    const double term_camel = s1 * scale * camel;

    // ---- Quadratic regularization term ----
    const double term_quad = 0.2 * (x3 * x3 + (x4 - 1.0) * (x4 - 1.0));

    // ---- Coupling squared term ----
    const double coupling = (x3 * x1 + x4 * x2);
    const double term_coupling = 0.05 * coupling * coupling;

    // ---- Mixed integer-continuous interaction ----
    const double term_mix = 0.01
        * (static_cast<double>(x_int1) - 5.0)
        * (static_cast<double>(x_int2) - 4.0)
        * (x3 + x4);

    // ---- s2(x3^{cat}, x3^{continuous}) ----
    double s2 = 0.0;
    switch (x_cat3)
    {
    case 0: // A
        s2 = std::abs(x3 - 0.5) + 0.05;
        break;
    case 1: // B
        s2 = 1.05 * std::abs(x3 - 0.5) + 0.05;
        break;
    case 2: // C
        s2 = 0.95 * std::abs(x3 - 0.5) + 0.06;
        break;
    case 3: // D
        s2 = 0.70 + 0.30 * std::exp(-0.5 * (x3 + 2.0));
        break;
    case 4: // E
        s2 = 1.00 + 0.40 * std::exp(-0.8 * (x3 + 1.0));
        break;
    }

    const double f = term_camel + term_quad + term_coupling + term_mix + 0.1 * s2;

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
    auto lb = NOMAD::ArrayOfDouble(N, -2.0);
    auto ub = NOMAD::ArrayOfDouble(N,  2.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    lb[2] = 0;
    // Categorical upper bounds
    ub[0] = 3; 
    ub[1] = 3;
    ub[2] = 4;
    // Integer lower bounds
    lb[Ncat+0] = -10; 
    lb[Ncat+1] = -10; 
    // Integer upper bounds
    ub[Ncat+0] = 10;
    ub[Ncat+1] = 10;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group: TODO
    NOMAD::VariableGroup vg0 = {0,1,2}; // categorical variables
    NOMAD::VariableGroup vg1 = {3,4, 5,6,7,8}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("camel.txt bbe sol obj cons_h"));

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
