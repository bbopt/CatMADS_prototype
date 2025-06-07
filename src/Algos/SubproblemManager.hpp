
#ifndef __NOMAD_4_5_SUBPROBLEMMANAGER__
#define __NOMAD_4_5_SUBPROBLEMMANAGER__

#ifdef _OPENMP
#include <omp.h>
#endif // _OPENMP
#include "../Algos/Algorithm.hpp"
#include "../Algos/Subproblem.hpp"

#include "../nomad_nsbegin.hpp"


/// Class to associate Algorithms with Subproblems
/**
 Ease the passage between sub-dimension and full dimension. Algorithm works in
 a sub dimentsion and does not know the full dimension.
 */
class SubproblemManager
{
private:
    std::map<const Algorithm*, const Subproblem> _map;

#ifdef _OPENMP
    DLL_ALGO_API static omp_lock_t _mapLock;
#endif // _OPENMP

    DLL_ALGO_API static std::unique_ptr<SubproblemManager> _single; ///< The singleton

    bool updateSubproblemFixedVariable;
    
    /// Constructor
    explicit SubproblemManager()
    {
        init();
    }

    /// Helper for constructor
    void init();

    /// Helper for destructor
    void destroy();

public:

    static const std::unique_ptr<SubproblemManager> & getInstance()
    {
        if (_single == nullptr)
        {
            _single = std::unique_ptr<NOMAD::SubproblemManager>(new SubproblemManager()) ;
        }
        return _single;
    }

    /// Destructor
    virtual ~SubproblemManager()
    {
        destroy();
    }

    /// Copy constructor not available
    SubproblemManager ( SubproblemManager const & ) = delete;

    /// Operator= not available
    SubproblemManager & operator= ( SubproblemManager const & ) = delete;

    const Subproblem& getSubproblem(const Step* step);

    const Point& getSubFixedVariable(const Step* step);
    
    void addSubproblem(const Algorithm* algo, const Subproblem& subproblem);

    void removeSubproblem(const Algorithm* algo);
    void reset();

};

#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_SUBPROBLEMMANAGER__
