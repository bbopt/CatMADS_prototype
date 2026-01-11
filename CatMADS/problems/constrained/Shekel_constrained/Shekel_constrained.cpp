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
    // Cat-cstrs-29:
    //   n^cat = 4 : x1^cat..x4^cat in {A,B,C} (encoded 0..2)
    //   n^int = 4 : x1^int..x4^int in {0..10}
    //   n^con = 6 : x1^con..x6^con in [0,10]

    // ----------------------------
    // Decode variables
    // ----------------------------
    const int c1 = static_cast<int>(x[0].todouble());
    const int c2 = static_cast<int>(x[1].todouble());
    const int c3 = static_cast<int>(x[2].todouble());
    const int c4 = static_cast<int>(x[3].todouble());

    const int i1 = static_cast<int>(x[Ncat + 0].todouble());
    const int i2 = static_cast<int>(x[Ncat + 1].todouble());
    const int i3 = static_cast<int>(x[Ncat + 2].todouble());
    const int i4 = static_cast<int>(x[Ncat + 3].todouble());

    const double x1 = x[Ncat + Nint + 0].todouble(); // x1^con
    const double x2 = x[Ncat + Nint + 1].todouble(); // x2^con
    const double x3 = x[Ncat + Nint + 2].todouble(); // x3^con
    const double x4 = x[Ncat + Nint + 3].todouble(); // x4^con
    const double x5 = x[Ncat + Nint + 4].todouble(); // x5^con
    const double x6 = x[Ncat + Nint + 5].todouble(); // x6^con

    // ----------------------------
    // Helper: s_i( cat_i, x1, x2 )
    // cat code: 0=A, 1=B, 2=C
    // ----------------------------
    auto s1 = [&](int cat, double u1, double u2) -> double {
        switch (cat) {
            case 0: // A
                return 0.35 + 0.10 * std::log(1.0 + std::pow(u1 - 3.0, 2.0) + std::pow(u2 - 7.0, 2.0));
            case 1: // B
                return 0.30 + 0.06 * std::abs(u1 - 5.0) + 0.04 * std::abs(u2 - 2.0);
            default: // C
                return 0.55 + 0.80 / (1.0 + std::pow(u1 - 8.0, 2.0) + std::pow(u2 - 1.0, 2.0));
        }
    };

    auto s2 = [&](int cat, double u1, double u2) -> double {
        switch (cat) {
            case 0: // A
                return 0.30 + 0.06 * std::abs(u1 - 2.0) + 0.05 * std::abs(u2 - 6.0);
            case 1: // B
                return 0.55 + 0.85 / (1.0 + std::pow(u1 - 1.0, 2.0) + std::pow(u2 - 9.0, 2.0));
            default: // C
                return 0.33 + 0.10 * std::log(1.0 + std::pow(u1 - 6.0, 2.0) + std::pow(u2 - 4.0, 2.0));
        }
    };

    auto s3 = [&](int cat, double u1, double u2) -> double {
        switch (cat) {
            case 0: // A
                return 0.55 + 0.75 / (1.0 + std::pow(u1 - 4.0, 2.0) + std::pow(u2 - 4.0, 2.0));
            case 1: // B
                return 0.34 + 0.10 * std::log(1.0 + std::pow(u1 - 9.0, 2.0) + std::pow(u2 - 2.0, 2.0));
            default: // C
                return 0.28 + 0.07 * std::abs(u1 - 7.0) + 0.03 * std::abs(u2 - 5.0);
        }
    };

    auto s4 = [&](int cat, double u1, double u2) -> double {
        switch (cat) {
            case 0: // A
                return 0.33 + 0.10 * std::log(1.0 + std::pow(u1 - 2.0, 2.0) + std::pow(u2 - 1.0, 2.0));
            case 1: // B
                return 0.55 + 0.90 / (1.0 + std::pow(u1 - 7.0, 2.0) + std::pow(u2 - 8.0, 2.0));
            default: // C
                return 0.29 + 0.06 * std::abs(u1 - 4.0) + 0.05 * std::abs(u2 - 7.0);
        }
    };

    const double S1 = s1(c1, x1, x2);
    const double S2 = s2(c2, x1, x2);
    const double S3 = s3(c3, x1, x2);
    const double S4 = s4(c4, x1, x2);

    // Shared abs term inside each inverse
    const double abs_mix = std::abs(x1 - x2 + 0.4 * x5 - 0.3 * x6);

    // ----------------------------
    // Objective
    // ----------------------------
    auto inv_term = [&](double Si, int ii) -> double {
        const double d12 =
            Si
            + std::pow(x1 - static_cast<double>(ii), 2.0)
            + std::pow(x2 - static_cast<double>(ii), 2.0)
            + 0.08 * std::pow(x5 - 0.7 * static_cast<double>(ii), 2.0)
            + 0.06 * std::pow(x6 - 0.4 * static_cast<double>(ii), 2.0)
            + 0.05 * abs_mix;
        return 1.0 / d12;
    };

    const double sum_inv =
        inv_term(S1, i1) +
        inv_term(S2, i2) +
        inv_term(S3, i3) +
        inv_term(S4, i4);

    // The extra penalties use x_i^continuous for i=1..4 (i.e., x1..x4) vs i1..i4
    const double pen_abs =
        std::abs(x1 - static_cast<double>(i1)) +
        std::abs(x2 - static_cast<double>(i2)) +
        std::abs(x3 - static_cast<double>(i3)) +
        std::abs(x4 - static_cast<double>(i4));

    const double pen_sqrt =
        std::sqrt(std::abs(x1 - static_cast<double>(i1))) +
        std::sqrt(std::abs(x2 - static_cast<double>(i2))) +
        std::sqrt(std::abs(x3 - static_cast<double>(i3))) +
        std::sqrt(std::abs(x4 - static_cast<double>(i4)));

    const double trig =
        (1.0 + 0.3 * S1) * std::cos((M_PI / 5.0) * (x5 + x6) + 0.4 * static_cast<double>(i1)) +
        (1.0 + 0.3 * S2) * std::cos((M_PI / 5.0) * (x5 + x6) + 0.4 * static_cast<double>(i2)) +
        (1.0 + 0.3 * S3) * std::cos((M_PI / 5.0) * (x5 + x6) + 0.4 * static_cast<double>(i3)) +
        (1.0 + 0.3 * S4) * std::cos((M_PI / 5.0) * (x5 + x6) + 0.4 * static_cast<double>(i4));

    const double f =
        sum_inv
        + 0.12 * pen_abs
        + 0.04 * pen_sqrt
        + 0.03 * std::abs(x5 - x6)
        + 0.02 * std::abs(x1 * x2 - x5 * x6)
        + 0.06 * trig;

    // ----------------------------
    // Constraints
    // ----------------------------
    const double g1 =
        (S1 + S2) * (S3 + S4)
        + (static_cast<double>(i1 + i2 + i3 + i4)) / 12.0
        - 2.60;

    const double g2 =
        std::pow(x3 - x4, 2.0)
        + std::pow(x5 - x6, 2.0)
        + 0.20 * std::abs(x1 - static_cast<double>(i1))
        + 0.20 * std::abs(x2 - static_cast<double>(i2))
        - 3.20;

    const double g3 =
        (x1 - 4.0) * (x2 - 6.0)
        + 0.30 * std::abs(static_cast<double>(i1 - i2))
        + 0.20 * std::abs(S2 - S4)
        - 2.50;

    // Set BBO output: "f g1 g2 g3"
    std::string bbo = NOMAD::Double(f).tostring()
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
