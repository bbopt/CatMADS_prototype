#ifndef __NOMAD_4_5_DEPRECATEDPARAMETERS__
#define __NOMAD_4_5_DEPRECATEDPARAMETERS__

#include "../Param/Parameters.hpp"

#include "../nomad_nsbegin.hpp"

/// The class for deprecated parameters.
/**
 - Register all deprecated parameters during construction.
 - Trigger exception if a deprecated parameter has been explicitly set.
*/
class DLL_UTIL_API DeprecatedParameters final : public Parameters
{
public:
    /// Constructor
    explicit DeprecatedParameters()
    : Parameters()
    {
        init();
    }
    /// Read and detect explicitly set deprecated parameter. Trigger exception.
    void readAndDetectExplicitlySet( );

private:
    /// Helper for constructor
    /**
     Register and set default values for all deprecated attributes. The information to register all the attributes is contained in deprecatedAttributesDefinition.hpp as a set of strings to be interpreted. This file is created by the writeAttributeDefinition executable from the deprecatedAttributeDefinition.txt.
     */
    void init() override ;

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_DEPRECATEDPARAMETERS__

