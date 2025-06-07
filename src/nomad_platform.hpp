#ifdef _MSC_VER
#define NOMAD_UNUSED(x)
#define NOMAD_PRETTY_FUNCTION __FUNCSIG__
#else
#define NOMAD_UNUSED(x) x __attribute__((unused))
#define NOMAD_PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

#ifdef _MSC_VER
#pragma warning(disable:4275)
#pragma warning(disable:4251)
#endif

#if defined(_WIN32) && !defined(NOMAD_STATIC_BUILD)
#ifdef DLL_UTIL_EXPORTS
# define DLL_UTIL_API __declspec(dllexport)
#else
# define DLL_UTIL_API __declspec(dllimport)
#endif
#ifdef DLL_EVAL_EXPORTS
# define DLL_EVAL_API __declspec(dllexport)
#else
# define DLL_EVAL_API __declspec(dllimport)
#endif
#ifdef DLL_ALGO_EXPORTS
# define DLL_ALGO_API __declspec(dllexport)
#else
# define DLL_ALGO_API __declspec(dllimport)
#endif
#else
#define DLL_UTIL_API
#define DLL_EVAL_API
#define DLL_ALGO_API
#endif
