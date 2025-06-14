
#include "../../Algos/QuadModelSLD/QuadModelSldEvaluator.hpp"
#include "../../Output/OutputQueue.hpp"

// Destructor
NOMAD::QuadModelSldEvaluator::~QuadModelSldEvaluator()
{
    
    if ( _model_ready )
    {
        for ( int i = 0 ; i < _m ; ++i )
            if ( _alpha[i] )
                delete [] _alpha[i];
        delete [] _alpha;
        delete [] _x;
    }
    
}


void NOMAD::QuadModelSldEvaluator::init()
{
    _displayLevel = (std::string::npos != _modelDisplay.find("X"))
                        ? NOMAD::OutputLevel::LEVEL_INFO
                        : NOMAD::OutputLevel::LEVEL_DEBUGDEBUG;

    if (nullptr == _model)
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Evaluator: a model is required (nullptr)");
    }
    
    if ( _model_ready )
    {
        const auto bbot = _evalParams->getAttributeValue<NOMAD::BBOutputTypeList>("BB_OUTPUT_TYPE");

        _m = static_cast<int>(NOMAD::getNbConstraints(bbot) + 1);   // Single objective
        
        int i , j , k , k2 , nalpha = (_n+1)*(_n+2)/2 , nfree = _model->get_nfree();
        NOMAD::Point ** model_alpha = _model->get_alpha();
        
        _x     = new double   [_n];
        _alpha = new double * [_m];
        
        for ( int io = 0 ; io < _m ; ++io )
        {
            _alpha[io] = NULL;
            if ( model_alpha[io] )
            {
                _alpha[io] = new double[nalpha];
                _alpha[io][0] = (*model_alpha[io])[0].todouble();
                
                for ( i = 1 ; i < nalpha ; ++i )
                    _alpha[io][i] = 0.0;
                
                k = 0;
                
                for ( i = 0 ; i < _n ; ++i )
                {
                    if ( !_model->variable_is_fixed(i) )
                    {
                        ++k;
                        _alpha[io][i+1   ] = (*model_alpha[io])[k      ].todouble();
                        _alpha[io][i+1+_n] = (*model_alpha[io])[k+nfree].todouble();
                    }
                }
                
                k += nfree;
                k2 = 2*_n;
                
                for ( i = 0 ; i < _nm1 ; ++i )
                {
                    if ( !_model->variable_is_fixed(i) )
                    {
                        for ( j = i+1 ; j < _n ; ++j )
                        {
                            ++k2;
                            if ( !_model->variable_is_fixed(j) )
                                _alpha[io][k2] = (*model_alpha[io])[++k].todouble();
                        }
                    }
                    else
                        for ( j = i+1 ; j < _n ; ++j )
                            ++k2;
                }
            }
        }
    }
}


//*------------------------------------------------------*/
//*       evaluate the quad model at a given point       */
//*------------------------------------------------------*/
bool NOMAD::QuadModelSldEvaluator::eval_x(NOMAD::EvalPoint &x,
                                          const NOMAD::Double& hMax,
                                          bool &countEval) const
{
    //Important remark about fixed variables
    //
    // FixedVariables are not needed.
    //
    // 1- The evaluator and the component requesting quad model evaluations are on the same local space.
    // 2- The values of variables detected as fixed when building the training set are visible.
    // 2- Quad model identifies fixed variables from the training set when building models.
    // 3- If the original definition of the problem has fixed variable, they are not 'seen' by models (training set, quad model evaluator and optimization).

    
    countEval = false;
    
    if (x.size() !=_n || !x.isComplete())
    {
        throw NOMAD::Exception(__FILE__, __LINE__, "Pb with evaluation point " + x.display());
    }
    if ( !_model_ready )
        return false;
    
    std::string s = "X=" + x.display();
    NOMAD::OutputQueue::Add(s, _displayLevel);
    
    // Reset point outputs
    // By default, set everything to -1
    // Note: Currently NOMAD cannot set a bbo value by index, so we have to
    // work around by constructing a suitable string.
    // Note: Why set some default values on bbo?
    NOMAD::ArrayOfString defbbo(_m, "-1");
    x.setBBO(defbbo.display(), _bbOutputTypeList, _evalType);
    
    // ------------------------- //
    //   Output Prediction    //
    // ------------------------- //
    NOMAD::OutputQueue::Add("Predict with quadratic (SLD) formulation... ", _displayLevel);
    
    
    int    i , j , k =0;
    double z , * alpha , * p;
    
    for ( i = 0 ; i < _n ; ++i )
    {
        _x[i] = x[i].todouble() / 1000.0;
    }
        
    
    // ====================================== //
    // Output display                    //
    // ====================================== //
    std::string sObj = "F = ";
    std::string sCons = "C = [ ";
    
    std::string bbo;
    
    for ( int oi = 0 ; oi < _m ; ++oi )
    {
        z = 0.0;
        alpha = _alpha[oi];
        
        if ( alpha )
        {
            
            z = alpha[0];
            p = _x;
            
            for ( k = 1 ; k <= _n ; ++k , ++p )
                z += *p * ( alpha[k] + 0.5 * alpha[k+_n] * *p );
            
            k += _n-1;
            
            for ( i = 0 ; i < _nm1 ; ++i )
                for ( j = i+1 ; j < _n ; ++j )
                    z += alpha[++k] * _x[i] * _x[j];
            
            // SLD special rounding to 8 decimals in order to improve numerical stability:
            // {
            //   long double prec = 1e-8;
            //   z = ( z < 0.0 ?
            //              -floor(-z/prec + .5) :
            //            floor( z/prec + .5)   ) * prec;
            // }
            
            
            
        }
        std::stringstream ss;
        ss.precision(NOMAD::DISPLAY_PRECISION_FULL);
        ss << z;
        
        bbo+= ss.str() + " " ;
        if (_bbOutputTypeList[oi] != NOMAD::BBOutputType::OBJ)
            //sCons += std::to_string(z) + " ";
            sCons += ss.str() + " ";
        else
            //sObj  += std::to_string(z) + " ";
            sObj  += ss.str() + " ";
    }

    
    s = sObj + ((_m-1>0 ) ? sCons+" ]":"") ;
    NOMAD::OutputQueue::Add(s, _displayLevel);
    NOMAD::OutputQueue::Add("ok", _displayLevel);
    
    x.setBBO(bbo, _bbOutputTypeList, _evalType);
    
    countEval = true;
    return true;

}


