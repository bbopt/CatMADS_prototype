
#include "../Param/DeprecatedParameters.hpp"
#include "../Util/fileutils.hpp"

/*----------------------------------------*/
/*         initializations (private)      */
/*----------------------------------------*/
void NOMAD::DeprecatedParameters::init()
{
    _typeName = "Deprecated";

    try
    {
        #include "../Attribute/deprecatedAttributesDefinition.hpp"
        registerAttributes( _definition );
    }
    catch (NOMAD::Exception& e)
    {
        std::string errorMsg = "Attribute registration failed: ";
        errorMsg += e.what();
        throw NOMAD::Exception(__FILE__,__LINE__, errorMsg);
    }
}

//*-------------------------------------------------*/
//*  Read and detect the parameters explicitly set  */
//*-------------------------------------------------*/
void NOMAD::DeprecatedParameters::readAndDetectExplicitlySet()
{

    std::shared_ptr<NOMAD::ParameterEntry> pe;
    std::string paramName;

    // Loop on all registered attributes.
    // Set the attribute value from entries
    // according to its type
    for(const auto &att : getAttributes())
    {
        paramName = att->getName();
        pe = _paramEntries.find(paramName);
        if (pe)
        {
            std::string err = "\n " + att->getName() + att->getShortInfo();
            throw NOMAD::Exception(__FILE__,__LINE__, err );
        }
    }
}



