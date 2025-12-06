cmake_minimum_required(VERSION 3.12...3.31)
project(linkshrink-app CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)


find_package(
    prometheus-cpp 
    REQUIRED
)

find_package(
    userver
    COMPONENTS postgresql
    REQUIRED
)
if(NOT userver_FOUND)
  # Tries TRY_DIR first, falls back to downloading userver from GitHub using CPM.
  download_userver(TRY_DIR third_party/userver VERSION 2.8)
endif()

userver_setup_environment()

# Common sources
add_library(${PROJECT_NAME}_objs OBJECT
    src/handlers/shorten.hpp
    src/handlers/shorten.cpp
    src/handlers/redirect.hpp
    src/handlers/redirect.cpp
    src/utils/short_code.hpp
    src/utils/short_code.cpp
    src/handlers/stats.hpp
    src/handlers/stats.cpp
    src/models/click_info.hpp
    src/storage/statistics_accumulator.hpp
    src/storage/statistics_accumulator.cpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC 
    userver::core 
    userver::postgresql
)


# The Service
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/prometheus_metrics.cpp
    src/handlers/prometheus_handler.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs prometheus-cpp)

# Unit Tests
add_executable(${PROJECT_NAME}_unittest
    src/utils/short_code_test.cpp
)
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
add_google_tests(${PROJECT_NAME}_unittest)


# Benchmarks
add_executable(${PROJECT_NAME}_benchmark
    src/utils/short_code_benchmark.cpp
)
target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_NAME}_objs userver::ubench)
add_google_benchmark_tests(${PROJECT_NAME}_benchmark)

# Functional testing
userver_testsuite_add_simple()

# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})
