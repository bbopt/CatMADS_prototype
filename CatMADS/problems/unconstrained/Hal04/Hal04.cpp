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
const int Ncat = 3;
const int Nint = 0;
const int Ncon = 5;
const int N = Ncat + Nint + Ncon;
const int Lcat = 12;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {
    NOMAD::BBOutputType::OBJ
};
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
    My_Evaluator(
        const std::shared_ptr<NOMAD::EvalParameters>& evalParams
    )
        : NOMAD::Evaluator(evalParams, NOMAD::EvalType::BB)
    {
    }

    ~My_Evaluator() {}

    bool eval_x(
        NOMAD::EvalPoint& x,
        const NOMAD::Double& hMax,
        bool& countEval
    ) const override;
};


/*----------------------------------------*/
/*           user-defined eval_x          */
/*----------------------------------------*/
bool My_Evaluator::eval_x(
    NOMAD::EvalPoint& x,
    const NOMAD::Double& hMax,
    bool& countEval
) const
{
    (void)hMax; // unused (no constraints handled here)

    // Expect: Ncat = 3, Nint = 0, Ncon = 5
    if (x.size() != (Ncat + Nint + Ncon))
    {
        throw NOMAD::Exception(
            __FILE__,
            __LINE__,
            "Dimension mismatch: expected Ncat + Nint + Ncon variables."
        );
    }

    // ---- Extract categorical variables (integer encoding) ----
    // x_cat1: A,B,C -> 0,1,2
    // x_cat2: D,E   -> 0,1
    // x_cat3: F,G   -> 0,1
    const int x_cat1 = static_cast<int>(x[0].todouble());
    const int x_cat2 = static_cast<int>(x[1].todouble());
    const int x_cat3 = static_cast<int>(x[2].todouble());

    // ---- Extract continuous variables x1..x5 ----
    std::vector<double> xc(Ncon);

    for (int k = 0; k < Ncon; ++k)
    {
        xc[k] = x[Ncat + Nint + k].todouble(); // starts at index 3
    }

    // ---- s(x^cat) ----
    //
    // First index  : x_cat1 = A,B,C
    // Second index : x_cat2 = D,E
    // Third index  : x_cat3 = F,G
    //
    // (A,D,F) ->  0.00
    // (A,D,G) ->  0.20
    // (A,E,F) -> -0.10
    // (A,E,G) ->  0.25
    // (B,D,F) ->  0.50
    // (B,D,G) -> -0.20
    // (B,E,F) -> -0.50
    // (B,E,G) ->  0.80
    // (C,D,F) ->  0.90
    // (C,D,G) -> -0.50
    // (C,E,F) ->  1.00
    // (C,E,G) ->  1.25
    static const double S_TABLE[3][2][2] = {
        {
            { 0.00,  0.20 },
            {-0.10,  0.25 }
        },
        {
            { 0.50, -0.20 },
            {-0.50,  0.80 }
        },
        {
            { 0.90, -0.50 },
            { 1.00,  1.25 }
        }
    };

    const double s = S_TABLE[x_cat1][x_cat2][x_cat3];

    // ---- Objective ----
    const double pi = M_PI;

    double f = 0.0;

    for (int i = 1; i <= 5; ++i)
    {
        const double xi = xc[i - 1];

        const double weight =
            std::pow(2.0, static_cast<double>(i - 1) / 4.0);

        const double term =
            5.0 * xi
            + 0.3 * s * std::pow(xi, 4)
            + std::pow(1.0 - xi, 2)
            + s * std::sin(
                3.0 * pi * xi
                + (static_cast<double>(i) * pi) / 5.0
            );

        f += term * weight;
    }

    // ---- Return to NOMAD ----
    NOMAD::Double F(f);
    x.setBBO(F.tostring());
    countEval = true;

    return true;
}


