# ----------------------------------------------------------------------------
#  Root CMake file for Nomad
# ----------------------------------------------------------------------------

# cmake version : the latest one
cmake_minimum_required(VERSION 3.15...3.15)

# Warning. This must be consistent with src/nomad_version.hpp
set(NOMAD_VERSION_MAJOR 4)
set(NOMAD_VERSION_MINOR 5)
set(NOMAD_VERSION_PATCH dev4)
# set(NOMAD_VERSION_PATCH b3)  # The patch version b3 is for develop branch of the private repo. For the public repo, the real patch version is used. THE VERSION ENDING WITH "bx" IS TO COMPLY WITH TestPyPi.
set(NOMAD_VERSION ${NOMAD_VERSION_MAJOR}.${NOMAD_VERSION_MINOR}.${NOMAD_VERSION_PATCH})

# name of the project
# Need to update when version changes
project (Nomad VERSION ${NOMAD_VERSION_MAJOR}.${NOMAD_VERSION_MINOR})

# Disable soname (used for Julia interface).
option(SETSONAME "Option to build Nomad libraries with SONAME" ON)
if(SETSONAME MATCHES OFF)
   message(STATUS "Dynamic libraries without soname")
   set(NO_SONAME True)
else()
   set(NO_SONAME False)
endif()

# Nomad src template (copy of template defined in src/nomad_version.hpp, used by jNomad)
# Need to update when version changes
set(NOMAD_SRC_TEMPLATE NOMAD_${NOMAD_VERSION_MAJOR}_${NOMAD_VERSION_MINOR})

# use standard compilers parameters for c++17
SET(CMAKE_CXX_STANDARD 17 )
SET(CMAKE_CXX_STANDARD_REQUIRED ON )

# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "FATAL: In-source builds are not allowed.
    You should create a separate directory for build files.
    ")
endif()

#check compiler version
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # require at least gcc 8
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
        message(FATAL_ERROR "GCC version < 8 has not been tested for Nomad")
    endif()
elseif (("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    # require at least clang 5
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
        message(FATAL_ERROR "Clang version has not been tested for Nomad")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # require at least 15.8 (MSVC 2017) for C++17 support
    if (MSVC_TOOLSET_VERSION VERSION_LESS 141)
        message(FATAL_ERROR "MSVC version ${CMAKE_CXX_COMPILER_VERSION} has not been tested for Nomad")
    endif()
else()
    message(WARNING "You are using an unsupported compiler. Compilation has only been tested with Clang, GCC, and MSVC.")
endif()


# Load some modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# special flag required
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# Message for starting configuration
#
message(CHECK_START "Configuring custom options")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

#
# Modify the build type if not specified on the command
#
if (CMAKE_CONFIGURATION_TYPES)
    message(STATUS "  Multi-configuration generator detected. Use the --config option during build to specify the build type (Release, Debug, ...).")
else()
   if (NOT CMAKE_BUILD_TYPE)
      set(CMAKE_BUILD_TYPE "Release" CACHE PATH "..." FORCE)
      message(STATUS "  Build type not set explicitly. Default is set to Release. To force build type selection, use --DCMAKE_BUILD_TYPE=xxx, options are Debug Release RelWithDebInfo MinSizeRel.")
   else()
      message(STATUS "  Build type is ${CMAKE_BUILD_TYPE}")
   endif()
endif()



#
# Modify the install prefix if not specified on the command
#
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR} CACHE PATH "..." FORCE)
endif()
message(STATUS "  Installation prefix set to ${CMAKE_INSTALL_PREFIX}")


#
# Choose to build with time stats enabled
#
option(TIME_STATS "Option to build with time stats enabled" OFF)
if(TIME_STATS MATCHES ON)
   message(CHECK_START "  Enabling time stats for build")
   add_compile_definitions(TIME_STATS)
endif()

#
# Choose to build with unit tests or not
# Other tests are performed on examples based on the BUILD_TESTS flag
#
option(BUILD_TESTS "Option to build the unit tests" OFF)
if(BUILD_TESTS MATCHES ON)
   message(CHECK_START "  Configuring unit tests build")

   if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/internal/unit_tests)
        message(FATAL_ERROR "The NOMAD 4 unit tests are not available. The unit tests are available only when using the private Nomad 4 repository")
   endif()

   include(FetchContent)

   # Download Googletest
   FetchContent_Declare(
       googletest
       GIT_REPOSITORY https://github.com/google/googletest.git
       GIT_TAG        v1.14.0
       )

   # Make the library available
   FetchContent_MakeAvailable(googletest)

   enable_testing()

   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/internal/unit_tests)
   message(CHECK_PASS " done")
else()
   message(STATUS "  Unit tests NOT built")
endif()

#
# Test openMP package
#
option(TEST_OPENMP "Option to compile Nomad with OpenMP" ON)
if(TEST_OPENMP MATCHES ON)
   find_package(OpenMP QUIET)
   if(OpenMP_FOUND)
      message(STATUS "  Test OpenMP for parallel functionalities  -- found ")
   else()
      message(STATUS "  OpenMP not found. Parallel functionalities NOT available")
   endif()
else()
    message(STATUS "  Compiling Nomad without OpenMP")
endif()

#
# Choose to build the examples/tests using batch and library mode
#
#option(BUILD_EXAMPLES "Option to build examples (library and batch)" ON)
#if(BUILD_EXAMPLES MATCHES ON)
#   message(CHECK_START "  Configuring build for library and batch mode examples")

#   enable_testing()

