#
# Copyright (C) 2018-2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(MAX_CORE 64)

set(ALL_CORE_TYPES "")
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake${BRANCH_DIR_SUFFIX}fill_core_types.cmake)

set(ALL_CORE_TYPES_REVERSED ${ALL_CORE_TYPES})
list(REVERSE ALL_CORE_TYPES_REVERSED)

macro(FIND_IDX_FOR_CORE_TYPE CORE_TYPE CORE_IDX)
  list(FIND ALL_CORE_TYPES "${CORE_TYPE}" CORE_IDX)
  if(${CORE_IDX} EQUAL -1)
    message(FATAL_ERROR "No ${CORE_TYPE} allowed, exiting")
  endif()
endmacro()

macro(INIT_LIST LIST_TYPE ELEMENT_TYPE)
  foreach(IT RANGE 0 ${MAX_CORE} 1)
    list(APPEND ALL_${ELEMENT_TYPE}_${LIST_TYPE} " ")
  endforeach()
endmacro()

macro(GET_LIST_FOR_CORE_TYPE LIST_TYPE ELEMENT_TYPE CORE_IDX OUT_LIST)
  list(GET ALL_${ELEMENT_TYPE}_${LIST_TYPE} ${CORE_IDX} CORE_X_${LIST_TYPE})
  string(REPLACE "," ";" ${OUT_LIST} ${CORE_X_${LIST_TYPE}})
endmacro()

macro(ADD_ITEM_FOR_CORE_TYPE LIST_TYPE ELEMENT_TYPE CORE_TYPE ITEM)
  FIND_IDX_FOR_CORE_TYPE(${CORE_TYPE} CORE_IDX)
  list(GET ALL_${ELEMENT_TYPE}_${LIST_TYPE} ${CORE_IDX} CORE_X_LIST)
  string(REPLACE " " "" CORE_X_LIST ${CORE_X_LIST})
  if("${CORE_X_LIST}" STREQUAL "")
    set(CORE_X_LIST "${ITEM}")
  else()
    set(CORE_X_LIST "${CORE_X_LIST},${ITEM}")
  endif()
  list(REMOVE_AT ALL_${ELEMENT_TYPE}_${LIST_TYPE} ${CORE_IDX})
  list(INSERT ALL_${ELEMENT_TYPE}_${LIST_TYPE} ${CORE_IDX} ${CORE_X_LIST})
endmacro()

macro(CORE_CONTAINS_ANY_PLATFORM TYPE CORE_TYPE OUT_FLAG)
  FIND_IDX_FOR_CORE_TYPE(${CORE_TYPE} CORE_IDX)
  GET_LIST_FOR_CORE_TYPE("PLATFORMS" ${TYPE} ${CORE_IDX} CORE_X_PLATFORMS)
  set(${OUT_FLAG} FALSE)
  foreach(PLATFORM_IT ${CORE_X_PLATFORMS})
    if(NOT ("${PLATFORM_IT}" STREQUAL " "))
      set(${OUT_FLAG} TRUE)
    endif()
  endforeach()
endmacro()

macro(CORE_CONTAINS_PLATFORM TYPE CORE_TYPE PLATFORM_TYPE OUT_FLAG)
  FIND_IDX_FOR_CORE_TYPE("${CORE_TYPE}" CORE_IDX)
  GET_LIST_FOR_CORE_TYPE("PLATFORMS" "${TYPE}" ${CORE_IDX} CORE_X_PLATFORMS)
  set(${OUT_FLAG} FALSE)
  foreach(PLATFORM_IT ${CORE_X_PLATFORMS})
    if("${PLATFORM_IT}" STREQUAL "${PLATFORM_TYPE}")
      set(${OUT_FLAG} TRUE)
    endif()
  endforeach()
endmacro()

macro(INIT_PRODUCTS_LIST TYPE)
  list(APPEND ALL_${TYPE}_PRODUCT_FAMILY " ")
  list(APPEND ALL_${TYPE}_PRODUCT_TO_PRODUCT_FAMILY " ")
endmacro()

