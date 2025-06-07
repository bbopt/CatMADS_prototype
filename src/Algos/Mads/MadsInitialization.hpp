#ifndef __NOMAD_4_5_MADSINITIALIZATION__
#define __NOMAD_4_5_MADSINITIALIZATION__

#include "../../Algos/Initialization.hpp"
#include "../../Type/BBInputType.hpp"

#include "../../nomad_nsbegin.hpp"

/// Class for Mads initialization (step 0)
/**
 The run function of this step validates and evaluates X0(s).
 Initialization of the mesh is performed at construction.
 */
class MadsInitialization : public Initialization
{
    
private:
    
    BBInputTypeList _bbInputType;
    
    Double _hMax0;  ///< Initial HMax for Mads barrier
    
protected:
    MeshBasePtr _initialMesh;
    
    bool _barrierInitializedFromCache;
    bool _isUsedForDMultiMads;
    bool _isUsedForDiscoMads;

public:
    /// Constructor
    /*
     \param parentStep                   The parent of this step -- \b IN.
     \param barrierInitializedFromCache  Flag to initialize barrier from cache or not -- \b IN.
     */
    explicit MadsInitialization(const Step* parentStep, bool barrierInitializedFromCache=true, bool isUsedForDMultiMads=false, bool isUsedForDiscoMads=false)
      : Initialization(parentStep),
        _initialMesh(nullptr),
        _barrierInitializedFromCache(barrierInitializedFromCache),
        _isUsedForDMultiMads(isUsedForDMultiMads),
        _isUsedForDiscoMads(isUsedForDiscoMads)
    {
        init();
    }

    virtual ~MadsInitialization() {}

    MeshBasePtr getMesh() const { return _initialMesh; }

private:
    void init();

    bool eval_x0s();

protected:
    
    virtual bool runImp() override;

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_MADSINITIALIZATION__
