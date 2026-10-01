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
    {}

    ~My_Evaluator() {}

    bool eval_x(
        NOMAD::EvalPoint& x,
        const NOMAD::Double& hMax,
        bool& countEval
    ) const override;
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
bool My_Evaluator::eval_x(
    NOMAD::EvalPoint& x,
    const NOMAD::Double& hMax,
    bool& countEval
) const
{
    (void)hMax;

    // Expect:
    // x = [x_cat1, x_cat2, x_cat3, x_con1, ..., x_con5]
    if (x.size() != (Ncat + Nint + Ncon))
    {
        throw NOMAD::Exception(
            __FILE__,
            __LINE__,
            "Dimension mismatch: expected Ncat + Nint + Ncon variables."
        );
        throw NOMAD::Exception(
            __FILE__,
            __LINE__,
            "Dimension mismatch: expected Ncat + Nint + Ncon variables."
        );
    }

    // ------------------------------------------------------------
    // Categorical variables
    //
    // x_cat1:
    //   0 -> A
    //   1 -> B
    //   2 -> C
    //
    // x_cat2:
    //   0 -> D
    //   1 -> E
    //
    // x_cat3:
    //   0 -> F
    //   1 -> G
    // ------------------------------------------------------------

    const int x_cat1 = static_cast<int>(x[0].todouble());
    const int x_cat2 = static_cast<int>(x[1].todouble());
    const int x_cat3 = static_cast<int>(x[2].todouble());

    // ------------------------------------------------------------
    // Continuous variables x1,...,x5
    // ------------------------------------------------------------

    std::vector<double> xc(Ncon);


    for (int k = 0; k < Ncon; ++k)
    {
        xc[k] = x[Ncat + Nint + k].todouble();
    }

    // ------------------------------------------------------------
    // s(x^cat)
    //
    // table_s[x_cat1][x_cat2][x_cat3]
    // ------------------------------------------------------------

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

    // ------------------------------------------------------------
    // Objective
    //
    // f(x) =
    //
    // sum_{i=1}^5 [
    //     5 x_i
    //     + 0.3 s x_i^4
    //     + (1-x_i)^2
    //     + s sin(3 pi x_i + i pi / 5)
    // ] 2^((i-1)/4)
    // ------------------------------------------------------------

    const double pi = M_PI;

    double f = 0.0;

    for (int i = 1; i <= 5; ++i)
    {
        const double xi = xc[i - 1];

        const double weight =
            std::pow(
                2.0,
                static_cast<double>(i - 1) / 4.0
            );

        const double term =
            5.0 * xi
            + 0.3 * s * std::pow(xi, 4)
            + 0.3 * s * std::pow(xi, 4)
            + std::pow(1.0 - xi, 2)
            + s * std::sin(
                3.0 * pi * xi
                + (static_cast<double>(i) * pi) / 5.0
            );
            + s * std::sin(
                3.0 * pi * xi
                + (static_cast<double>(i) * pi) / 5.0
            );

        f += term * weight;
    }

    // Return to NOMAD
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
    // Dimension
    allParams->setAttributeValue("DIMENSION", N);


    // Black-box evaluations
    allParams->setAttributeValue("MAX_BB_EVAL", nbEvals);

    // LHS
    std::string budgetLHsFormat =
        std::to_string(nbEvalsLHS) + " 0";

    allParams->setAttributeValue(
        "LH_SEARCH",
        NOMAD::LHSearchType(budgetLHsFormat.c_str())
    );
    std::string budgetLHsFormat =
        std::to_string(nbEvalsLHS) + " 0";

    allParams->setAttributeValue(
        "LH_SEARCH",
        NOMAD::LHSearchType(budgetLHsFormat.c_str())
    );

    // ------------------------------------------------------------
    // Bounds
    // ------------------------------------------------------------

    auto lb = NOMAD::ArrayOfDouble(N, -6.0);
    auto ub = NOMAD::ArrayOfDouble(N,  9.0);

    // Categorical variables
    //
    // cat1 = A,B,C -> 0,1,2
    // cat2 = D,E   -> 0,1
    // cat3 = F,G   -> 0,1

    lb[0] = 0;
    lb[1] = 0;
    lb[2] = 0;

    ub[0] = 2;
    ub[1] = 1;
    ub[2] = 1;


    // Continuous lower bounds
    lb[Ncat + 0] = -9;
    lb[Ncat + 1] = -7;
    lb[Ncat + 2] = -10;
    lb[Ncat + 3] = -8;
    lb[Ncat + 4] = -6;

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

    ub[Ncat + 0] = 12;
    ub[Ncat + 1] = 14;
    ub[Ncat + 2] = 10;
    ub[Ncat + 3] = 13;
    ub[Ncat + 4] = 15;

    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);

    // ------------------------------------------------------------
    // Variable types
    // ------------------------------------------------------------

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

    allParams->setAttributeValue(
        "BB_INPUT_TYPE",
        bbinput
    );

    // ------------------------------------------------------------
    // Variable groups
    // ------------------------------------------------------------

    NOMAD::VariableGroup vg0 = {
        0, 1, 2
    };

    NOMAD::VariableGroup vg1 = {
        3, 4, 5, 6, 7
    };

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

    myMapDirTypeToVG = {
        {dtList[0], {vg0}},
        {dtList[1], {vg1}}
    };

    // Objective
    allParams->setAttributeValue(
        "BB_OUTPUT_TYPE",
        bbOutputTypeListSetup
    );

    // Quad model search:
    // categorical variables fixed
    allParams->setAttributeValue(
        "QUAD_MODEL_SEARCH",
        true
    );

    myListFixVGForQMS = {vg0};

    // Default searches deactivated
    allParams->setAttributeValue(
        "NM_SEARCH",
        false
    );

    allParams->setAttributeValue(
        "SPECULATIVE_SEARCH",
        false
    );

    // Enable user search
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

    // Seed
    allParams->setAttributeValue(
        "SEED",
        seedSetup
    );

    allParams->setAttributeValue(
        "RNG_ALT_SEEDING",
        true
    );

    // History
    allParams->setAttributeValue(
        "STATS_FILE",
        NOMAD::ArrayOfString(
            "hal04.txt bbe sol obj cons_h"
        )
    );

    // Validation
    allParams->checkAndComply();
}