/*----------------------------------------*/
/*             Parameters                 */
/*----------------------------------------*/
void initAllParams(
    std::shared_ptr<NOMAD::AllParameters> allParams,
    std::map<
        NOMAD::DirectionType,
        NOMAD::ListOfVariableGroup
    >& myMapDirTypeToVG,
    NOMAD::ListOfVariableGroup& myListFixVGForQMS
)
{
    // Parameters creation
    allParams->setAttributeValue("DIMENSION", N);

    // Black-box evaluations
    allParams->setAttributeValue("MAX_BB_EVAL", nbEvals);

    // Starting point
    // allParams->setAttributeValue("X0", NOMAD::Point(N, 0.0));

    // LHS
    std::string budgetLHsFormat =
        std::to_string(nbEvalsLHS) + " 0";

    allParams->setAttributeValue(
        "LH_SEARCH",
        NOMAD::LHSearchType(budgetLHsFormat.c_str())
    );

    // Bounds for all variables
    auto lb = NOMAD::ArrayOfDouble(N, -6.0);
    auto ub = NOMAD::ArrayOfDouble(N, 9.0);

    // Categorical lower bounds
    lb[0] = 0;
    lb[1] = 0;
    lb[2] = 0;

    // Categorical upper bounds
    ub[0] = 2;
    ub[1] = 1;
    ub[2] = 1;

    // Continuous lower bounds
    lb[Ncat + 0] = -9;
    lb[Ncat + 1] = -7;
    lb[Ncat + 2] = -10;
    lb[Ncat + 3] = -8;
    lb[Ncat + 4] = -6;

    // Continuous upper bounds
    ub[Ncat + 0] = 12;
    ub[Ncat + 1] = 14;
    ub[Ncat + 2] = 10;
    ub[Ncat + 3] = 13;
    ub[Ncat + 4] = 15;

    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);

    // Types
    NOMAD::BBInputTypeList bbinput = {
        NOMAD::BBInputType::INTEGER,
        NOMAD::BBInputType::INTEGER,
        NOMAD::BBInputType::INTEGER,       // categorical variables

        NOMAD::BBInputType::CONTINUOUS,
        NOMAD::BBInputType::CONTINUOUS,
        NOMAD::BBInputType::CONTINUOUS,
        NOMAD::BBInputType::CONTINUOUS,
        NOMAD::BBInputType::CONTINUOUS
    };

    allParams->setAttributeValue(
        "BB_INPUT_TYPE",
        bbinput
    );

    // Variable groups
    NOMAD::VariableGroup vg0 = {
        0, 1, 2
    }; // categorical variables

    NOMAD::VariableGroup vg1 = {
        3, 4, 5, 6, 7
    }; // quantitative variables

    allParams->setAttributeValue(
        "VARIABLE_GROUP",
        NOMAD::ListOfVariableGroup({vg0, vg1})
    );

    // Poll in two subpolls
    NOMAD::DirectionTypeList dtList = {
        NOMAD::DirectionType::USER_FREE_POLL,
        NOMAD::DirectionType::ORTHO_2N
    };

    allParams->setAttributeValue(
        "DIRECTION_TYPE",
        dtList
    );

    // Associate direction types and variable groups
    myMapDirTypeToVG = {
        {dtList[0], {vg0}},
        {dtList[1], {vg1}}
    };

    // Constraints and objective
    allParams->setAttributeValue(
        "BB_OUTPUT_TYPE",
        bbOutputTypeListSetup
    );

    // Quad model search where categorical variables are fixed
    allParams->setAttributeValue(
        "QUAD_MODEL_SEARCH",
        true
    );

    myListFixVGForQMS = {vg0};

    // Default searches that are deactivated
    allParams->setAttributeValue(
        "NM_SEARCH",
        false
    );

    allParams->setAttributeValue(
        "SPECULATIVE_SEARCH",
        false
    );

    // Enable the user search method
    allParams->setAttributeValue(
        "USER_SEARCH",
        true
    );

    // Display
    allParams->setAttributeValue(
        "DISPLAY_DEGREE",
        2
    );

    allParams->setAttributeValue(
        "DISPLAY_STATS",
        NOMAD::ArrayOfString(
            "bbe ( sol ) obj cons_h"
        )
    );

    allParams->setAttributeValue(
        "DISPLAY_ALL_EVAL",
        true
    );

    // Fix seed for reproducibility of results
    allParams->setAttributeValue(
        "SEED",
        seedSetup
    );

    allParams->setAttributeValue(
        "RNG_ALT_SEEDING",
        true
    );

    // File history for convergence plots and profiles
    allParams->setAttributeValue(
        "STATS_FILE",
        NOMAD::ArrayOfString(
            "hal04.txt bbe sol obj cons_h"
        )
    );

    // Parameters validation
    allParams->checkAndComply();
}


