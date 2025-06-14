/**
 * \file   StatsInfo.hpp
 * \brief  Class for Stats info and display
 * \author Viviane Rochon Montplaisir, Christophe Tribes
 * \date   February 2018
 */

#ifndef __NOMAD_4_5_STATSINFO__
#define __NOMAD_4_5_STATSINFO__

#include <memory>   // For unique_ptr
#include <vector>
#include "../Math/ArrayOfDouble.hpp"
#include "../Math/Point.hpp"
#include "../Util/ArrayOfString.hpp"

#include "../nomad_platform.hpp"
#include "../nomad_nsbegin.hpp"


/// Types for DISPLAY_STATS and STATS_FILE parameters
/**
 \warning Do not modify the order
 */
enum class DisplayStatsType
{
    DS_OBJ        ,    ///< Objective (f) value
    //   (keep in first position)
    DS_CONS_H     ,    ///< Infeasibility (h) value
    DS_H_MAX      ,    ///< Max infeasibility (h) acceptable at the time of eval
    //DS_SMOOTH_OBJ ,    ///< Smoothed objective value (f~)
    //DS_SIM_BBE    ,    ///< Number of simulated bb evaluations
    DS_BBE        ,    ///< Number of bb evaluations
    DS_FEAS_BBE   ,    ///< Number of feasible bb evaluations
    DS_INF_BBE    ,    ///< Number of infeasible bb evaluations
    DS_ALGO_BBE   ,    ///< Number of bb evaluations for a single algo run
    DS_BLK_EVA    ,    ///< Number of block evaluation calls
    DS_BLK_SIZE   ,    ///< Number of EvalPoints in the block
    DS_LAP        ,    ///< Number of evaluations since this lap started
    DS_MODEL_EVAL ,    ///< Number of model evaluations since this model started
    DS_TOTAL_MODEL_EVAL, ///< Total number of model evaluations
    DS_BBO        ,    ///< All blackbox outputs
    DS_EVAL       ,    ///< Number of evaluations
    DS_REL_SUCC   ,    ///< Number of relative success evaluations
    DS_PHASE_ONE_SUCC, ///< Number of success evaluations in PhaseOne
    DS_CACHE_HITS ,    ///< Number of cache hits
    DS_CACHE_SIZE ,    ///< Number of points in cache
    DS_ITER_NUM   ,    ///< Iteration number
    DS_TIME       ,    ///< Wall-clock time
    DS_MESH_INDEX ,    ///< Mesh index
    DS_MESH_SIZE  ,    ///< Mesh size parameter Delta^m_k
    DS_DELTA_M    ,    ///< Same as \c DS_MESH_SIZE
    DS_FRAME_SIZE ,    ///< Frame size parameter Delta^f_k
    DS_DELTA_F    ,    ///< Same as \c DS_FRAME_SIZE
    DS_FRAME_CENTER ,  ///< Frame center: Point that was used as center to generate this point
    DS_DIRECTION  ,    ///< Direction that generated this point
    DS_SURROGATE_EVAL , ///< Number of static surrogate evaluations
    DS_SOL        ,    ///< Solution vector
    DS_THREAD_ALGO,    ///< Thread number for the algorithm
    DS_THREAD_NUM ,    ///< Thread number in which this evaluation was done
    DS_GEN_STEP   ,    ///< Name of the step in which this point was generated
    DS_SUCCESS_TYPE,    ///< Success type for this evaluation
    //DS_VAR        ,    ///< One variable
    //DS_STAT_SUM   ,    ///< Stat sum
    //DS_STAT_AVG   ,    ///< Stat avg
    DS_TAG,             ///< Tag
    DS_USER         ,  ///< User-defined string
    DS_UNDEFINED       ///< Undefined value
    //   (keep in last position)
};

typedef ArrayOfString DisplayStatsTypeList;


/// Information for stats format (parameters DISPLAY_STATS and STATS_FILE).
/**
 Also holds information about stats file.
 */
class DLL_UTIL_API StatsInfo
{
private:
    // Stats infos
    bool            _failedEval;
    Double          _obj;
    Double          _consH;
    Double          _hMax;
    size_t          _bbe;
    size_t          _feasBBE;
    size_t          _infBBE;
    size_t          _nbRelativeSuccess;
    size_t          _PhaseOneSuccess;
    size_t          _algoBBE;
    size_t          _blkEva;
    size_t          _blkSize;
    std::string     _bbo;
    size_t          _eval;
    size_t          _cacheHits;
    size_t          _cacheSize;
    size_t          _iterNum;
    size_t          _time;
    ArrayOfDouble   _meshIndex;
    ArrayOfDouble   _meshSize;
    ArrayOfDouble   _frameSize;
    Point           _frameCenter;
    Direction       _direction;
    size_t          _lap;
    size_t          _modelEval;
    size_t          _totalModelEval;
    Point           _sol;
    size_t          _surrogateEval;
    int             _threadAlgoNum;
    int             _threadNum;
    bool            _relativeSuccess;   ///> Used for printing star, or when DISPLAY_ALL_EVAL is false.
    size_t          _tag;         
    std::string     _comment;   ///> General comment, ex. Algorithm from where this point was generated.
    std::string     _genStep;   ///> Step in which this point was generated
    SuccessType     _success;   ///> Success type for this evaluation


public:
    /*---------------*/
    /* Class Methods */
    /*---------------*/