macro(ADD_PRODUCT TYPE PRODUCT ITEM)
  list(APPEND ALL_${TYPE}_PRODUCT_FAMILY ${ITEM})
  list(APPEND ALL_${TYPE}_PRODUCT_TO_PRODUCT_FAMILY ${PRODUCT})
endmacro()

macro(GET_AVAILABLE_PRODUCTS TYPE PRODUCT_FAMILY_LIST DEFAULT_PRODUCT_FAMILY)
  list(REMOVE_ITEM ALL_${TYPE}_PRODUCT_FAMILY " ")
  list(REMOVE_ITEM ALL_${TYPE}_PRODUCT_TO_PRODUCT_FAMILY " ")

  set(${PRODUCT_FAMILY_LIST} ${ALL_${TYPE}_PRODUCT_FAMILY})
  set(${DEFAULT_PRODUCT_FAMILY})

  if(NOT "${DEFAULT_${TYPE}_PLATFORM}" STREQUAL "")
    list(FIND ALL_${TYPE}_PRODUCT_TO_PRODUCT_FAMILY ${DEFAULT_${TYPE}_PLATFORM} INDEX)
    if(${INDEX} EQUAL -1)
      message(FATAL_ERROR "${DEFAULT_${TYPE}_PLATFORM} not found in product families.")
    endif()

    list(GET ALL_${TYPE}_PRODUCT_FAMILY ${INDEX} DEFAULT)
    set(${DEFAULT_PRODUCT_FAMILY} ${DEFAULT})
  endif()
endmacro()

macro(GET_AVAILABLE_PLATFORMS TYPE FLAG_NAME OUT_STR)
  set(${TYPE}_PLATFORM_LIST)
  set(${TYPE}_CORE_FLAGS_DEFINITONS)
  if(NOT DEFAULT_${TYPE}_PLATFORM AND DEFINED PREFERRED_PLATFORM AND ${FLAG_NAME}_${PREFERRED_PLATFORM})
    set(DEFAULT_${TYPE}_PLATFORM ${PREFERRED_PLATFORM})
  endif()
  foreach(CORE_TYPE ${ALL_CORE_TYPES_REVERSED})
    set(COREX_HAS_PLATFORMS FALSE)
    CORE_CONTAINS_ANY_PLATFORM("${TYPE}" ${CORE_TYPE} COREX_HAS_PLATFORMS)
    if(${COREX_HAS_PLATFORMS})
      FIND_IDX_FOR_CORE_TYPE(${CORE_TYPE} CORE_IDX)
      list(APPEND ${TYPE}_CORE_FLAGS_DEFINITONS ${FLAG_NAME}_${CORE_TYPE})
      GET_LIST_FOR_CORE_TYPE("PLATFORMS" ${TYPE} ${CORE_IDX} ${TYPE}_COREX_PLATFORMS)
      list(APPEND ${TYPE}_PLATFORM_LIST ${${TYPE}_COREX_PLATFORMS})
      if(NOT DEFAULT_${TYPE}_PLATFORM)
        list(GET ${TYPE}_PLATFORM_LIST 0 DEFAULT_${TYPE}_PLATFORM ${PLATFORM_IT})
      endif()
      if(NOT DEFAULT_${TYPE}_${CORE_TYPE}_PLATFORM)
        list(GET ${TYPE}_COREX_PLATFORMS 0 DEFAULT_${TYPE}_${CORE_TYPE}_PLATFORM)
      endif()
    endif()
  endforeach()
  foreach(PLATFORM_IT ${${TYPE}_PLATFORM_LIST})
    set(${OUT_STR} "${${OUT_STR}} ${PLATFORM_IT}")
    list(APPEND ${TYPE}_CORE_FLAGS_DEFINITONS ${FLAG_NAME}_${PLATFORM_IT})
  endforeach()
endmacro()

macro(GET_PLATFORMS_FOR_CORE_TYPE TYPE CORE_TYPE OUT_LIST)
  FIND_IDX_FOR_CORE_TYPE(${CORE_TYPE} CORE_IDX)
  GET_LIST_FOR_CORE_TYPE("PLATFORMS" ${TYPE} ${CORE_IDX} ${OUT_LIST})
