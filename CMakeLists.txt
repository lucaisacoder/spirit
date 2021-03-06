######### CMake Version #####################
cmake_minimum_required(VERSION 3.1)
### We need at least C++11
set (CMAKE_CXX_STANDARD 11)
### Distinction between Clang and AppleClang
cmake_policy(SET CMP0025 NEW)
#############################################



######### Build Flags #######################
### CMake Verbosity
SET( SPIRIT_PRINT_SOURCES     OFF  CACHE BOOL "Print Spirit Headers and Sources from CMake." )
### These decide which projects are built
SET( SPIRIT_BUILD_FOR_JS      OFF  CACHE BOOL "Build the JavaScript library." )
SET( SPIRIT_BUILD_FOR_JULIA   OFF  CACHE BOOL "Build the shared library for Julia." )
SET( SPIRIT_BUILD_FOR_PYTHON  ON   CACHE BOOL "Build the shared library for Python." )
SET( SPIRIT_BUILD_FOR_CXX     ON   CACHE BOOL "Build the static library for C++ applications" )
### Feature switches for Spirit
SET( SPIRIT_ENABLE_PINNING    OFF  CACHE BOOL "Enable pinning individual or rows of spins." )
SET( SPIRIT_ENABLE_DEFECTS    OFF  CACHE BOOL "Enable defects and disorder in the lattice." )
### Options for Spirit
SET( SPIRIT_BUILD_TEST        ON   CACHE BOOL "Build unit tests for the Spirit library." )
SET( SPIRIT_TEST_COVERAGE     OFF  CACHE BOOL "Build in debug mode with special flags for coverage checks." )
SET( SPIRIT_USE_CUDA          OFF  CACHE BOOL "Use CUDA to speed up certain parts of the code." )
SET( SPIRIT_USE_OPENMP        OFF  CACHE BOOL "Use OpenMP to speed up certain parts of the code." )
SET( SPIRIT_USE_THREADS       OFF  CACHE BOOL "Use std threads to speed up certain parts of the code." )
SET( SPIRIT_USE_FFTW          ON   CACHE BOOL "If available, use the FFTW library instead of kissFFT." )
### Set the scalar type used in the Spirit library
SET( SPIRIT_SCALAR_TYPE "double" CACHE STRING "The scalar type to be used in the Spirit library." )
### Set the compute capability for CUDA compilation
SET( SPIRIT_CUDA_ARCH   "sm_60"  CACHE STRING "The CUDA compute architecture to use in case of a CUDA build." )
#############################################
### CMake Verbosity
option( SPIRIT_PRINT_SOURCES  "Print Headers and Sources from Cmake."                         OFF )
### Decide CXX UI
option( SPIRIT_UI_CXX_USE_QT  "Build the QT User Interface instead of console version."       ON  )
### Bundle option
option( SPIRIT_BUNDLE_APP     "On installation, bundle the executable with its dependencies." OFF )
### Option for building on the IFF cluster
option( SPIRIT_USER_PATHS_IFF "Use the compiler and library paths etc. for the IFF Cluster."  OFF )
#############################################



#############################################
### Set a default build type in case none is passed
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS ">> Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
elseif(CMAKE_BUILD_TYPE)
    message(STATUS ">> Did not need to set build type, using: ${CMAKE_BUILD_TYPE}")
else()
    message(STATUS ">> Did not need to set build type. Configuration types: ${CMAKE_CONFIGURATION_TYPES}")
endif()
#############################################