/*------------------------------------------*/
/*            NOMAD main function           */
/*------------------------------------------*/
int main(int argc, char** argv)
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
    auto params =
        std::make_shared<NOMAD::AllParameters>();

    // Map to associate a direction type to a group of variables
    std::map<
        NOMAD::DirectionType,
        NOMAD::ListOfVariableGroup
    > myMapDirTypeToVG;

    // List of fixed variable groups for Quad model search
    NOMAD::ListOfVariableGroup myListFixVGForQMS;

    initAllParams(
        params,
        myMapDirTypeToVG,
        myListFixVGForQMS
    );

    TheMainStep.setAllParameters(params);

    // Custom evaluator
    std::shared_ptr<NOMAD::Evaluator> ev(
        new My_Evaluator(params->getEvalParams())
    );

    TheMainStep.setEvaluator(std::move(ev));

    // Main step start initializes Mads
    TheMainStep.start();

    // Define new sort function and sort according to that function
    auto customOrder =
        std::make_shared<CustomOrder>();

    NOMAD::EvcInterface::getEvaluatorControl()
        ->setUserCompMethod(customOrder);

    // Define post-evaluation callback
    NOMAD::EvalCallbackFunc<
        NOMAD::CallbackType::POST_EVAL_UPDATE
    > cbPostEvalUpdate = customPostEvalUpdateCB;

    NOMAD::EvcInterface::getEvaluatorControl()
        ->addEvalCallback<
            NOMAD::CallbackType::POST_EVAL_UPDATE
        >(cbPostEvalUpdate);

    // Register callback functions
    auto mads =
        std::dynamic_pointer_cast<NOMAD::Mads>(
            TheMainStep.getAlgo(
                NOMAD::StepType::ALGORITHM_MADS
            )
        );

    if (nullptr == mads)
    {
        throw NOMAD::Exception(
            __FILE__,
            __LINE__,
            "Cannot access to Mads algorithm"
        );
    }

    // Callbacks for search
    mads->addCallback(
        NOMAD::CallbackType::USER_METHOD_SEARCH,
        userSearchMethodCallbackSpeculative
    );

    // mads->addCallback(
    //     NOMAD::CallbackType::USER_METHOD_SEARCH_2,
    //     userSearchMethodCallbackGP
    // );

    // Default quad model search must not consider categorical variables
    params->getRunParams()
        ->setListFixVGForQuadModelSearch(
            params->getPbParams(),
            myListFixVGForQMS
        );

    // Callback to generate Mads user poll trial points
    mads->addCallback(
        NOMAD::CallbackType::USER_METHOD_FREE_POLL,
        userPollMethodCallback
    );

    // Associate direction types and variable groups
    params->getRunParams()
        ->setMapDirTypeToVG(
            params->getPbParams(),
            myMapDirTypeToVG
        );

    // Set user extended poll method
    std::unique_ptr<NOMAD::ExtendedPollMethod>
        extendedPollMethod =
            std::make_unique<MyExtendedPollMethod2>(
                mads,
                ev
            );

    mads->setExtendedPollMethod(
        std::move(extendedPollMethod)
    );

    TheMainStep.run();
    TheMainStep.end();

    return 0;
}
