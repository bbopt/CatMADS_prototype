#ifndef __NOMAD_4_5_TEMPLATESIMPLESEARCHMETHOD__
#define __NOMAD_4_5_TEMPLATESIMPLESEARCHMETHOD__

#include "../../Algos/Mads/SearchMethodSimple.hpp"

#include "../../nomad_nsbegin.hpp"


/// Template example for simple search method.
/**
Called when RANDOM_SIMPLE_SEARCH is enabled.
 
 Can be used as a TEMPLATE for a new search method: copy and rename the file and the class name. Adapt the code to your needs.  It is IMPORTANT to register the new search method in ../Algos/Mads/Search.cpp (NOMAD::Search::init()).
 */
class TemplateSimpleSearchMethod  final : public SearchMethodSimple
{
public:
    /// Constructor
    /**
     \param parentStep      The parent of this search step -- \b IN.
     */
    explicit TemplateSimpleSearchMethod(const Step* parentStep )
    : SearchMethodSimple( parentStep )
    {
        init();
    }

private:
    void init();

    /// Generate new points to evaluate
    /**
     \copydoc SearchMethodSimple::generateTrialPointsFinal \n
     The search method generates random trial points around best incumbent .
     */
    void generateTrialPointsFinal() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_TEMPLATESIMPLESEARCHMETHOD__