/*------------------------------------------*/
/*            NOMAD main function           */
/*------------------------------------------*/
int main(int argc, char** argv)
int main(int argc, char** argv)
{
    std::vector<std::string> filesToClear = {
        fileCache,
        fileCatDirections,
        fileParams
    };

    deleteFiles(filesToClear);

    NOMAD::MainStep TheMainStep;

    // Parameters
    auto params =
        std::make_shared<NOMAD::AllParameters>();

    std::map<
        NOMAD::DirectionType,
        NOMAD::ListOfVariableGroup
    > myMapDirTypeToVG;

    NOMAD::ListOfVariableGroup
        myListFixVGForQMS;

    initAllParams(
        params,
        myMapDirTypeToVG,
        myListFixVGForQMS
    );

    initAllParams(
        params,
        myMapDirTypeToVG,
        myListFixVGForQMS
    );

    TheMainStep.setAllParameters(params);

    // Custom evaluator
    std::shared_ptr<NOMAD::Evaluator> ev(
        new My_Evaluator(
            params->getEvalParams()
        )
    );

    TheMainStep.setEvaluator(
        std::move(ev)
    );

    // Initialize MADS
    TheMainStep.start();

    // Custom ordering
    auto customOrder =
        std::make_shared<CustomOrder>();

    NOMAD::EvcInterface::
        getEvaluatorControl()->
        setUserCompMethod(customOrder);

    // Post-evaluation callback
    NOMAD::EvalCallbackFunc<
        NOMAD::CallbackType::POST_EVAL_UPDATE
    > cbPostEvalUpdate =
        customPostEvalUpdateCB;

    NOMAD::EvcInterface::
        getEvaluatorControl()->
        addEvalCallback<
            NOMAD::CallbackType::POST_EVAL_UPDATE
        >(cbPostEvalUpdate);

    // Access MADS
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

    // User search
    mads->addCallback(
        NOMAD::CallbackType::USER_METHOD_SEARCH,
        userSearchMethodCallbackSpeculative
    );

    // QMS does not modify categorical variables
    params->getRunParams()->
        setListFixVGForQuadModelSearch(
            params->getPbParams(),
            myListFixVGForQMS
        );

    // Categorical poll
    mads->addCallback(
        NOMAD::CallbackType::USER_METHOD_FREE_POLL,
        userPollMethodCallback
    );

    params->getRunParams()->
        setMapDirTypeToVG(
            params->getPbParams(),
            myMapDirTypeToVG
        );

    // Extended poll
    std::unique_ptr<NOMAD::ExtendedPollMethod>
        extendedPollMethod =
            std::make_unique<
                MyExtendedPollMethod2
            >(mads, ev);

    mads->setExtendedPollMethod(
        std::move(extendedPollMethod)
    );

    TheMainStep.run();
    TheMainStep.end();

    return 0;
}