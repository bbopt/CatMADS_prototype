
#include "../Param/EvalParameters.hpp"
#include "../Type/BBInputType.hpp"
#include "../Type/BBOutputType.hpp"
#include "../Type/EvalSortType.hpp"
#include "../Util/fileutils.hpp"

/*----------------------------------------*/
/*         initializations (private)      */
/*----------------------------------------*/
void NOMAD::EvalParameters::init()
{
    _typeName = "Eval";

    try
    {
        #include "../Attribute/evalAttributesDefinition.hpp"
        registerAttributes( _definition );

    }
    catch (NOMAD::Exception& e)
    {
        std::string errorMsg = "Attribute registration failed: ";
        errorMsg += e.what();
        throw NOMAD::Exception(__FILE__,__LINE__, errorMsg);
    }
}

/*----------------------------------------*/
/*            check the parameters        */
/*----------------------------------------*/
void NOMAD::EvalParameters::checkAndComply(const std::shared_ptr<NOMAD::RunParameters>& runParams,
                                           const std::shared_ptr<NOMAD::PbParameters>& pbParams,
                                           const std::shared_ptr<EvaluatorControlGlobalParameters>& evaluatorControlGlobalParams,
                                           const std::shared_ptr<EvaluatorControlParameters>& evaluatorControlParams)
{
    checkInfo();

    if (!toBeChecked())
    {
        // Early out
        return;
    }

    /*--------------------------*/
    /* BB_EXE and SURROGATE_EXE */
    /*--------------------------*/
    // If SURROGATE_EXE is defined, verify that it is used.
    auto surrogateExe = getAttributeValueProtected<std::string>("SURROGATE_EXE", false);
    if (!surrogateExe.empty())
    {
        // Surrogate executable is defined. Verify that it is used.
        bool surrogateUsed = false;
        // Verify EVAL_QUEUE_SORT
        if (NOMAD::EvalSortType::SURROGATE == evaluatorControlParams->getAttributeValue<NOMAD::EvalSortType>("EVAL_QUEUE_SORT"))
        {
            surrogateUsed = true;
        }
        else if (evaluatorControlParams->getAttributeValue<bool>("EVAL_SURROGATE_OPTIMIZATION"))
        {
            surrogateUsed = true;
        }
        /*
        // TODO: See issue #584
        else if (runParams->getAttributeValue<bool>("VNS_MADS_SEARCH"))
        {
            surrogateUsed = true;
        }
        */

        if (!surrogateUsed)
        {
            throw NOMAD::InvalidParameter(__FILE__, __LINE__, "Parameter SURROGATE_EXE is defined but not used. To fix this, unset SURROGATE_EXE (if set by mistake), or set parameter EVAL_QUEUE_SORT to SURROGATE, or set parameter EVAL_SURROGATE_OPTIMIZATION to true.");
        }
    }

    updateExeParam(runParams, "BB_EXE");
    updateExeParam(runParams, "SURROGATE_EXE");


    /*----------------*/
    /* BB_OUTPUT_TYPE */
    /*----------------*/
    // The default value is empty: set a single OBJ
    auto bbOutputList = getAttributeValueProtected<NOMAD::BBOutputTypeList>("BB_OUTPUT_TYPE", false);
    if ( bbOutputList.empty() )
    {
        bbOutputList.emplace_back(NOMAD::BBOutputType::Type::OBJ);
        setAttributeValue("BB_OUTPUT_TYPE", bbOutputList );
    }
    
    /*----------------*/
    /* DISCO MADS */
    /*----------------*/
    auto itBBO_RPB = std::find(bbOutputList.begin(),bbOutputList.end(),BBOutputType::RPB);
    
    // Check if at least one revealing output for discoMads algorithm
    bool useAlgoDiscoMads = runParams->getAttributeValue<bool>("DISCO_MADS_OPTIMIZATION");
    if(useAlgoDiscoMads)
    {
        bool useDiscoMadsforDiscontinuity = !runParams->getAttributeValue<bool>("DISCO_MADS_HID_CONST");
        if(useDiscoMadsforDiscontinuity)
        {
            // Check if at least one revealing output for discoMads algorithm when used to reveal discontinuities
            const size_t nbRevealingOutput(NOMAD::getNbRevealing(bbOutputList));
            if(nbRevealingOutput==0)
            {
                throw NOMAD::Exception(__FILE__,__LINE__, "Parameters check: DiscoMads requires at least one revealing output" );
            }
            
            // Warn user if a EB constraint is set as revealing output
            bool EBConstRevealing = false;
            for (const auto& bbot : bbOutputList)
            {
                if (bbot.isRevealing()&&  bbot ==NOMAD::BBOutputType::EB)
                {
                    EBConstRevealing = true;
                    break;
                }
            }
            if(EBConstRevealing)
            {
                std::cerr << "At least one EB constraint is set as revealing. This is ok but be aware that revelation will only be conducted for points satisfying all EB constraints."<<  std::endl;
            }
        }

        // Add the revealed constraint RPB if not already present.
        // For suboptimization, updated BBoutputTypeList is passed -> not need to add another one
        if ( itBBO_RPB == bbOutputList.end())
        {
            bbOutputList.emplace_back(NOMAD::BBOutputType::RPB);
            setAttributeValue("BB_OUTPUT_TYPE", bbOutputList);
        }        
    }
    // User should not be able to set RPB constraint (internal).
    else if (itBBO_RPB != bbOutputList.end())
    {
        throw NOMAD::Exception(__FILE__,__LINE__, "Parameters check: User cannot set RPB constraint. Done internally for DISCO_MADS_OPTIMIZATION." );
    }
    
    /*---------------------------*/
    /* BB_EVAL_FORMAT (internal) */
    /*---------------------------*/
    
    // Copy of POINT_FORMAT
    auto pointFormat = pbParams->getAttributeValue<NOMAD::ArrayOfDouble>("POINT_FORMAT");
    setAttributeValue("BB_EVAL_FORMAT", pointFormat);

    _toBeChecked = false;

}
// End checkAndComply()


void NOMAD::EvalParameters::updateExeParam(const std::shared_ptr<NOMAD::RunParameters>& runParams, const std::string& paramName)
{
    // Update BB_EXE / SURROGATE_EXE if it was set by user:
    // - Set full path
    // - Remove '$' indicating a global call (e.g. python, perl)
    // - Verify file is executable
    if (isSetByUser(paramName))
    {
        auto exe = getAttributeValueProtected<std::string>(paramName, false);

        bool localExe = true;
        auto problemDir = runParams->getAttributeValue<std::string>("PROBLEM_DIR");

        if ('$' == exe[0])
        {
            // When the '$' character is put in first
            // position of a string, it is considered
            // as global and no path will be added.
            localExe = false;
        }

        // Convert arguments; add path as needed.
        auto exeAsArray = NOMAD::ArrayOfString(exe);
        exe.clear();
        for (size_t i = 0; i < exeAsArray.size(); i++)
        {
            std::string word = exeAsArray[i];
            if (i > 0)
            {
                exe += " ";
            }

            if ('$' == word[0])
            {
                exe += word.substr(1, word.size());
            }
            else
            {
                // word is relative to problem directory.
                NOMAD::completeFileName(word, problemDir);
                exe += word;
            }
        }

        setAttributeValue(paramName, exe);
        exeAsArray = NOMAD::ArrayOfString(exe);

        if (localExe && !exe.empty() && !checkExeFile(exeAsArray[0]))
        {
            throw NOMAD::Exception(__FILE__, __LINE__, paramName + " needs to be an executable file: " + exeAsArray[0]);
        }
    }
}



