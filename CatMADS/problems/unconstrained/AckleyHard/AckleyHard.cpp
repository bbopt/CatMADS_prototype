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
const int Ncon=4;
const int N=Ncat+Nint+Ncon;
const int Lcat=9;
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
    // Ensure the input dimension matches the expected size
    if (x.size() != (2 + 2 + Ncon)) // n_cat + n_int + n_con
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Dimension mismatch: Ensure the number of variables matches n_cat + n_int + n_con.");
    }

    // Extract categorical variables
    int x_cat1 = static_cast<int>(x[0].todouble()); // x1^{cat}
    int x_cat2 = static_cast<int>(x[1].todouble()); // x2^{cat}

    // Extract integer variables
    std::vector<int> x_int(2); // Integer variables
    for (int i = 0; i < 2; ++i)
    {
        x_int[i] = static_cast<int>(x[2 + i].todouble());
    }

    // Extract continuous variables
    std::vector<double> x_con(Ncon); // Continuous variables
    for (int i = 0; i < Ncon; ++i)
    {
        x_con[i] = x[4 + i].todouble();
    }

    // Compute penalty term p(x_cat2, x_con)
    auto compute_penalty = [&](int cat2, const std::vector<double> &x_con) -> double {
        double sum_con = 0.0;
        for (double xi : x_con)
        {
            sum_con += xi;
        }

        switch (cat2)
        {
        case 0: // "positive"
            return 15 * std::abs(sum_con);
        case 1: // "negative"
            return 15 * std::abs(sum_con - 1);
        case 2: // "neutral"
            return 10 * std::abs(sum_con);
        default:
            return 0.0;
        }
    };

    double penalty = compute_penalty(x_cat2, x_con);

    // Initialize the function value
    double f = penalty;

    // Compute ACK function based on x_cat1
    if (x_cat1 == 0) // Case "A"
    {
        double sum_squares = 0.0, cos_sum = 0.0;

        for (double xi : x_con)
        {
            sum_squares += std::pow(xi + x_int[0], 2);
            cos_sum += std::cos(2 * M_PI * xi * x_int[1]);
        }

        f += -20 * std::exp(-0.2 * std::sqrt(sum_squares / Ncon));
        f += -std::exp(cos_sum / Ncon);
    }
    else if (x_cat1 == 1) // Case "B"
    {
        double sum_abs = 0.0, cos_sum = 0.0;

        for (double xi : x_con)
        {
            sum_abs += std::pow(std::abs(xi + x_int[0]), 2);
            cos_sum += std::cos(2 * M_PI * xi - x_int[1]);
        }

        f += -10 * std::exp(-0.1 * std::sqrt(sum_abs / Ncon));
        f += 5 * cos_sum;
    }
    else if (x_cat1 == 2) // Case "C"
    {
        double product_abs = 0.0, cos_squared_sum = 0.0;

        for (double xi : x_con)
        {
            product_abs += std::abs(xi * x_int[0]);
            cos_squared_sum += std::cos(2 * M_PI * xi + x_int[1]);
        }

        f += -15 * std::exp(-std::sqrt(product_abs / Ncon));
        f += -5 * std::exp(-cos_squared_sum / Ncon);
    }

    // Shift the function value for positivity
    f += 50;

    // Set the computed value into NOMAD's framework
    NOMAD::Double F(f);
    x.setBBO(F.tostring());
    countEval = true;

    return true; // The evaluation succeeded
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
    auto lb = NOMAD::ArrayOfDouble(N, -5.0);
    auto ub = NOMAD::ArrayOfDouble(N, 5.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 2; 
    ub[1] = 2;
    // Integer lower bounds
    lb[Ncat+0] = 1; 
    lb[Ncat+1] = -1;
    // Integer upper bounds
    ub[Ncat+0] = 10;
    ub[Ncat+1] = 1;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER, // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);


    // Variable group
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {2,3, 4,5,6,7}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("ackleyhard.txt bbe sol obj cons_h"));

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
