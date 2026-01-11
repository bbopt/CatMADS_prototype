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
const int Ncat=6;
const int Nint=2;
const int Ncon=2;
const int N=Ncat+Nint+Ncon;
const int Lcat=64;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ,
                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
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

    // Extract categorical variables (A,B encoded as 0,1)
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

    // Unpack variables
    const double xc1 = x_con[0]; // x_1^continuous
    const double xc2 = x_con[1]; // x_2^continuous
    const int xi1 = x_int[0];    // x_1^integer
    const int xi2 = x_int[1];    // x_2^integer

    // Coefficients (i = 1..6)
    static const double a[6] = {2.00, 3.10, 9.80, 5.00, 3.20, 1.10};
    static const double b[6] = {1.10, 0.95, 0.85, 0.60, 0.90, 0.55};
    static const double c[6] = {1.40, 2.60, 9.10, 4.60, 2.05, 0.90};
    static const double d[6] = {1.80, 1.55, 1.25, 1.05, 1.08, 0.85};

    // Helper: compute s_i
    auto compute_s = [&](int i, int cat) -> double {
        // cat: 0 -> A, 1 -> B
        if (cat == 0)
        {
            return a[i] + b[i] * std::log(1.0 + xc1 * xc1 + xc2 * xc2);
        }
        else
        {
            return c[i] + d[i] * std::exp(0.02 * (xc1 - xc2));
        }
    };

    // Compute s1..s6
    const double s1 = compute_s(0, x_cat[0]);
    const double s2 = compute_s(1, x_cat[1]);
    const double s3 = compute_s(2, x_cat[2]);
    const double s4 = compute_s(3, x_cat[3]);
    const double s5 = compute_s(4, x_cat[4]);
    const double s6 = compute_s(5, x_cat[5]);

    // Objective
    const double f =
        s1 * s1 + s2 * s2 + s1 * s2 - 14.0 * s1 - 16.0 * s2
        + std::pow(s3 - 10.0, 2.0)
        + 4.0 * std::pow(s4 - 5.0, 2.0)
        + std::pow(s5 - 3.0, 2.0)
        + 2.0 * std::pow(s6 - 1.0, 2.0)
        + 5.0 * (xc1 * xc1)
        + 7.0 * std::pow(xc2 - 11.0, 2.0)
        + 2.0 * std::pow(static_cast<double>(xi1) - 10.0, 2.0)
        + std::pow(static_cast<double>(xi2) - 7.0, 2.0)
        + 45.0;

    // Constraints g_i(x) <= 0
    const double g1 = -105.0 + 4.0 * s1 + 5.0 * s2 - 3.0 * xc1 + 9.0 * xc2;
    const double g2 =  10.0 * s1 - 8.0 * s2 - 17.0 * xc1 + 2.0 * xc2;
    const double g3 =  -8.0 * s1 + 2.0 * s2 + 5.0 * xc1 - 2.0 * (xc2 * xc2) - 12.0;
    const double g4 =  3.0 * std::pow(s1 - 2.0, 2.0) + 4.0 * std::pow(s2 - 3.0, 2.0) + 2.0 * (s3 * s3) - 7.0 * s4 - 120.0;
    const double g5 =  5.0 * (s1 * s1) + 8.0 * s2 + std::pow(s3 - 6.0, 2.0) - 2.0 * s4 - 40.0;
    const double g6 =  (s1 * s1) + 2.0 * std::pow(s2 - 2.0, 2.0) - 2.0 * s1 * s2 + 14.0 * s5 - 6.0 * s6;
    const double g7 =  0.5 * std::pow(s1 - 8.0, 2.0) + 2.0 * std::pow(s2 - 4.0, 2.0) + 3.0 * (s5 * s5) - s6 - 30.0;
    const double g8 = -3.0 * s1 + 6.0 * s2 + 12.0 * std::pow(xc1 - 8.0, 2.0) - 7.0 * (xc2 * xc2);

    // Set BBO output: "f g1 g2 ... g8"
    std::string bbo = NOMAD::Double(f).tostring()
        + " " + NOMAD::Double(g1).tostring()
        + " " + NOMAD::Double(g2).tostring()
        + " " + NOMAD::Double(g3).tostring()
        + " " + NOMAD::Double(g4).tostring()
        + " " + NOMAD::Double(g5).tostring()
        + " " + NOMAD::Double(g6).tostring()
        + " " + NOMAD::Double(g7).tostring()
        + " " + NOMAD::Double(g8).tostring();

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
    auto lb = NOMAD::ArrayOfDouble(N, -10.0);
    auto ub = NOMAD::ArrayOfDouble(N,  10.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    lb[2] = 0; 
    lb[3] = 0;
    lb[4] = 0; 
    lb[5] = 0;
    // Categorical upper bounds
    ub[0] = 1; 
    ub[1] = 1;
    ub[2] = 1; 
    ub[3] = 1;
    ub[4] = 1; 
    ub[5] = 1;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    // integer variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    // continuous variables  
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group: TODO
    NOMAD::VariableGroup vg0 = {0,1,2,3,4,5}; // categorical variables
    NOMAD::VariableGroup vg1 = {6,7, 8,9}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("G07_constrained.txt bbe sol obj cons_h"));

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