endmacro()

macro(PLATFORM_HAS_2_0 CORE_TYPE PLATFORM_NAME OUT_FLAG)
  FIND_IDX_FOR_CORE_TYPE(${CORE_TYPE} CORE_IDX)
  GET_LIST_FOR_CORE_TYPE("PLATFORMS" "SUPPORTED_2_0" ${CORE_IDX} CORE_X_PLATFORMS)
  list(FIND CORE_X_PLATFORMS ${PLATFORM_NAME} PLATFORM_EXISTS)
  if("${PLATFORM_EXISTS}" LESS 0)
    set(${OUT_FLAG} FALSE)
  else()
    set(${OUT_FLAG} TRUE)
  endif()
endmacro()

# default flag for CoreX devices support
set(SUPPORT_GEN_DEFAULT TRUE CACHE BOOL "default value for SUPPORT_COREx")
# default flag for platform support
set(SUPPORT_PLATFORM_DEFAULT TRUE CACHE BOOL "default value for support platform")

# Define the hardware configurations we support and test
macro(SET_FLAGS_FOR CORE_TYPE)
  foreach(SKU_NAME ${ARGN})
    if(SUPPORT_${SKU_NAME})
      if(NOT SUPPORT_${CORE_TYPE})
        message(STATUS "Auto-Enabling ${CORE_TYPE} support for ${SKU_NAME}")
        set(SUPPORT_${CORE_TYPE} TRUE CACHE BOOL "Support ${CORE_TYPE} devices" FORCE)
      endif()
    endif()
    if(TESTS_${SKU_NAME})
      if(NOT TESTS_${CORE_TYPE})
        message(STATUS "Auto-Enabling ${CORE_TYPE} tests for ${SKU_NAME}")
        set(TESTS_${CORE_TYPE} TRUE CACHE BOOL "Build ULTs for ${CORE_TYPE} devices" FORCE)
      endif()
    endif()
    string(TOLOWER ${CORE_TYPE} MAP_${SKU_NAME}_CORE_lower)
    string(TOLOWER ${SKU_NAME} MAP_${SKU_NAME}_lower)
    set(MAP_${SKU_NAME}_CORE_lower "${CORE_PREFIX}${MAP_${SKU_NAME}_CORE_lower}${CORE_SUFFIX}" CACHE STRING "Core name for SKU" FORCE)
    set(MAP_${SKU_NAME}_lower ${MAP_${SKU_NAME}_lower} CACHE STRING "SKU in lower case" FORCE)
  endforeach()

  set(SUPPORT_${CORE_TYPE} ${SUPPORT_GEN_DEFAULT} CACHE BOOL "Support ${CORE_TYPE} devices")
  set(TESTS_${CORE_TYPE} ${SUPPORT_${CORE_TYPE}} CACHE BOOL "Build ULTs for ${CORE_TYPE} devices")

  if(NOT SUPPORT_${CORE_TYPE} OR NEO_SKIP_UNIT_TESTS)
    set(TESTS_${CORE_TYPE} FALSE)
  endif()

  if(SUPPORT_${CORE_TYPE})
    list(APPEND ALL_SUPPORTED_CORE_FAMILIES ${CORE_TYPE})
    list(REMOVE_DUPLICATES ALL_SUPPORTED_CORE_FAMILIES)

    foreach(${CORE_TYPE}_PLATFORM ${ARGN})
      set(SUPPORT_${${CORE_TYPE}_PLATFORM} ${SUPPORT_PLATFORM_DEFAULT} CACHE BOOL "Support ${${CORE_TYPE}_PLATFORM}")
      if(TESTS_${CORE_TYPE})
        set(TESTS_${${CORE_TYPE}_PLATFORM} ${SUPPORT_${${CORE_TYPE}_PLATFORM}} CACHE BOOL "Build ULTs for ${${CORE_TYPE}_PLATFORM}")
      endif()
      if(NOT SUPPORT_${${CORE_TYPE}_PLATFORM} OR NOT TESTS_${CORE_TYPE} OR NEO_SKIP_UNIT_TESTS)
        set(TESTS_${${CORE_TYPE}_PLATFORM} FALSE)
      endif()
    endforeach()
  endif()

  if(TESTS_${CORE_TYPE})
    list(APPEND ALL_TESTED_CORE_FAMILIES ${CORE_TYPE})
    list(REMOVE_DUPLICATES ALL_TESTED_CORE_FAMILIES)
  endif()