    /// Constructor
    explicit StatsInfo();

    // Destructor is not implemented, so the compiler creates all default functions.
    //virtual ~StatsInfo() {}

public:

    // Get/Set
    void setFailEval (bool fail)                    { _failedEval = fail;}
    void setObj(const Double& obj)                  { _obj = obj; }
    void setConsH(const Double consH)               { _consH = consH; }
    void setHMax(const Double hMax)                 { _hMax = hMax; }
    void setBBE(const size_t bbe)                   { _bbe = bbe; }
    void setFeasBBE(const size_t feasBBE)           { _feasBBE = feasBBE; }
    void setInfBBE(const size_t infBBE)             { _infBBE = infBBE; }
    void setAlgoBBE(const size_t bbe)               { _algoBBE = bbe; }
    void setBlkEva(const size_t blkEva)             { _blkEva = blkEva; }
    void setBlkSize(const size_t blkSize)           { _blkSize = blkSize; }
    void setBBO(const std::string& bbo)             { _bbo = bbo; }
    void setEval(const size_t eval)                 { _eval = eval; }
    void setNbRelativeSuccess(const size_t nbRelSuccess)   { _nbRelativeSuccess = nbRelSuccess; }
    void setPhaseOneSuccess(const size_t phaseOneSuccess)   { _PhaseOneSuccess = phaseOneSuccess; }
    void setCacheHits(const size_t cacheHits)       { _cacheHits = cacheHits; }
    void setCacheSize(const size_t cacheSize)       { _cacheSize = cacheSize; }
    void setIterNum(const size_t iterNum)           { _iterNum = iterNum; }
    void setTime(const size_t time)                 { _time = time; }
    void setMeshIndex(const ArrayOfDouble& meshIndex) { _meshIndex = meshIndex; }
    void setMeshSize(const ArrayOfDouble& meshSize)   { _meshSize = meshSize; }
    void setFrameSize(const ArrayOfDouble& frameSize) { _frameSize = frameSize; }
    void setFrameCenter(const Point& frameCenter)    { _frameCenter = frameCenter; }
    void setDirection(const Direction& direction)    { _direction = direction; }
    void setLap(const size_t lap)                   { _lap = lap; }
    void setModelEval(const size_t modelEval)       { _modelEval = modelEval; }
    void setTotalModelEval(const size_t totalModelEval) { _totalModelEval = totalModelEval; }
    void setSol(const Point& sol)                    { _sol = sol; }
    void setSurrogateEval(const size_t surrogateEval) { _surrogateEval = surrogateEval; }
    void setThreadAlgo(const int threadAlgoNum)     { _threadAlgoNum = threadAlgoNum; }
    void setThreadNum(const int threadNum)          { _threadNum = threadNum; }
    void setRelativeSuccess(bool relativeSuccess)   { _relativeSuccess = relativeSuccess; }
    void setComment(const std::string& comment)     { _comment = comment; }
    void setTag(const size_t tag) { _tag = tag; }
    void setGenStep(const std::string& genStep)     { _genStep = genStep; }
    void setSuccessType(const SuccessType& success) { _success = success; }

    // Should these stats be printed even if DISPLAY_ALL_EVAL is false
    bool alwaysDisplay(const bool displayFailed,
                       const bool displayInfeasible,
                       const bool displayUnsuccessful,
                       const bool forStatsFile) const;

    /// Display header
    std::string displayHeader(const DisplayStatsTypeList& format,
                              const ArrayOfDouble & solFormat = ArrayOfDouble(),
                              const size_t objWidth = 0) const;

    /**
     Display with an empty format will output BBE OBJ
     */
    std::string display(const DisplayStatsTypeList& format,
                        const ArrayOfDouble & solFormat = ArrayOfDouble(),
                        const size_t objWidth = 0,
                        const size_t hWidth = 0,
                        const bool starSuccess = false,
                        const bool appendComment = true) const;

    // Convert a string in a DisplayStatsType.
    static DisplayStatsType stringToDisplayStatsType(const std::string& inputString,
                                                     std::string& format);
    static std::string displayStatsTypeToString(const DisplayStatsType& displayStatsType);

};

typedef std::unique_ptr<StatsInfo> StatsInfoUPtr;


inline std::ostream& operator<< (std::ostream& os, const DisplayStatsType& displayStatsType)
{
    return os << StatsInfo::displayStatsTypeToString(displayStatsType);
}


#include "../nomad_nsend.hpp"

#endif // __NOMAD_4_5_STATSINFO__