#############################################
### Depending on compiler versions it may be necessary to specify
### the compiler. Either pass them in via command-line or use
### the CUDA_TOOLKIT_ROOT_DIR variable.
if( SPIRIT_USE_CUDA )
    ### Deactivate OpenMP
    set( SPIRIT_USE_OPENMP          OFF )
    ### Set cuda toolkit path
    if( NOT CUDA_TOOLKIT_ROOT_DIR )
        if ( APPLE OR UNIX )
            set( CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda )
            # set( CUDA_TOOLKIT_ROOT_DIR /opt/cuda )
        elseif (WIN32)
            set( CUDA_TOOLKIT_ROOT_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v8.0/" )
            MESSAGE( WARNING ">> We are on Windows... CUDA_TOOLKIT_ROOT_DIR may need to be passed to cmake..." )
        endif()
    endif()
    ### Set compilers
    if( APPLE OR UNIX )
        if( DEFINED CUDA_TOOLKIT_ROOT_DIR )
            MESSAGE( STATUS ">> CUDA toolkit root dir: ${CUDA_TOOLKIT_ROOT_DIR}" )
            if( NOT DEFINED CMAKE_C_COMPILER )
                set( CMAKE_C_COMPILER   ${CUDA_TOOLKIT_ROOT_DIR}/bin/gcc )
                MESSAGE( STATUS ">> Set C compiler accordingly" )
            endif( )
            if( NOT DEFINED CMAKE_CXX_COMPILER )
                set( CMAKE_CXX_COMPILER ${CUDA_TOOLKIT_ROOT_DIR}/bin/g++ )
                MESSAGE( STATUS ">> Set CXX compiler accordingly" )
            endif( )
        else( )
            MESSAGE( STATUS ">> No CUDA toolkit root dir specified" )
        endif( )
    elseif( WIN32 )
        # MESSAGE( STATUS ">> We are on Windows... CUDA untested" )
    endif( )
endif( )
#############################################



######### General Paths #####################
### Set the cmake subdirectory
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" )
######### User Paths ########################
include(PathsIFF)
### Set these if you do not want cmake to choose your compiler
# set( USER_COMPILER_C    "gcc" )
# set( USER_COMPILER_CXX  "g++" )
# set( USER_PATH_COMPILER "/usr/bin" )
### Set this if you need cmake to find your QT installation
# set( USER_PATH_QT       "~/QT/5.7" )
#############################################



######### Set all Flags and Options #########
### Choose Compiler
include(ChooseCompiler)
### Project Name
PROJECT(spirit)
### Print compiler info
MESSAGE( STATUS ">> Please check the CMAKE_CXX_COMPILER to make sure it's the right one" )
MESSAGE( STATUS ">> CMAKE_C_COMPILER:               " ${CMAKE_C_COMPILER} )
MESSAGE( STATUS ">> CMAKE_CXX_COMPILER:             " ${CMAKE_CXX_COMPILER} )
### Platform-specific Flags
include(Platforms)
### Compiler-specific Flags
include(GlobalCompilerFlags)
### Prevent in-source builds
# set(CMAKE_DISABLE_SOURCE_CHANGES ON) # we need source changes for the generated VERSION.txt
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
### Position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
### Installation Prefix
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE)
endif()
#############################################



#############################################
if( SPIRIT_BUILD_TEST )
    enable_testing()
endif()
#############################################
if ( SPIRIT_USE_OPENMP )
    include( FindOpenMP )
    if( OPENMP_FOUND )
        set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
        set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}" )
    endif( )
endif( )
#############################################