endmacro()

macro(DISABLE_FLAGS_FOR CORE_TYPE)
  set(SUPPORT_${CORE_TYPE} FALSE CACHE BOOL "Support ${CORE_TYPE} devices" FORCE)
  set(TESTS_${CORE_TYPE} FALSE CACHE BOOL "Build ULTs for ${CORE_TYPE} devices" FORCE)
  foreach(SKU_NAME ${ARGN})
    set(SUPPORT_${SKU_NAME} FALSE CACHE BOOL "Support ${SKU_NAME}" FORCE)
    set(TESTS_${SKU_NAME} FALSE CACHE BOOL "Build ULTs for ${SKU_NAME}" FORCE)
  endforeach()
endmacro()

macro(DISABLE_32BIT_FLAGS_FOR CORE_TYPE)
  DISABLE_FLAGS_FOR(${CORE_TYPE} ${ARGN})
endmacro()

macro(DISABLE_WDDM_LINUX_FOR CORE_TYPE)
  if(SUPPORT_${CORE_TYPE})
    foreach(SKU_NAME ${ARGN})
      if(SUPPORT_${SKU_NAME})
        list(APPEND PLATFORMS_TO_HAVE_WDDM_DISABLED ${SKU_NAME})
        list(REMOVE_DUPLICATES PLATFORMS_TO_HAVE_WDDM_DISABLED)
      endif()
    endforeach()
  endif()
endmacro()

macro(ADD_PLATFORM_FOR_CORE_TYPE LIST_TYPE CORE_TYPE PLATFORM_NAME)
  ADD_ITEM_FOR_CORE_TYPE("PLATFORMS" ${LIST_TYPE} ${CORE_TYPE} ${PLATFORM_NAME})
  set(${CORE_TYPE}_HAS_${PLATFORM_NAME} TRUE)
  if(NOT DEFAULT_${LIST_TYPE}_${CORE_TYPE}_${PLATFORM_NAME}_PLATFORM)
    string(TOLOWER ${PLATFORM_NAME} DEFAULT_${LIST_TYPE}_${CORE_TYPE}_${PLATFORM_NAME}_PLATFORM)
  endif()
endmacro()

macro(ENABLE_ADDITIONAL_SKU SKU_NAME)
  set(SUPPORT_${SKU_NAME} TRUE CACHE BOOL "Support ${SKU_NAME}" FORCE)
endmacro()

macro(TEST_ADDITIONAL_SKU SKU_NAME)
  set(TESTS_${SKU_NAME} TRUE CACHE BOOL "Build ULTs for ${SKU_NAME}" FORCE)
endmacro()

macro(ADD_AOT_DEFINITION CONFIG_NAME)
  list(APPEND AOT_DEFINITIONS "SUPPORT_AOT_${CONFIG_NAME}")
endmacro()

macro(SET_FLAGS_FOR_LEGACY CORE_TYPE)
  if(NEO_LEGACY_PLATFORMS_SUPPORT)
    SET_FLAGS_FOR(${CORE_TYPE} ${ARGN})
  else()
    DISABLE_FLAGS_FOR(${CORE_TYPE} ${ARGN})
  endif()
endmacro()

macro(SET_FLAGS_FOR_CURRENT CORE_TYPE)
  if(NEO_CURRENT_PLATFORMS_SUPPORT)
    SET_FLAGS_FOR(${CORE_TYPE} ${ARGN})
  else()
    DISABLE_FLAGS_FOR(${CORE_TYPE} ${ARGN})
  endif()
