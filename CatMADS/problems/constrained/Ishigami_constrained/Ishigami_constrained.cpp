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
const int Lcat=49;
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
    // Assumed ordering:
    // [cat vars][int vars][cont vars]
    //
    // Cat-cstrs-30:
    //   n^cat = 2 : x1^cat, x2^cat in {A..G} (encoded 0..6)
    //   n^int = 1 : x^int in {1..20}
    //   n^con = 7 : x1^con..x7^con in [-1,1]

    // ----------------------------
    // Decode variables
    // ----------------------------
    const int c1 = static_cast<int>(x[0].todouble()); // 0..6
    const int c2 = static_cast<int>(x[1].todouble()); // 0..6

    const int xi = static_cast<int>(x[Ncat + 0].todouble()); // 1..20

    if (c1 < 0 || c1 > 6 || c2 < 0 || c2 > 6){
        throw NOMAD::Exception(__FILE__, __LINE__, "Categorical index out of range 0..6.");
    }

    if (xi < 1 || xi > 20){
        throw NOMAD::Exception(__FILE__, __LINE__, "Integer variable out of range 1..20.");
    }

    const double x1 = x[Ncat + Nint + 0].todouble(); // x1^con
    const double x2 = x[Ncat + Nint + 1].todouble(); // x2^con
    const double x3 = x[Ncat + Nint + 2].todouble(); // x3^con
    const double x4 = x[Ncat + Nint + 3].todouble(); // x4^con
    const double x5 = x[Ncat + Nint + 4].todouble(); // x5^con
    const double x6 = x[Ncat + Nint + 5].todouble(); // x6^con
    const double x7 = x[Ncat + Nint + 6].todouble(); // x7^con

    // ----------------------------
    // Categorical mapping s(c1,c2)
    // Table given as: rows = x2^cat (A..G), cols = x1^cat (A..G)
    // with A->0, ..., G->6
    // ----------------------------
    static const double Smap[7][7] = {
        /* x2=A */ {0.40, 0.66, 0.91, 1.06, 0.89, 0.61, 0.37},
        /* x2=B */ {0.56, 0.81, 1.07, 1.26, 1.09, 0.78, 0.52},
        /* x2=C */ {0.71, 0.97, 1.22, 1.43, 1.24, 0.94, 0.69},
        /* x2=D */ {0.91, 1.17, 1.43, 1.67, 1.47, 1.14, 0.85},
        /* x2=E */ {1.12, 1.37, 1.63, 1.92, 1.70, 1.31, 1.03},
        /* x2=F */ {1.36, 1.61, 1.94, 2.23, 1.96, 1.57, 1.26},
        /* x2=G */ {1.61, 1.93, 2.24, 2.53, 2.22, 1.82, 1.51}
    };

    const double s = Smap[c2][c1];

    // ----------------------------
    // Objective
    // ----------------------------
    const double pi = M_PI;

    const double term1 =
        (s * s) *
        ( std::sin(pi * x1)
          + 7.0 * std::pow(std::sin(pi * x2), 2.0)
          + 0.1 * std::pow(pi * x3, 4.0) * std::sin(pi * x1) );

    const double term2 =
        s *
        ( (x4 * x4 + x5 * x5 - 0.5)
          + (static_cast<double>(xi) / 20.0) * std::sin(4.0 * (x4 + x5)) );

    const double term3 =
        0.3 *
        ( std::pow(x6 * x6 + x7 * x7 - 1.0, 2.0)
          + (static_cast<double>(xi) / 20.0) * std::cos(4.0 * (x6 - x7)) );

    const double f = term1 + term2 + term3;

    // ----------------------------
    // Constraints
    // ----------------------------
    const double g1 =
        std::pow( (x4 * x4 + x5 * x5) - (0.35 + 0.25 * s), 2.0 ) - 0.0064;

    const double g2 =
        std::pow( (x6 * x6 + x7 * x7) - (0.55 + 0.15 * s), 2.0 ) - 0.0100;

    const double expr3 =
        std::sin(pi * x1)
        + 0.5 * std::sin(pi * x2)
        + 0.25 * x3
        + (static_cast<double>(xi) - 10.0) / 20.0
        - 0.15 * s;

    const double g3 = std::pow(expr3, 2.0) - 0.0064;

    // Set BBO output: "f g1 g2 g3"
    std::string bbo =
        NOMAD::Double(f).tostring()
        + " " + NOMAD::Double(g1).tostring()
        + " " + NOMAD::Double(g2).tostring()
        + " " + NOMAD::Double(g3).tostring();

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
    auto lb = NOMAD::ArrayOfDouble(N, -1.0);
    auto ub = NOMAD::ArrayOfDouble(N,  1.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 6; 
    ub[1] = 6;
    // Integer lower bounds
    lb[Ncat+0] = 1; 
    // Integer upper bounds
    ub[Ncat+0] = 20;
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("ishigami_constrained.txt bbe sol obj cons_h"));

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


    auto dim = params->getAttributeValue<size_t>("DIMENSION");
    std::cout << "DIM=" << dim << "\n";

    auto lb = params->getAttributeValue<NOMAD::ArrayOfDouble>("LOWER_BOUND");
    auto ub = params->getAttributeValue<NOMAD::ArrayOfDouble>("UPPER_BOUND");
    std::cout << "LB size=" << lb.size() << " UB size=" << ub.size() << "\n";

    auto x0 = params->getAttributeValue<NOMAD::Point>("X0"); // or EvalPoint depending
    std::cout << "X0 size=" << x0.size() << "\n";

    auto bb = params->getAttributeValue<std::vector<NOMAD::BBInputType>>("BB_INPUT_TYPE");
    std::cout << "BB_INPUT_TYPE size=" << bb.size() << "\n";

    TheMainStep.run();
    TheMainStep.end();

    return 0;
}
