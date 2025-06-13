
#include "../../Algos/Mads/QR2NPollMethod.hpp"
#include "../../Math/Direction.hpp"
#include "../../Math/MatrixUtils.hpp"

void NOMAD::QR2NPollMethod::init()
{
    setStepType(NOMAD::StepType::POLL_METHOD_QR_2N);
    verifyParentNotNull();

}

// Generate poll directions
void NOMAD::QR2NPollMethod::generateUnitPollDirections(std::list<NOMAD::Direction> &directions, const size_t n) const
{
    directions.clear();

    NOMAD::Direction dirUnit(n, 0.0);
    NOMAD::Direction::computeDirOnUnitSphere(dirUnit, _rng);
    
    OUTPUT_DEBUG_START
    AddOutputDebug("Unit sphere direction: " + dirUnit.display());
    OUTPUT_DEBUG_END
    
    while (dirUnit[0] == 0)
    {
        NOMAD::Direction::computeDirOnUnitSphere(dirUnit, _rng);
    }
    
    // Matrix M
    auto M = new double*[n];
    for (size_t i = 0; i < n; ++i)
    {
        M[i] = new double [n];
        M[i][0] = dirUnit[i].todouble();
        for (size_t j = 1; j < n; ++j)
        {
            M[i][j] = (i == j)? 1.0:0.0;
        }
    }
    OUTPUT_DEBUG_START
    AddOutputDebug("M matrix for QR:");
    for (size_t i = 0; i < n; ++i)
    {
        NOMAD::ArrayOfDouble aod(n);
        for (size_t j = 0; j < n; ++j)
        {
            aod[j]=M[i][j];
        }
        AddOutputDebug(aod.display());
        
    }
    OUTPUT_DEBUG_END
    
    // Matrices Q and R
    auto Q = new double*[n];
    auto R = new double*[n];
    for (size_t i = 0; i < n; ++i)
    {
        Q[i] = new double [n];
        R[i] = new double [n];
    }
    
    std::string error_msg;
    bool success = NOMAD::qr_factorization (error_msg,M,Q,R,static_cast<int>(n),static_cast<int>(n));
    
    if ( !success || !error_msg.empty())
    {
        OUTPUT_INFO_START
        AddOutputInfo("QR decomposition for QR 2N poll method has failed");
        OUTPUT_INFO_END
    }
    
    OUTPUT_DEBUG_START
    AddOutputDebug("Direction after QR decomposition: ");
    OUTPUT_DEBUG_END
    
    // Ordering D_k alternates Qk and -Qk instead of [Q_k -Q_k]
    NOMAD::Direction dir(n);
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            dir[j] = Q[j][i];
        }
        OUTPUT_DEBUG_START
        AddOutputDebug(dir.display());
        OUTPUT_DEBUG_END
        
        directions.push_back(dir);
        directions.push_back(-dir);
    }
    
    OUTPUT_DEBUG_START
    NOMAD::OutputQueue::Flush();
    OUTPUT_DEBUG_END
    
    // Delete M, Q and R:
    for ( size_t i = 0 ; i < n ; ++i )
    {
        delete [] M[i];
        delete [] Q[i];
        delete [] R[i];
    }
    delete [] Q;
    delete [] R;
    delete [] M;
    
}
