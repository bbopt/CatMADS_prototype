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
const int Ncat=4;
const int Nint=4;
const int Ncon=6;
const int N=Ncat+Nint+Ncon;
const int Lcat=81;
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



bool My_Evaluator::eval_x(NOMAD::EvalPoint &x,
                          const NOMAD::Double &hMax,
                          bool &countEval) const
{
    (void)hMax;

    if (x.size() != N)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Dimension mismatch.");
    }

    // --- categorical (0..2) ---
    const int c1 = static_cast<int>(x[0].todouble());
    const int c2 = static_cast<int>(x[1].todouble());
    const int c3 = static_cast<int>(x[2].todouble());
    const int c4 = static_cast<int>(x[3].todouble());

    // --- integer (0..10) ---
    const int i1 = static_cast<int>(x[4].todouble());
    const int i2 = static_cast<int>(x[5].todouble());
    const int i3 = static_cast<int>(x[6].todouble());
    const int i4 = static_cast<int>(x[7].todouble());

    // --- continuous (0..10) ---
    const double x1c = x[8].todouble();
    const double x2c = x[9].todouble();
    const double x3c = x[10].todouble();
    const double x4c = x[11].todouble();
    const double x5c = x[12].todouble();
    const double x6c = x[13].todouble();

    auto s1_fun = [&](int cat)->double {
        switch(cat){
        case 0: // A
            return 0.35 + 0.10 * std::log(1.0 + std::pow(x1c - 3.0,2.0) + std::pow(x2c - 7.0,2.0));
        case 1: // B
            return 0.30 + 0.06 * std::abs(x1c - 5.0) + 0.04 * std::abs(x2c - 2.0);
        case 2: // C
        default:
            return 0.55 + 0.80 / (1.0 + std::pow(x1c - 8.0,2.0) + std::pow(x2c - 1.0,2.0));
        }
    };

    auto s2_fun = [&](int cat)->double {
        switch(cat){
        case 0: // A
            return 0.30 + 0.06 * std::abs(x1c - 2.0) + 0.05 * std::abs(x2c - 6.0);
        case 1: // B
            return 0.55 + 0.85 / (1.0 + std::pow(x1c - 1.0,2.0) + std::pow(x2c - 9.0,2.0));
        case 2: // C
        default:
            return 0.33 + 0.10 * std::log(1.0 + std::pow(x1c - 6.0,2.0) + std::pow(x2c - 4.0,2.0));
        }
    };

    auto s3_fun = [&](int cat)->double {
        switch(cat){
        case 0: // A
            return 0.55 + 0.75 / (1.0 + std::pow(x1c - 4.0,2.0) + std::pow(x2c - 4.0,2.0));
        case 1: // B
            return 0.34 + 0.10 * std::log(1.0 + std::pow(x1c - 9.0,2.0) + std::pow(x2c - 2.0,2.0));
        case 2: // C
        default:
            return 0.28 + 0.07 * std::abs(x1c - 7.0) + 0.03 * std::abs(x2c - 5.0);
        }
    };

    auto s4_fun = [&](int cat)->double {
        switch(cat){
        case 0: // A
            return 0.33 + 0.10 * std::log(1.0 + std::pow(x1c - 2.0,2.0) + std::pow(x2c - 1.0,2.0));
        case 1: // B
            return 0.55 + 0.90 / (1.0 + std::pow(x1c - 7.0,2.0) + std::pow(x2c - 8.0,2.0));
        case 2: // C
        default:
            return 0.29 + 0.06 * std::abs(x1c - 4.0) + 0.05 * std::abs(x2c - 7.0);
        }
    };

    const double s1 = s1_fun(c1);
    const double s2 = s2_fun(c2);
    const double s3 = s3_fun(c3);
    const double s4 = s4_fun(c4);

    const int    ints[4] = {i1,i2,i3,i4};
    const double svals[4] = {s1,s2,s3,s4};

    // (x_i^con - x_i^int) uses x1..x4 continuous with i1..i4
    const double con_for_pen[4] = {x1c,x2c,x3c,x4c};

    // ----- objective (complexified) -----
    double sum_inv = 0.0;
    for (int k = 0; k < 4; ++k)
    {
        const double ik = static_cast<double>(ints[k]);
        const double denom =
            svals[k]
            + std::pow(x1c - ik, 2.0)
            + std::pow(x2c - ik, 2.0)
            + 0.08 * std::pow(x5c - 0.7 * ik, 2.0)
            + 0.06 * std::pow(x6c - 0.4 * ik, 2.0)
            + 0.05 * std::abs(x1c - x2c + 0.4 * x5c - 0.3 * x6c);

        sum_inv += 1.0 / denom;
    }

    double pen_abs = 0.0;
    double pen_sqrt = 0.0;
    for (int k = 0; k < 4; ++k)
    {
        const double dk = con_for_pen[k] - static_cast<double>(ints[k]);
        pen_abs  += std::abs(dk);
        pen_sqrt += std::sqrt(std::abs(dk));
    }

    double osc = 0.0;
    for (int k = 0; k < 4; ++k)
    {
        osc += (1.0 + 0.3 * svals[k]) *
               std::cos((M_PI/5.0) * (x5c + x6c) + 0.4 * static_cast<double>(ints[k]));
    }

    const double f =
        sum_inv
        + 0.12 * pen_abs
        + 0.04 * pen_sqrt
        + 0.03 * std::abs(x5c - x6c)
        + 0.02 * std::abs(x1c * x2c - x5c * x6c)
        + 0.06 * osc;

    x.setBBO(NOMAD::Double(f).tostring());
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
    auto ub = NOMAD::ArrayOfDouble(N, 10.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    lb[2] = 0;
    lb[3] = 0;
    // Categorical upper bounds
    ub[0] = 2; 
    ub[1] = 2;
    ub[2] = 2;
    ub[3] = 2;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    // integer variables 
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    // continuous variables 
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group: TODO
    NOMAD::VariableGroup vg0 = {0,1,2,3}; // categorical variables
    NOMAD::VariableGroup vg1 = {4,5,6,7, 8,9,10,11,12,13}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("shekel.txt bbe sol obj cons_h"));

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
