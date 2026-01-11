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
const int Nint=0;
const int Ncon=6;
const int N=Ncat+Nint+Ncon;
const int Lcat=64;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ,
                    NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
                    NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
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
    // Dimension check
    if (x.size() != Ncat + Nint + Ncon)
    {
        throw NOMAD::Exception(__FILE__, __LINE__,
                               "Dimension mismatch: expected Ncat + Nint + Ncon.");
    }

    // Extract categorical variables (assumed encoded as 0..7 for A..H)
    std::vector<int> x_cat(Ncat);
    for (int i = 0; i < Ncat; ++i)
        x_cat[i] = static_cast<int>(x[i].todouble());

    // Extract integer variables (none here, but keep generic)
    std::vector<int> x_int(Nint);
    for (int i = 0; i < Nint; ++i)
        x_int[i] = static_cast<int>(x[Ncat + i].todouble());

    // Extract continuous variables
    std::vector<double> x_con(Ncon);
    for (int i = 0; i < Ncon; ++i)
        x_con[i] = x[Ncat + Nint + i].todouble();

    // Unpack continuous vars (this problem: n_con = 6)
    const double x1 = x_con[0];
    const double x2 = x_con[1];
    const double x3 = x_con[2];
    const double x4 = x_con[3];
    const double x5 = x_con[4];
    const double x6 = x_con[5];

    // s = s(x1^cat, x2^cat) via table (rows: x2^cat, cols: x1^cat), A..H -> 0..7
    auto s_value = [&](int c1, int c2) -> double {
        static const double S[8][8] = {
            /* x2=A */ {23, 19, 27, 21, 18, 24, 20, 26},
            /* x2=B */ {20, 25, 18, 24, 22, 19, 27, 21},
            /* x2=C */ {26, 22, 24, 19, 27, 21, 18, 23},
            /* x2=D */ {19, 27, 21, 23, 20, 26, 22, 18},
            /* x2=E */ {24, 18, 23, 20, 26, 22, 19, 27},
            /* x2=F */ {21, 23, 20, 26, 22, 18, 24, 19},
            /* x2=G */ {17, 24, 22, 19, 27, 21, 23, 20},
            /* x2=H */ {22, 19, 26, 22, 18, 23, 20, 25}
        };
        return S[c2][c1]; // row = x2^cat, col = x1^cat
    };

    const double s = s_value(x_cat[0], x_cat[1]);

    // Objective
    const double poly = 3.3333 * s * s + 14.9334 * s - 43.0934;

    const double f =
        0.7854 * x1 * (x2 * x2) * poly
        - 1.508 * x1 * (x5 * x5 + x6 * x6)
        + 7.477 * (std::pow(x5, 3.0) + std::pow(x6, 3.0))
        + 0.7854 * (x3 * (x5 * x5) + x4 * (x6 * x6));

    // Constraints g_i(x) <= 0
    const double g1  = 27.0   - x1 * (x2 * x2) * s;
    const double g2  = 397.5  - x1 * (x2 * x2) * (s * s);
    const double g3  = 1.93 * std::pow(x3, 3.0) - x2 * s * std::pow(x5, 4.0);
    const double g4  = 1.93 * std::pow(x4, 3.0) - x2 * s * std::pow(x6, 4.0);

    const double g5  =
        std::sqrt(std::pow(745.0 * x3, 2.0) + (16.9e6) * (x2 * x2) * (s * s))
        - 110.0 * x2 * s * std::pow(x5, 3.0);

    const double g6  =
        std::sqrt(std::pow(745.0 * x4, 2.0) + (157.5e6) * (x2 * x2) * (s * s))
        - 85.0 * x2 * s * std::pow(x6, 3.0);

    const double g7  = x2 * s - 40.0;
    const double g8  = 5.0 * x2 - x1;
    const double g9  = x1 - 12.0 * x2;
    const double g10 = 1.9 + 1.5 * x5 - x3;
    const double g11 = 1.9 + 1.1 * x6 - x4;

    // Set BBO output: "f g1 g2 ... g11"
    std::string bbo = NOMAD::Double(f).tostring()
        + " " + NOMAD::Double(g1).tostring()
        + " " + NOMAD::Double(g2).tostring()
        + " " + NOMAD::Double(g3).tostring()
        + " " + NOMAD::Double(g4).tostring()
        + " " + NOMAD::Double(g5).tostring()
        + " " + NOMAD::Double(g6).tostring()
        + " " + NOMAD::Double(g7).tostring()
        + " " + NOMAD::Double(g8).tostring()
        + " " + NOMAD::Double(g9).tostring()
        + " " + NOMAD::Double(g10).tostring()
        + " " + NOMAD::Double(g11).tostring();

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
    auto lb = NOMAD::ArrayOfDouble(N, 0.0);
    auto ub = NOMAD::ArrayOfDouble(N, 0.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 7; 
    ub[1] = 7;
    // Continuous lower bounds
    lb[Ncat+0] = 2.6; 
    lb[Ncat+1] = 0.7;
    lb[Ncat+2] = 7.3; 
    lb[Ncat+3] = 0.5;
    lb[Ncat+4] = 2.6; 
    lb[Ncat+5] = 5.0;
    // Continuous upper bounds
    ub[Ncat+0] = 3.6; 
    ub[Ncat+1] = 0.8;
    ub[Ncat+2] = 20.0; 
    ub[Ncat+3] = 8.3;
    ub[Ncat+4] = 3.9; 
    ub[Ncat+5] = 5.5;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // categorical variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {2,3,4,5,6,7}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("speedreducer_constrained.txt bbe sol obj cons_h"));

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