endmacro()
# Init lists
INIT_LIST("FAMILY_NAME" "TESTED")
INIT_LIST("PLATFORMS" "SUPPORTED")
INIT_LIST("PLATFORMS" "SUPPORTED_2_0")
INIT_LIST("PLATFORMS" "SUPPORTED_IMAGES")
INIT_LIST("PLATFORMS" "SUPPORTED_AUX_TRANSLATION")
INIT_LIST("PLATFORMS" "TESTED")
INIT_LIST("PLATFORMS" "SUPPORTED_HEAPLESS")
INIT_PRODUCTS_LIST("TESTED")
INIT_PRODUCTS_LIST("SUPPORTED")

set(AOT_DEFINITIONS)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake${BRANCH_DIR_SUFFIX}setup_platform_flags.cmake)

# Get platform lists, flag definition and set default platforms
GET_AVAILABLE_PLATFORMS("SUPPORTED" "SUPPORT" ALL_AVAILABLE_SUPPORTED_PLATFORMS)
GET_AVAILABLE_PLATFORMS("TESTED" "TESTS" ALL_AVAILABLE_TESTED_PLATFORMS)
GET_AVAILABLE_PRODUCTS("TESTED" ALL_PRODUCT_FAMILY_LIST DEFAULT_TESTED_PRODUCT_FAMILY)
GET_AVAILABLE_PRODUCTS("SUPPORTED" ALL_PRODUCT_FAMILY_LIST DEFAULT_SUPPORTED_PRODUCT_FAMILY)

# Output platforms
message(STATUS "All supported platforms: ${ALL_AVAILABLE_SUPPORTED_PLATFORMS}")
message(STATUS "All tested platforms: ${ALL_AVAILABLE_TESTED_PLATFORMS}")
message(STATUS "Default supported platform: ${DEFAULT_SUPPORTED_PLATFORM}")
message(STATUS "Default tested platform: ${DEFAULT_TESTED_PLATFORM}")

# Output families
message(STATUS "All supported core families: ${ALL_SUPPORTED_CORE_FAMILIES}")
message(STATUS "All tested core families: ${ALL_TESTED_CORE_FAMILIES}")

list(FIND SUPPORTED_PLATFORM_LIST ${DEFAULT_SUPPORTED_PLATFORM} VALID_DEFAULT_SUPPORTED_PLATFORM)
if(VALID_DEFAULT_SUPPORTED_PLATFORM LESS 0)
  message(FATAL_ERROR "Not a valid supported platform: ${DEFAULT_SUPPORTED_PLATFORM}")
endif()

if(DEFAULT_TESTED_PLATFORM)
  list(FIND TESTED_PLATFORM_LIST ${DEFAULT_TESTED_PLATFORM} VALID_DEFAULT_TESTED_PLATFORM)
  if(VALID_DEFAULT_TESTED_PLATFORM LESS 0)
    message(FATAL_ERROR "Not a valid tested platform: ${DEFAULT_TESTED_PLATFORM}")
  endif()
else()
  set(NEO_SKIP_UNIT_TESTS TRUE)
endif()

if(NOT DEFAULT_TESTED_FAMILY_NAME)
  if(DEFINED PREFERRED_FAMILY_NAME)
    list(FIND ALL_TESTED_FAMILY_NAME ${PREFERRED_FAMILY_NAME} CORE_IDX)
    if(${CORE_IDX} GREATER -1)
      set(DEFAULT_TESTED_FAMILY_NAME ${PREFERRED_FAMILY_NAME})
    endif()
  endif()
  if(NOT DEFINED DEFAULT_TESTED_FAMILY_NAME)
    foreach(CORE_TYPE ${ALL_CORE_TYPES_REVERSED})
      FIND_IDX_FOR_CORE_TYPE(${CORE_TYPE} CORE_IDX)
      list(GET ALL_TESTED_FAMILY_NAME ${CORE_IDX} CORE_FAMILY_NAME)
      if(NOT CORE_FAMILY_NAME STREQUAL " ")
        set(DEFAULT_TESTED_FAMILY_NAME ${CORE_FAMILY_NAME})
        break()
      endif()
    endforeach()
  endif()
endif()
message(STATUS "Default tested family name: ${DEFAULT_TESTED_FAMILY_NAME}")
