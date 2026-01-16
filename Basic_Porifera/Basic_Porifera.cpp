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
//#include "../CatMADS/CatMADS.hpp"
//#include "../CatMADS/MyExtendedPoll/MyExtendedPollMethod2.hpp"


// Setup of the problem
const int Ncat=2;
const int Nint=0;
const int Ncon=2;
const int N=Ncat+Nint+Ncon;
const int Lcat=81;
const NOMAD::BBOutputTypeList bbOutputTypeListSetup = {NOMAD::BBOutputType::OBJ, 
                                                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::EB,
                                                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
                                                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB,
                                                        NOMAD::BBOutputType::PB, NOMAD::BBOutputType::PB};

// Harcoded (from CatMADS.cpp)
const int nbEvalsPerVariable=10; //250
const int nbEvals = N*nbEvalsPerVariable; // N is initialized in a problem specific folder
const int nbEvalsLHS=static_cast<int>(nbEvals*0.2); //0.2 for GPCatMADS 
const int seedSetup = 0;

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
/*------------------------------------x----*/
bool My_Evaluator::eval_x(NOMAD::EvalPoint &x,
                          const NOMAD::Double &hMax,
                          bool &countEval) const
{
    // Ensure the input dimension matches the expected size
    if (x.size() != (N)) // n_cat + n_int + n_con
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Dimension mismatch: Ensure the number of variables matches n_cat + n_int + n_con.");
    }
    
    // TODO 
    std::ostringstream command{};
    //command << "docker run porifera-elastic " << x[0] << " " << x[1] << " " << x[2] << " " << x[3] << " 2>/dev/null | sed 1d >X.txt";
    command << "docker run porifera-elastic " << x[0] << " " << x[1] << " " << x[2] << " " << x[3] << " >out.txt";

    std::cout << "the string was: " << command.str() << std::endl;

    // Execute the command and capture the exit code<
    int exitCode = system(command.str().c_str());

    // Handle normal termination
    if (!WIFEXITED(exitCode) || WEXITSTATUS(exitCode) != 0) {
        throw std::runtime_error("Error running Porifera.");
    }

    // ---------------------------- //
    // Read
    std::ifstream file("out.txt");
    std::vector<std::vector<double>> result;
    std::string line;

    countEval = true;
    if(std::getline(file, line)){
        // TODO NEXT: parse new data output formatting 
        x.setBBO(line);
        return true;
    }
    else{
        // TODO throw exception
        return false;
    }
    // ---------------------------- //


}


void initAllParams( std::shared_ptr<NOMAD::AllParameters> allParams, std::map<NOMAD::DirectionType,NOMAD::ListOfVariableGroup> & myMapDirTypeToVG, NOMAD::ListOfVariableGroup & myListFixVGForQMS)
{

    // Parameters creation
    allParams->setAttributeValue("DIMENSION", N);
    // Black-box evaluations
    allParams->setAttributeValue("MAX_BB_EVAL", nbEvals);
    // LHS
    std::string budgetLHsFormat = std::to_string(nbEvalsLHS) + " 0";
    allParams->setAttributeValue("LH_SEARCH", NOMAD::LHSearchType(budgetLHsFormat.c_str()));

    // Bounds for all variables 
    auto lb = NOMAD::ArrayOfDouble(N, 0.0);
    auto ub = NOMAD::ArrayOfDouble(N, 0.0);
    // Categorical lower bounds
    lb[0] = 0; 
    lb[1] = -1;
    // Categorical upper bounds
    ub[0] = 26; 
    ub[1] = 1;
    // Continuous lower bounds
    lb[2] = -1; 
    lb[3] = 0.5;
    // Continuous upper bounds
    ub[2] = 3;
    ub[3] = 2;
    allParams->setAttributeValue("LOWER_BOUND", lb);
    allParams->setAttributeValue("UPPER_BOUND", ub);
    
    // Types
    NOMAD::BBInputTypeList bbinput = {
    NOMAD::BBInputType::INTEGER, NOMAD::BBInputType::INTEGER,
    NOMAD::BBInputType::CONTINUOUS, NOMAD::BBInputType::CONTINUOUS};
    allParams->setAttributeValue("BB_INPUT_TYPE", bbinput);

    // Constraints and objective
    allParams->setAttributeValue("BB_OUTPUT_TYPE", bbOutputTypeListSetup);

    // Quad search where the first group of variables is fixed
    allParams->setAttributeValue("QUAD_MODEL_SEARCH", true);

    // Default searches that are deactivated 
    allParams->setAttributeValue("NM_SEARCH", false);
    allParams->setAttributeValue("SPECULATIVE_SEARCH", false);
    
    // Display
    allParams->setAttributeValue("DISPLAY_DEGREE", 2);
    allParams->setAttributeValue("DISPLAY_STATS", NOMAD::ArrayOfString("bbe ( sol ) obj cons_h"));
    allParams->setAttributeValue("DISPLAY_ALL_EVAL", true);

    // Fix seed for duplicity of results
    allParams->setAttributeValue("SEED", seedSetup);
    allParams->setAttributeValue("RNG_ALT_SEEDING", true);

    // File history for convergence plots and profiles
    allParams->setAttributeValue("STATS_FILE", NOMAD::ArrayOfString("basic_porifera.txt bbe sol obj cons_h"));

    // Parameters validation
    allParams->checkAndComply();
    
}


/*------------------------------------------*/
/*            NOMAD main function           */
/*------------------------------------------*/
int main ( int argc , char ** argv )
{

    NOMAD::MainStep TheMainStep;

    // Set parameters
    auto params = std::make_shared<NOMAD::AllParameters>();
    TheMainStep.setAllParameters(params);

    // Custom Evaluator
    std::shared_ptr<NOMAD::Evaluator> ev(new My_Evaluator(params->getEvalParams()));
    TheMainStep.setEvaluator(std::move(ev));
    
    // Main step start initializes Mads (default algorithm)
    TheMainStep.start();
    TheMainStep.run();
    TheMainStep.end();

    return 0;
}