#   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
#   message(CHECK_PASS " done")
#else()
#   message(STATUS "  Examples NOT built")
#endif()

option(BUILD_INTEGRATION_TESTS "Option to build integration tests" OFF)
if(BUILD_INTEGRATION_TESTS MATCHES ON)
   message(CHECK_START "  Configuring build for integration tests")

   if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/internal/integration_tests)
        message(FATAL_ERROR "The NOMAD 4 integration tests are not available. The integration tests are available only when using the private Nomad 4 repository")
   endif()

   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/internal/integration_tests)
   message(CHECK_PASS " done")
else()
   message(STATUS "  Integration tests NOT built")
endif()

#
# Choose to build the C interface
#
option(BUILD_INTERFACE_C "Option to build C interface to Nomad" OFF)
if(BUILD_INTERFACE_C MATCHES ON)
   message(CHECK_START "  Configuring build for C interface")
   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/interfaces/CInterface)
   message(CHECK_PASS " done")
else()
   message(STATUS "  C interface to Nomad NOT built")
endif()

#
# Choose to build the Python interface
#
option(BUILD_INTERFACE_PYTHON "Option to build Python interface to Nomad" OFF)
if(BUILD_INTERFACE_PYTHON MATCHES ON)
       set(Python3_FIND_VIRTUALENV "First") # Using virtualenv to have cython and wheel is easy
       find_package(Python 3.6 QUIET REQUIRED)
       message(CHECK_START "  Configuring build for Python interface (Python ${Python_VERSION})")
       add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/interfaces/PyNomad)
       message(CHECK_PASS " done")
else()
   message(STATUS "  Python interface to Nomad NOT built")
endif()

#
# Choose to build the Matlab interface
#
option(BUILD_INTERFACE_MATLAB "Option to build Matlab interface to Nomad" OFF)
if(BUILD_INTERFACE_MATLAB MATCHES ON)
   if(OpenMP_FOUND)
       message(STATUS "  Warning: Cannot build Matlab interface with OpenMP enabled")
   else()
       find_package(Matlab QUIET REQUIRED COMPONENTS MEX_COMPILER)
       if(MATLAB_FOUND)
          message(CHECK_START "  Configuring build for Matlab interface")
          add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/interfaces/Matlab_MEX)
       else()
          message(STATUS "  Matlab no found... interface not built")
       endif()
   endif()
else()
   message(STATUS "  Matlab interface to Nomad NOT built")
endif()

#
# Choose to build the Java interface
#
option(BUILD_INTERFACE_JAVA "Option to build Java Swig interface to Nomad" OFF)
if(BUILD_INTERFACE_JAVA MATCHES ON)

   # The following lines maybe required. See issue #83 in bbopt/nomad
   #set(JAVA_INCLUDE_PATH "$ENV{JAVA_HOME}/Contents/Home/include")
   #set(JAVA_INCLUDE_PATH2 "$ENV{JAVA_HOME}/Contents/Home/include")
   #set(JAVA_AWT_INCLUDE_PATH "$ENV{JAVA_HOME}/Contents/Home/include")
   set(JAVA_JVM_LIBRARY NotNeeded)

   find_package(Java REQUIRED COMPONENTS Runtime Development)
   find_package(SWIG 4.0 REQUIRED COMPONENTS java)
   find_package(JNI REQUIRED)
   if(SWIG_FOUND)
       if(NOT SWIG_java_FOUND)
           message(WARNING "   SWIG java bindings cannot be generated")
       else()
           message(CHECK_START "  Configuring build for Java Swig interface")
           add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/interfaces/jNomad)
       endif()
   else()
       message(STATUS "  Swig not found... java interface not built")
   endif()

else()
   message(STATUS "  Java Swig interface to Nomad NOT built")
endif()


option(USE_IBEX "Option to use IBEX" OFF)
if(USE_IBEX MATCHES ON)
   message(CHECK_START "  Enabling Ibex use.")
   if(NOT DEFINED ENV{IBEX_ROOT})
        message(FATAL_ERROR "FATAL: Ibex root environment variable IBEX_ROOT is not defined")
   endif()
   If (NOT EXISTS "$ENV{IBEX_ROOT}")
        message(FATAL_ERROR "FATAL: Ibex root dir $ENV{IBEX_ROOT} is not present")
   endif()
   message(STATUS "  Ibex root: $ENV{IBEX_ROOT}")
   set(USE_IBEX ON)
else()
   message(STATUS "  Use of Ibex for projecting NOT enabled")
endif()


#
# Use sgtelib
#
option(USE_SGTELIB "Option to use the Sgtelib library" ON)
if(USE_SGTELIB MATCHES ON)
  message(CHECK_START "  Configuring for use of Sgtelib library")

  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ext/sgtelib)
     message(CHECK_FAIL "  failed. Directory ${CMAKE_CURRENT_SOURCE_DIR}/ext/sgtelib not found. Let's configure without Sgtelib library")
     set(USE_SGTELIB OFF)
  else()
     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext)
     add_compile_definitions(USE_SGTELIB)
     list(APPEND EXTRA_LIBS sgtelib)
     message(CHECK_PASS "   Sgtelib found. done")
  endif()
else()
   message(STATUS "  Sgtelib library will NOT be used\n")
endif()

#
# Custom options final message
#
list(REMOVE_ITEM CMAKE_MESSAGE_INDENT "  ")
message(CHECK_PASS " done")


#
# Add nomad app directory for building
#
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)


#
# Add CatMADS prototype implementation
#
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/CatMADS)
