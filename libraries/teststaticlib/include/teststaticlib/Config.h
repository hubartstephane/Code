// BEWARE: this file is generated by premake. do not edit by hand

#pragma once

// kind of project
#define TESTDLL_IS_SHARED_LIBRARY

// defines an import/export token
#if defined TESTDLL_IS_SHARED_LIBRARY
#  if defined DEATH_IS_BUILDING_TESTDLL // are we building the project, or this file is included from external ?
#    define TESTDLL_API __declspec(dllexport)
#  else
#    define TESTDLL_API __declspec(dllimport)
#  endif
#else
#  define TESTDLL_API // no import/export unless for shared library
#endif