######### External Project: QHull ###########
set (       CMAKE_QHULL_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/thirdparty-install")
list(APPEND CMAKE_QHULL_ARGS "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
list(APPEND CMAKE_QHULL_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
list(APPEND CMAKE_QHULL_ARGS "-DCMAKE_AR=${CMAKE_AR}")
list(APPEND CMAKE_QHULL_ARGS "-DCMAKE_RANLIB=${CMAKE_RANLIB}")
include(ExternalProject)
ExternalProject_add(qhull
    SOURCE_DIR    "${CMAKE_SOURCE_DIR}/thirdparty/qhull"
    # GIT_REPOSITORY https://github.com/qhull/qhull.git
    BINARY_DIR ${CMAKE_BINARY_DIR}/thirdparty-build/qhull
    # INSTALL_DIR ${CMAKE_BINARY_DIR}/thirdparty-install
    CMAKE_ARGS ${CMAKE_QHULL_ARGS}
    CMAKE_CACHE_ARGS "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true"
)
### qhullstatic reentrant library
add_library(libqhullstatic_r STATIC IMPORTED)
# set_property(TARGET libqhullstatic_r PROPERTY MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
set_property(TARGET libqhullstatic_r PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/${CMAKE_STATIC_LIBRARY_PREFIX}qhullstatic_r${CMAKE_STATIC_LIBRARY_SUFFIX})
if (WIN32)
    set_property(TARGET libqhullstatic_r PROPERTY IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/Debug/${CMAKE_STATIC_LIBRARY_PREFIX}qhullstatic_r${CMAKE_STATIC_LIBRARY_SUFFIX})
    set_property(TARGET libqhullstatic_r PROPERTY IMPORTED_LOCATION_MINSIZEREL ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/MinSizeRel/${CMAKE_STATIC_LIBRARY_PREFIX}qhullstatic_r${CMAKE_STATIC_LIBRARY_SUFFIX})
    set_property(TARGET libqhullstatic_r PROPERTY IMPORTED_LOCATION_RELEASE ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/Release/${CMAKE_STATIC_LIBRARY_PREFIX}qhullstatic_r${CMAKE_STATIC_LIBRARY_SUFFIX})
    set_property(TARGET libqhullstatic_r PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/RelWithDebInfo/${CMAKE_STATIC_LIBRARY_PREFIX}qhullstatic_r${CMAKE_STATIC_LIBRARY_SUFFIX})
endif ()
add_dependencies(libqhullstatic_r qhull)
### qhullcpp library
add_library(libqhullcpp STATIC IMPORTED)
set_property(TARGET libqhullcpp PROPERTY INTERFACE_LINK_LIBRARIES libqhullstatic_r)
# set_property(TARGET libqhullcpp PROPERTY MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
set_property(TARGET libqhullcpp PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/${CMAKE_STATIC_LIBRARY_PREFIX}qhullcpp${CMAKE_STATIC_LIBRARY_SUFFIX})
if (WIN32)
    set_property(TARGET libqhullcpp PROPERTY IMPORTED_LOCATION_DEBUG ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/Debug/${CMAKE_STATIC_LIBRARY_PREFIX}qhullcpp${CMAKE_STATIC_LIBRARY_SUFFIX})
    set_property(TARGET libqhullcpp PROPERTY IMPORTED_LOCATION_MINSIZEREL ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/MinSizeRel/${CMAKE_STATIC_LIBRARY_PREFIX}qhullcpp${CMAKE_STATIC_LIBRARY_SUFFIX})
    set_property(TARGET libqhullcpp PROPERTY IMPORTED_LOCATION_RELEASE ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/Release/${CMAKE_STATIC_LIBRARY_PREFIX}qhullcpp${CMAKE_STATIC_LIBRARY_SUFFIX})
    set_property(TARGET libqhullcpp PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/thirdparty-build/qhull/RelWithDebInfo/${CMAKE_STATIC_LIBRARY_PREFIX}qhullcpp${CMAKE_STATIC_LIBRARY_SUFFIX})
endif ()
add_dependencies(libqhullcpp qhull)
set(qhull_LIBS libqhullcpp)
### Add corresponding include directories
set(qhull_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/thirdparty-install/include;${CMAKE_BINARY_DIR}/thirdparty-install/include/libqhullcpp")
include_directories( ${qhull_INCLUDE_DIRS} )
#############################################


if( SPIRIT_SKIP_HTST )
    add_definitions("-DSPIRIT_SKIP_HTST")
    MESSAGE( STATUS ">> Skipping compilation of HTST!" )
endif( )


######### Add Subdirectory Projects #########
### Spirit library is built in any case
add_subdirectory( core )
### Web UI
if( SPIRIT_BUILD_FOR_JS )
    add_subdirectory( ui-web )
### CXX UI
elseif( SPIRIT_BUILD_FOR_CXX )
    if ( SPIRIT_UI_CXX_USE_QT )
        add_subdirectory( VFRendering )
        add_definitions( -DSPIRIT_UI_CXX_USE_QT )
    endif()
    add_subdirectory( ui-cpp )
endif()
#############################################



################ Install ####################
install(DIRECTORY docs        DESTINATION docs/Spirit)
install(FILES   LICENSE.txt
                README.md
                VERSION.txt   DESTINATION docs/Spirit)
if( SPIRIT_BUILD_FOR_CXX )
    install(DIRECTORY input   DESTINATION bin)
endif( )
#############################################



######### Write VERSION.txt #################
file(WRITE "${CMAKE_SOURCE_DIR}/VERSION.txt" "${SPIRIT_META_NAME_VERSION}")
#############################################
