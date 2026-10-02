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

#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>


// -----------------------------------------------------------------------------
// Problem setup: Cat-Suite Cat-23 (Hal-04)
// -----------------------------------------------------------------------------

const int Ncat = 3;
const int Nint = 0;
const int Ncon = 5;
const int N = Ncat + Nint + Ncon;
const int Lcat = 12;

const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {
    NOMAD::BBOutputType::OBJ
};

const bool IsConstrained = false;


// -----------------------------------------------------------------------------
// Global variables used by the CatMADS prototype callbacks
// -----------------------------------------------------------------------------

bool LastSuccessIsQuantitative = false;
bool LastSuccessIsCategorical = false;
bool isCatDistanceUpdated = true;


// -----------------------------------------------------------------------------
// Black-box evaluator
// -----------------------------------------------------------------------------

class My_Evaluator : public NOMAD::Evaluator
{
public:
    explicit My_Evaluator(
        const std::shared_ptr<NOMAD::EvalParameters>& evalParams
    )
        : NOMAD::Evaluator(evalParams, NOMAD::EvalType::BB)
    {}

    ~My_Evaluator() = default;

    bool eval_x(
        NOMAD::EvalPoint& x,
        const NOMAD::Double& hMax,
        bool& countEval
    ) const override;
};


bool My_Evaluator::eval_x(
    NOMAD::EvalPoint& x,
    const NOMAD::Double& hMax,
    bool& countEval
) const
{
    (void)hMax;

    // x = [x_cat1, x_cat2, x_cat3, x_cont1, ..., x_cont5]
    if (x.size() != static_cast<size_t>(N))
    {
        throw NOMAD::Exception(
            __FILE__,
            __LINE__,
            "Dimension mismatch: expected Ncat + Nint + Ncon variables."
        );
    }

    // Categorical encoding:
    // x_cat1: 0 -> A, 1 -> B, 2 -> C
    // x_cat2: 0 -> D, 1 -> E
    // x_cat3: 0 -> F, 1 -> G
    const int x_cat1 = static_cast<int>(x[0].todouble());
    const int x_cat2 = static_cast<int>(x[1].todouble());
    const int x_cat3 = static_cast<int>(x[2].todouble());

    if (x_cat1 < 0 || x_cat1 > 2 ||
        x_cat2 < 0 || x_cat2 > 1 ||
        x_cat3 < 0 || x_cat3 > 1)
    {
        throw NOMAD::Exception(
            __FILE__,
            __LINE__,
            "Invalid categorical value for Cat-Suite Cat-23 (Hal-04)."
        );
    }

    std::vector<double> xc(Ncon);
    for (int k = 0; k < Ncon; ++k)
    {
        xc[k] = x[Ncat + Nint + k].todouble();
    }

    // s(x^cat), exactly as in Cat-Suite Cat-23 (Hal-04).
    static const double table_s[3][2][2] = {
        // x_cat1 = A
        {
            { 0.00,  0.20 },   // x_cat2 = D: F, G
            {-0.10,  0.25 }    // x_cat2 = E: F, G
        },
        // x_cat1 = B
        {
            { 0.50, -0.20 },   // x_cat2 = D: F, G
            {-0.50,  0.80 }    // x_cat2 = E: F, G
        },
        // x_cat1 = C
        {
            { 0.90, -0.50 },   // x_cat2 = D: F, G
            { 1.00,  1.25 }    // x_cat2 = E: F, G
        }
    };

    const double s = table_s[x_cat1][x_cat2][x_cat3];

    // Cat-Suite Cat-23 objective:
    //
    // f(x) = sum_{i=1}^5 [
    //     5 x_i
    //     + 0.3 s x_i^4
    //     + (1 - x_i)^2
    //     + s sin(3 pi x_i + i pi / 5)
    // ] 2^((i-1)/4)
    //
    // IMPORTANT: each quartic and sine term appears exactly once.
    const double pi = std::acos(-1.0);
    double f = 0.0;

    for (int i = 1; i <= Ncon; ++i)
    {
        const double xi = xc[i - 1];
        const double weight = std::pow(
            2.0,
            static_cast<double>(i - 1) / 4.0
        );

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

    NOMAD::Double F(f);
    x.setBBO(F.tostring());

    countEval = true;
    return true;
}


// -----------------------------------------------------------------------------
// NOMAD parameters
// -----------------------------------------------------------------------------

void initAllParams(
    std::shared_ptr<NOMAD::AllParameters> allParams,
    std::map<NOMAD::DirectionType, NOMAD::ListOfVariableGroup>& myMapDirTypeToVG,
    NOMAD::ListOfVariableGroup& myListFixVGForQMS
)
{
    // Dimension and budget.
    allParams->setAttributeValue("DIMENSION", N);
    allParams->setAttributeValue("MAX_BB_EVAL", nbEvals);

    // Initial LHS search.
    const std::string budgetLHsFormat = std::to_string(nbEvalsLHS) + " 0";
    allParams->setAttributeValue(
        "LH_SEARCH",
        NOMAD::LHSearchType(budgetLHsFormat.c_str())
    );

    // Bounds.
    NOMAD::ArrayOfDouble lb(N);
    NOMAD::ArrayOfDouble ub(N);

    // Categorical variables.
    lb[0] = 0;
    lb[1] = 0;
    lb[2] = 0;

    ub[0] = 2;
    ub[1] = 1;
    ub[2] = 1;

    // Continuous variables, Cat-Suite Cat-23 bounds.
    lb[Ncat + Nint + 0] = -9;
    lb[Ncat + Nint + 1] = -7;
    lb[Ncat + Nint + 2] = -10;
    lb[Ncat + Nint + 3] = -8;
    lb[Ncat + Nint + 4] = -6;

    ub[Ncat + Nint + 0] = 12;
    ub[Ncat + Nint + 1] = 14;
    ub[Ncat + Nint + 2] = 10;
    ub[Ncat + Nint + 3] = 13;
    ub[Ncat + Nint + 4] = 15;

    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);

    // NOMAD sees categorical variables as integer variables; CatMADS handles
    // their categorical semantics through the dedicated variable group/free poll.
    NOMAD::BBInputTypeList bbinput = {
        NOMAD::BBInputType::INTEGER,
        NOMAD::BBInputType::INTEGER,
        NOMAD::BBInputType::INTEGER,
        NOMAD::BBInputType::CONTINUOUS,
        NOMAD::BBInputType::CONTINUOUS,
        NOMAD::BBInputType::CONTINUOUS,
        NOMAD::BBInputType::CONTINUOUS,
        NOMAD::BBInputType::CONTINUOUS
    };

    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Variable groups.
    const NOMAD::VariableGroup vgCat = {0, 1, 2};
    const NOMAD::VariableGroup vgQuant = {3, 4, 5, 6, 7};

    allParams->setAttributeValue(
        "VARIABLE_GROUP",
        NOMAD::ListOfVariableGroup({vgCat, vgQuant})
    );

    // Two subpolls:
    //  - categorical free poll on vgCat
    //  - ORTHO 2N quantitative poll on vgQuant
    NOMAD::DirectionTypeList dtList = {
        NOMAD::DirectionType::USER_FREE_POLL,
        NOMAD::DirectionType::ORTHO_2N
    };

    allParams->setAttributeValue("DIRECTION_TYPE", dtList);

    myMapDirTypeToVG = {
        {dtList[0], {vgCat}},
        {dtList[1], {vgQuant}}
    };

    // Objective.
    allParams->setAttributeValue("BB_OUTPUT_TYPE", bbOutputTypeListSetup);

    // Quad-model search: categorical variables remain fixed.
    allParams->setAttributeValue("QUAD_MODEL_SEARCH", true);
    myListFixVGForQMS = {vgCat};

    // Disable NOMAD's default NM and speculative searches. The prototype
    // supplies its own speculative-search callback below.
    allParams->setAttributeValue("NM_SEARCH", false);
    allParams->setAttributeValue("SPECULATIVE_SEARCH", false);

    // Enable user search for the prototype callback.
    allParams->setAttributeValue("USER_SEARCH", true);

    // Display.
    allParams->setAttributeValue("DISPLAY_DEGREE", 2);
    allParams->setAttributeValue(
        "DISPLAY_STATS",
        NOMAD::ArrayOfString("bbe ( sol ) obj cons_h")
    );
    allParams->setAttributeValue("DISPLAY_ALL_EVAL", true);

    // Seed.
    allParams->setAttributeValue("SEED", seedSetup);
    allParams->setAttributeValue("RNG_ALT_SEEDING", true);

    // History.
    allParams->setAttributeValue(
        "STATS_FILE",
        NOMAD::ArrayOfString("hal04.txt bbe sol obj cons_h")
    );

    allParams->checkAndComply();
}


// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    std::vector<std::string> filesToClear = {
        fileCache,
        fileCatDirections,
        fileParams
    };

    deleteFiles(filesToClear);

    NOMAD::MainStep TheMainStep;

    auto params = std::make_shared<NOMAD::AllParameters>();

    std::map<NOMAD::DirectionType, NOMAD::ListOfVariableGroup>
        myMapDirTypeToVG;

    NOMAD::ListOfVariableGroup myListFixVGForQMS;

    // Initialize parameters exactly once.
    initAllParams(
        params,
        myMapDirTypeToVG,
        myListFixVGForQMS
    );

    TheMainStep.setAllParameters(params);

    // Keep a shared reference to the evaluator because the prototype extended
    // poll also receives it later.
    std::shared_ptr<NOMAD::Evaluator> ev =
        std::make_shared<My_Evaluator>(params->getEvalParams());

    // MainStep takes ownership of one shared_ptr instance. Keep another copy
    // for MyExtendedPollMethod2, which also needs the evaluator.
    auto evForMainStep = ev;
    TheMainStep.setEvaluator(std::move(evForMainStep));

    // Initialize MADS/evaluator control.
    TheMainStep.start();

    // Custom categorical ordering.
    auto customOrder = std::make_shared<CustomOrder>();
    NOMAD::EvcInterface::getEvaluatorControl()->setUserCompMethod(customOrder);

    // Post-evaluation callback: tracks categorical vs quantitative successes.
    NOMAD::EvalCallbackFunc<NOMAD::CallbackType::POST_EVAL_UPDATE>
        cbPostEvalUpdate = customPostEvalUpdateCB;

    NOMAD::EvcInterface::getEvaluatorControl()
        ->addEvalCallback<NOMAD::CallbackType::POST_EVAL_UPDATE>(
            cbPostEvalUpdate
        );

    // Access the MADS algorithm.
    auto mads = std::dynamic_pointer_cast<NOMAD::Mads>(
        TheMainStep.getAlgo(NOMAD::StepType::ALGORITHM_MADS)
    );

    if (nullptr == mads)
    {
        throw NOMAD::Exception(
            __FILE__,
            __LINE__,
            "Cannot access Mads algorithm"
        );
    }

    // Prototype speculative-search callback.
    mads->addCallback(
        NOMAD::CallbackType::USER_METHOD_SEARCH,
        userSearchMethodCallbackSpeculative
    );

    // Quad-model search must not modify categorical variables.
    params->getRunParams()->setListFixVGForQuadModelSearch(
        params->getPbParams(),
        myListFixVGForQMS
    );

    // Categorical free-poll callback.
    mads->addCallback(
        NOMAD::CallbackType::USER_METHOD_FREE_POLL,
        userPollMethodCallback
    );

    params->getRunParams()->setMapDirTypeToVG(
        params->getPbParams(),
        myMapDirTypeToVG
    );

    // Prototype extended poll.
    std::unique_ptr<NOMAD::ExtendedPollMethod> extendedPollMethod =
        std::make_unique<MyExtendedPollMethod2>(mads, ev);

    mads->setExtendedPollMethod(std::move(extendedPollMethod));

    TheMainStep.run();
    TheMainStep.end();

    return 0;
}
