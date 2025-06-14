/**
 \file   BBOutput.cpp
 \brief  Output from a Blackbox evaluation
 \author Viviane Rochon Montplaisir
 \date   January 2018
 \see    BBOutput.hpp
 */
#include <utility>

#include "../Eval/BBOutput.hpp"

// Initialize static variables
const std::string NOMAD::BBOutput::bboStart = "(";
const std::string NOMAD::BBOutput::bboEnd = ")";


/*---------------------------------------------------------------------*/
/*                            Constructors                              */
/*---------------------------------------------------------------------*/
// Reading BBOutput from string
NOMAD::BBOutput::BBOutput(std::string rawBBO, const bool evalOk)
  : _rawBBO(std::move(rawBBO)),
    _evalOk(evalOk)
{
    NOMAD::ArrayOfString array(_rawBBO);
    _BBO =  ArrayOfDouble( array.size() );
    for (size_t i = 0; i < array.size(); i++)
    {
        NOMAD::Double d;
        d.atof(array[i]);
        _BBO[i] = d;
    }
}
// Reading BBOutput from ArrayOfDouble
NOMAD::BBOutput::BBOutput(const ArrayOfDouble & bbo)
  : _BBO(bbo)
{
    _evalOk = true;
    for (size_t i = 0; i < _BBO.size(); i++)
    {
        if (!_BBO.isDefined())
        {
            _evalOk = false;
            break;
        }
    }
}


void NOMAD::BBOutput::setBBO(const std::string &bbOutputString, const bool evalOk)
{
    _rawBBO = bbOutputString;
    _evalOk = evalOk;
    NOMAD::ArrayOfString array(_rawBBO);
    _BBO =  ArrayOfDouble( array.size() );
    for (size_t i = 0; i < array.size(); i++)
    {
        NOMAD::Double d;
        d.atof(array[i]);
        _BBO[i] = d;
    }
}


bool NOMAD::BBOutput::getCountEval(const BBOutputTypeList &bbOutputType) const
{
    bool countEval = true;

    for (size_t i = 0; i < _BBO.size(); i++)
    {
        if (bbOutputType[i] == NOMAD::BBOutputType::Type::CNT_EVAL)
        {
            countEval = (bool) _BBO[i].todouble();
        }
    }

    return countEval;
}


bool NOMAD::BBOutput::isComplete(const NOMAD::BBOutputTypeList &bbOutputType) const
{
    bool itIsComplete = true;
    if (!bbOutputType.empty() && checkSizeMatch(bbOutputType))
    {
        for (size_t i = 0; i < _BBO.size(); i++)
        {
            if (bbOutputType[i].isObjective()
                || bbOutputType[i].isConstraint())
            {
                if (!_BBO[i].isDefined())
                {
                    itIsComplete = false;
                    break;
                }
            }
        }
    }
    else
    {
        itIsComplete = false;
    }

    return itIsComplete;
}


NOMAD::Double NOMAD::BBOutput::getObjective(const NOMAD::BBOutputTypeList &bbOutputType) const
{
    NOMAD::Double obj;

    if (_evalOk && !bbOutputType.empty() && checkSizeMatch(bbOutputType))
    {
        for (size_t i = 0; i < _BBO.size(); i++)
        {
            if (bbOutputType[i].isObjective())
            {
                obj = _BBO[i];
                break;
            }
        }
    }
    return obj;
}

NOMAD::ArrayOfDouble NOMAD::BBOutput::getObjectives(const NOMAD::BBOutputTypeList &bbOutputType) const
{
    NOMAD::ArrayOfDouble objectives;

    if (_evalOk && !bbOutputType.empty() && checkSizeMatch(bbOutputType))
    {
        for (size_t i = 0; i < _BBO.size(); i++)
        {
            if (bbOutputType[i].isObjective())
            {
                size_t objSize = objectives.size();
                objectives.resize(objSize + 1);
                objectives[objSize] = _BBO[i];
            }
        }
    }

    return objectives;
}


NOMAD::ArrayOfDouble NOMAD::BBOutput::getConstraints(const NOMAD::BBOutputTypeList &bbOutputType) const
{
    NOMAD::ArrayOfDouble constraints;

    if (_evalOk && !bbOutputType.empty() && checkSizeMatch(bbOutputType))
    {
        for (size_t i = 0; i < _BBO.size(); i++)
        {
            if ( bbOutputType[i].isConstraint())
            {
                size_t constrSize = constraints.size();
                constraints.resize(constrSize + 1);
                constraints[constrSize] = _BBO[i];
            }
        }
    }

    return constraints;
}

NOMAD::ArrayOfDouble NOMAD::BBOutput::getExtraOutputs(const NOMAD::BBOutputTypeList &bbOutputType) const
{
    NOMAD::ArrayOfDouble extraOs;

    if (_evalOk && !bbOutputType.empty() && checkSizeMatch(bbOutputType))
    {
        for (size_t i = 0; i < _BBO.size(); i++)
        {
            if (bbOutputType[i].isExtraOutput())
            {
                size_t s = extraOs.size();
                extraOs.resize(s + 1);
                extraOs[s] = _BBO[i];
            }
        }
    }
    return extraOs;
}

const NOMAD::ArrayOfDouble & NOMAD::BBOutput::getBBOAsArrayOfDouble() const
{
    return _BBO;
}


// Helper function.
// Verify that the given output type list has the same size as the raw output.
// Show an error, and return false, if this is not the case.
bool NOMAD::BBOutput::checkSizeMatch(const NOMAD::BBOutputTypeList &bbOutputType) const
{
    bool ret = true;

    if (bbOutputType.size() != _BBO.size())
    {
        /*
        std::string err = "Error: Parameter BB_OUTPUT_TYPE has " + NOMAD::itos(bbOutputType.size());
        err += " type";
        if (bbOutputType.size() > 1)
        {
            err += "s";
        }
        err += ", but raw output has " + NOMAD::itos(array.size());
        err += " field";
        if (array.size() > 1)
        {
            err += "s";
        }
        err += ":\n";
        err += _rawBBO;
        std::cerr << err << std::endl;
        */
        ret = false;
    }

    return ret;
}


std::ostream& NOMAD::operator<<(std::ostream& os, const NOMAD::BBOutput &bbo)
{
    os << NOMAD::BBOutput::bboStart << " " << bbo.getBBO();
    os << " " << NOMAD::BBOutput::bboEnd;
    return os;
}


std::istream& NOMAD::operator>>(std::istream& is, NOMAD::BBOutput &bbo)
{
    std::string s, bboString;
    bool firstField = true;

    is >> s;

    if (NOMAD::BBOutput::bboStart != s)
    {
        is.setstate(std::ios::failbit);
        std::string err = "Expecting \"" + NOMAD::BBOutput::bboStart + "\", got \"" + s + "\"";
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }

    while (is >> s && NOMAD::BBOutput::bboEnd != s)
    {
        if (firstField)
        {
            firstField = false;
        }
        else
        {
            bboString += " ";
        }
        bboString += s;
    }

    if (NOMAD::BBOutput::bboEnd != s)
    {
        is.setstate(std::ios::failbit);
        std::string err = "Expecting \"" + NOMAD::BBOutput::bboEnd + "\", got \"" + s + "\"";
        throw NOMAD::Exception(__FILE__, __LINE__, err);
    }

    bbo.setBBO(bboString);

    return is;
}





