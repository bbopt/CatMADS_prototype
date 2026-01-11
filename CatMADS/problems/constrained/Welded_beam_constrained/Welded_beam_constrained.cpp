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
const int Lcat=100;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ,
                NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
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

    // Categorical (A..J encoded 0..9)
    std::vector<int> x_cat(Ncat);
    for (int i = 0; i < Ncat; ++i)
        x_cat[i] = static_cast<int>(x[i].todouble());

    // Integer (x1^int, x2^int)
    std::vector<int> x_int(Nint);
    for (int i = 0; i < Nint; ++i)
        x_int[i] = static_cast<int>(x[Ncat + i].todouble());

    // Continuous (x1..x4)
    std::vector<double> x_con(Ncon);
    for (int i = 0; i < Ncon; ++i)
        x_con[i] = x[Ncat + Nint + i].todouble();

    const int c1 = x_cat[0]; // x1^cat
    const int c2 = x_cat[1]; // x2^cat

    const int i1 = x_int[0]; // x1^int
    const int i2 = x_int[1]; // x2^int

    const double xc1 = x_con[0]; // x1^con
    const double xc2 = x_con[1]; // x2^con
    const double xc3 = x_con[2]; // x3^con
    const double xc4 = x_con[3]; // x4^con

    // s(x1^cat, x2^cat): table is "transposed version" with rows = x2^cat, cols = x1^cat
    static const double s_tab[10][10] = {
        {0.40,0.44,0.50,0.60,0.74,0.86,0.92,0.84,0.70,0.54}, // row A
        {0.62,0.70,0.80,0.94,1.12,1.28,1.36,1.22,1.02,0.78}, // row B
        {0.82,0.94,1.08,1.26,1.50,1.70,1.78,1.58,1.30,0.98}, // row C
        {0.98,1.12,1.30,1.54,1.84,2.05,2.08,1.84,1.50,1.12}, // row D
        {1.08,1.26,1.48,1.76,2.12,2.28,2.20,1.94,1.58,1.18}, // row E
        {1.02,1.18,1.38,1.62,1.96,2.14,2.16,1.90,1.54,1.14}, // row F
        {0.90,1.02,1.18,1.38,1.66,1.82,1.90,1.74,1.42,1.06}, // row G
        {0.74,0.84,0.96,1.12,1.34,1.46,1.56,1.50,1.24,0.94}, // row H
        {0.56,0.62,0.70,0.82,0.98,1.06,1.14,1.12,1.00,0.78}, // row I
        {0.36,0.40,0.46,0.54,0.66,0.72,0.78,0.80,0.72,0.56}  // row J
    };
    const double s = s_tab[c2][c1];

    // Constants
    const double L = 14.0;

    // Helper definitions
    const double h  = 0.18 + 0.22 * xc1 + 0.006 * (static_cast<double>(i1) + 6.0);
    const double ell= 2.5  + 4.8  * xc2 + 0.08  * (static_cast<double>(i2) + 6.0);
    const double t  = 5.0  + 4.5  * xc3 + 0.10  * static_cast<double>(i1);
    const double b  = 0.20 + 0.30 * xc4;
    const double P  = 2600.0 + 1600.0 * xc2 + 60.0 * static_cast<double>(i2);

    // E(s), G(s)
    const double E = 30.0e6 * (0.55 + 0.35 * s);
    const double G = 12.0e6 * (0.60 + 0.30 * s);

    // Stress / deflection auxiliaries
    const double tau_p = P / (std::sqrt(2.0) * h * ell);
    const double M     = P * (L + ell / 2.0);
    const double R     = std::sqrt((ell * ell) / 4.0 + std::pow((h + t) / 2.0, 2.0));
    const double J     = 2.0 * (std::sqrt(2.0) * h * ell) * ( (ell * ell) / 12.0 + std::pow(h + t, 2.0) / 4.0 );
    const double tau_pp= (M * R) / J;

    const double tau = std::sqrt(tau_p * tau_p + 2.0 * tau_p * tau_pp * (ell / (2.0 * R)) + tau_pp * tau_pp);

    const double sigma = (6.0 * P * L) / (b * t * t);
    const double delta = (4.0 * P * std::pow(L, 3.0)) / (E * b * std::pow(t, 3.0));

    const double Pc =
        (4.013 * E * std::sqrt((t * t * std::pow(b, 6.0)) / 36.0) / (L * L))
        * (1.0 - (t / (2.0 * L)) * std::sqrt(E / (4.0 * G)));

    // Objective
    const double base_cost = 1.10471 * h * h * ell + 0.04811 * t * b * (L + ell);
    const double f =
        (1.35 - 0.25 * s) * base_cost
        + 10.0 * s * std::abs((static_cast<double>(i1) - static_cast<double>(i2)) / 24.0)
        + 0.08 * std::abs(s - 1.35);

    // Constraints g_i(x) <= 0
    const double g1 = tau   - 13600.0 * s;
    const double g2 = sigma - 30000.0 * s;
    const double g3 = h - b;
    const double g4 = delta - 0.25;
    const double g5 = P - Pc;

    // Set BBO output: "f g1 g2 g3 g4 g5"
    std::string bbo = NOMAD::Double(f).tostring()
        + " " + NOMAD::Double(g1).tostring()
        + " " + NOMAD::Double(g2).tostring()
        + " " + NOMAD::Double(g3).tostring()
        + " " + NOMAD::Double(g4).tostring()
        + " " + NOMAD::Double(g5).tostring();

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
    auto lb = NOMAD::ArrayOfDouble(N, 0.0);
    auto ub = NOMAD::ArrayOfDouble(N, 0.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = 0;
    // Categorical upper bounds
    ub[0] = 9; 
    ub[1] = 9;
    // Integer lower bounds
    lb[Ncat+0] = -6; 
    lb[Ncat+1] = -6;
    // Integer upper bounds
    ub[Ncat+0] = 18; 
    ub[Ncat+1] = 18;
    // Continuous lower bounds
    lb[Ncat+Nint+0] = 0.7; 
    lb[Ncat+Nint+1] = 0.5;
    lb[Ncat+Nint+2] = 0.6; 
    lb[Ncat+Nint+3] = 0.7;
    // Continuous upper bounds
    ub[Ncat+Nint+0] = 1.5; 
    ub[Ncat+Nint+1] = 1.4;
    ub[Ncat+Nint+2] = 1.6; 
    ub[Ncat+Nint+3] = 1.5;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // categorical variables
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,  // integer variables
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable group: TODO
    NOMAD::VariableGroup vg0 = {0,1}; // categorical variables
    NOMAD::VariableGroup vg1 = {2,3, 4,5,6,7}; // quantitative variables
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
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("welded_beam_constrained.txt bbe sol obj cons_h"));

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
