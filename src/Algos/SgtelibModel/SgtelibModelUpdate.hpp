#ifndef __NOMAD_4_5_SGTELIB_MODEL_UPDATE__
#define __NOMAD_4_5_SGTELIB_MODEL_UPDATE__

#include "../../Algos/Step.hpp"
#include "../../Output/OutputInfo.hpp"  // for OutputLevel

#include "../../nomad_nsbegin.hpp"

class SgtelibModelUpdate : public Step
{
private:
    OutputLevel _displayLevel;

public:
    explicit SgtelibModelUpdate(const Step* parentStep)
      : Step(parentStep),
        _displayLevel(OutputLevel::LEVEL_INFO)
    {
        init();
    }

    virtual ~SgtelibModelUpdate();

    std::string getName() const override;

private:
    void init();

    virtual void startImp() override;
    virtual bool runImp() override;
    virtual void endImp() override;

    static bool validForUpdate(const EvalPoint& evalPoint);

};

#include "../../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SGTELIB_MODEL_UPDATE__
