#=========================================================
# Find ITK (required)
find_package(ITK 6.0 REQUIRED)
include("${ITK_USE_FILE}")
#=========================================================

#=========================================================
# Find VTK (required)
find_package(VTK 9.5 REQUIRED)
#========================================================

#=========================================================
# Find gengetopt, will create a target exe if not found
set(CMAKE_MODULE_PATH "${CLITK_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(Gengetopt)
#=========================================================

#=========================================================
# Find libstatgrab is installed, add clitkMemoryUsage.cxx in the library
if(NOT DEFINED CLITK_MEMORY_INFO OR CLITK_MEMORY_INFO)
  find_library(LIBSTATGRAB NAMES statgrab PATHS)
  if(${LIBSTATGRAB} MATCHES "LIBSTATGRAB-NOTFOUND")
#  message("Install libstatgrab (http://www.i-scream.org/libstatgrab/) for memory usage information")
    set(CLITK_MEMORY_INFO OFF)
  else(${LIBSTATGRAB} MATCHES "LIBSTATGRAB-NOTFOUND")
    set(CLITK_MEMORY_INFO ON)
  endif(${LIBSTATGRAB} MATCHES "LIBSTATGRAB-NOTFOUND")
endif()
#=========================================================


#=========================================================
### Check if ITK was compiled with SYSTEM_GDCM = ON (Not possible anymore with ITK 4.13)
### Add option to activate external GDCM in clitk
set(CLITK_USE_SYSTEM_GDCM FALSE)
if(CLITK_EXTERNAL_GDCM)
  message("Be sure to use external GDCM with ITK")
  set(CLITK_USE_SYSTEM_GDCM TRUE)
endif()


