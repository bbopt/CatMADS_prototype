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
const int Ncon=7;
const int N=Ncat+Nint+Ncon;
const int Lcat=100;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ,
                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB};
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

    // Extract categorical variables (A..J encoded as 0..9)
    std::vector<int> x_cat(Ncat);
    for (int i = 0; i < Ncat; ++i)
        x_cat[i] = static_cast<int>(x[i].todouble());

    // Extract integer variables
    std::vector<int> x_int(Nint);
    for (int i = 0; i < Nint; ++i)
        x_int[i] = static_cast<int>(x[Ncat + i].todouble());

    // Extract continuous variables
    std::vector<double> x_con(Ncon);
    for (int i = 0; i < Ncon; ++i)
        x_con[i] = x[Ncat + Nint + i].todouble();

    // Unpack variables (corrected LaTeX ordering)
    const double x1 = x_con[0]; // x_1^continuous
    const double x2 = x_con[1]; // x_2^continuous
    const double x3 = x_con[2]; // x_3^continuous
    const double x4 = x_con[3]; // x_4^continuous
    const double x5 = x_con[4]; // x_5^continuous
    const double x6 = x_con[5]; // x_6^continuous
    const double x7 = x_con[6]; // x_7^continuous

    const int xi1 = x_int[0];   // x_1^integer
    const int xi2 = x_int[1];   // x_2^integer

    // s = s(x1^cat, x2^cat) via table (rows: x2^cat, cols: x1^cat)
    auto s_value = [&](int c1, int c2) -> double {
        static const double S[10][10] = {
            /* x2=A */ {0.45, 0.65, 0.78, 0.87, 0.93, 0.92, 0.89, 0.81, 0.63, 0.44},
            /* x2=B */ {0.46, 0.66, 0.81, 0.90, 0.96, 0.97, 0.89, 0.80, 0.63, 0.43},
            /* x2=C */ {0.46, 0.69, 0.83, 0.96, 1.02, 1.01, 0.92, 0.85, 0.69, 0.49},
            /* x2=D */ {0.52, 0.75, 0.90, 1.02, 1.10, 1.07, 1.03, 0.93, 0.75, 0.54},
            /* x2=E */ {0.63, 0.84, 0.99, 1.12, 1.19, 1.15, 1.13, 1.01, 0.83, 0.66},
            /* x2=F */ {0.75, 0.93, 1.12, 1.23, 1.28, 1.29, 1.23, 1.12, 0.97, 0.72},
            /* x2=G */ {0.87, 1.07, 1.25, 1.34, 1.40, 1.41, 1.37, 1.26, 1.06, 0.86},
            /* x2=H */ {1.04, 1.21, 1.39, 1.50, 1.57, 1.56, 1.49, 1.39, 1.23, 1.05},
            /* x2=I */ {1.20, 1.41, 1.57, 1.67, 1.72, 1.70, 1.67, 1.56, 1.43, 1.22},
            /* x2=J */ {1.42, 1.62, 1.76, 1.87, 1.91, 1.92, 1.84, 1.77, 1.59, 1.39}
        };
        return S[c2][c1]; // row = x2^cat, col = x1^cat
    };

    const double s = s_value(x_cat[0], x_cat[1]);

    // Objective
    const double f =
        1.98
        + 4.90 * x1
        + 6.67 * x2
        + 6.98 * x3
        + 4.01 * x4
        + 1.78 * x5
        + 2.73 * static_cast<double>(xi2)
        + 0.60 * s
        + 0.20 * std::abs(s - 1.20)
        + 0.12 * std::abs(x4 - 1.00)
        + 0.06 * std::abs(static_cast<double>(xi1) + 2.0)
        + 0.25 * std::pow(x1 - 1.0, 2.0) * std::abs(x2 - 0.9);

    // Constraints g_i(x) <= 0
    const double g1 =
        1.16
        - 0.3717 * x2 * x4
        - 0.00931 * x2 * x6
        - 0.484 * x3 * s
        + 0.01343 * static_cast<double>(xi1) * x6
        - 1.0;

    const double g2 =
        0.261
        - 0.0159 * x1 * x2
        - 0.188 * x1 * s
        - 0.019 * x2 * static_cast<double>(xi2)
        + 0.0144 * x3 * x5
        + 0.0008757 * x5 * x6
        + 0.08045 * static_cast<double>(xi1) * s
        + 0.00139 * s * x7
        + 0.000001575 * x6 * x7
        - 0.32;

    const double g3 =
        0.214
        + 0.00817 * x5
        - 0.131 * x1 * s
        - 0.0704 * x1 * s
        + 0.03099 * x2 * static_cast<double>(xi1)
        - 0.018 * x2 * static_cast<double>(xi2)
        + 0.0208 * x3 * s
        + 0.121 * x3 * s
        - 0.00364 * x5 * static_cast<double>(xi1)
        + 0.0007715 * x5 * x6
        - 0.0005354 * static_cast<double>(xi1) * x6
        + 0.00121 * s * x7
        - 0.32;

    const double g4 =
        0.74
        - 0.061 * x2
        - 0.163 * x3 * s
        + 0.001232 * x3 * x6
        - 0.166 * static_cast<double>(xi2) * s
        + 0.227 * std::pow(static_cast<double>(xi2), 2.0)
        - 0.32;

    const double g5 =
        28.98
        + 3.818 * x3
        - 4.2 * x1 * x2
        + 0.0207 * x5 * x6
        + 6.63 * static_cast<double>(xi1) * s
        - 7.7 * static_cast<double>(xi2) * s
        + 0.32 * s * x6
        - 32.0;

    const double g6 =
        33.86
        + 2.95 * x3
        + 0.1792 * x6
        - 5.057 * x1 * x2
        - 11.02 * x2 * s
        - 0.0215 * x5 * x6
        - 9.98 * static_cast<double>(xi2) * s
        + 22.0 * (s * s)
        - 32.0;

    const double g7 =
        46.36
        - 9.9 * x2
        - 12.9 * x1 * s
        + 0.1107 * x3 * x6
        - 32.0;

    const double g8 =
        4.72
        - 0.5 * x4
        - 0.19 * x2 * x3
        - 0.0122 * x4 * x6
        + 0.009325 * static_cast<double>(xi1) * x6
        + 0.000191 * std::pow(x7, 2.0)
        - 4.0;

    const double g9 =
        10.58
        - 0.674 * x1 * x2
        - 1.95 * x2 * s
        + 0.02054 * x3 * x6
        - 0.0198 * x4 * x6
        + 0.028 * static_cast<double>(xi1) * x6
        - 9.9;

    const double g10 =
        16.45
        - 0.489 * x3 * static_cast<double>(xi2)
        - 0.843 * x5 * static_cast<double>(xi1)
        + 0.432 * s * x6
        - 0.0556 * s * x7
        - 0.000786 * std::pow(x7, 2.0)
        - 15.7;

    // Set BBO output: "f g1 g2 ... g10"
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
        + " " + NOMAD::Double(g10).tostring();

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

    // Bounds for all variables
    auto lb = NOMAD::ArrayOfDouble(N, 0.5);
    auto ub = NOMAD::ArrayOfDouble(N, 1.5);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 9; 
    ub[1] = 9;
    // Integer lower bounds
    lb[Ncat+0] = -10; 
    lb[Ncat+1] =  25;
    // Integer upper bounds
    ub[Ncat+0] = -10; 
    ub[Ncat+1] =  25;
    // Continuous lower bounds: specific variables
    lb[Ncat+Nint+1] =  0.45; //2
    lb[Ncat+Nint+4] =  0.875; //5
    // Continuous upper bounds: specific variables
    ub[Ncat+Nint+1] =  1.35; //2
    ub[Ncat+Nint+4] =  2.625; //5
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group: TODO
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {1,2, 3,4,5,6,7,8,9}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("carsideimpact_constrained.txt bbe sol obj cons_h"));

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
