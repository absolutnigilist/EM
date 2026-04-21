cmake_minimum_required(VERSION 3.18)

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>")
endif()

project(EM LANGUAGES CXX)

# ------------------------------------------------------------
# Общие настройки проекта
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED APP_NAME)
    set(APP_NAME "EM")
endif()

if(NOT DEFINED PROJECT_VERSION_STRING)
    set(PROJECT_VERSION_STRING "1.0.0")
endif()

# ------------------------------------------------------------
# Распаковка glog при первом запуске
# ------------------------------------------------------------
set(LOCAL_GLOG_DIR "${CMAKE_SOURCE_DIR}/glog")

if(WIN32)
    set(GLOG_ARCHIVE "${CMAKE_SOURCE_DIR}/glog.7z")
else()
    set(GLOG_ARCHIVE "${CMAKE_SOURCE_DIR}/glog.tar.gz")
endif()

set(GLOG_HEADER "${LOCAL_GLOG_DIR}/include/glog/logging.h")

if(NOT EXISTS "${GLOG_HEADER}")
    if(NOT EXISTS "${GLOG_ARCHIVE}")
        message(FATAL_ERROR "glog archive not found: ${GLOG_ARCHIVE}")
    endif()

    message(STATUS "glog is not unpacked yet. Extracting ${GLOG_ARCHIVE} ...")

    file(ARCHIVE_EXTRACT
        INPUT "${GLOG_ARCHIVE}"
        DESTINATION "${CMAKE_SOURCE_DIR}"
    )

    if(NOT EXISTS "${GLOG_HEADER}")
        message(FATAL_ERROR
            "glog archive was extracted, but ${GLOG_HEADER} was not found. "
            "Check archive structure.")
    endif()
endif()

# Подсказываем CMake, где искать локальный glog
if(EXISTS "${LOCAL_GLOG_DIR}/lib/cmake/glog")
    set(glog_DIR "${LOCAL_GLOG_DIR}/lib/cmake/glog")
    list(PREPEND CMAKE_PREFIX_PATH "${LOCAL_GLOG_DIR}")
endif()

# ------------------------------------------------------------
# Поиск glog
# ------------------------------------------------------------
find_package(glog 0.6.0 CONFIG QUIET)

if(NOT TARGET glog::glog)
    if(WIN32)
        if(EXISTS "${LOCAL_GLOG_DIR}/lib/glog.lib")
            add_library(glog::glog UNKNOWN IMPORTED)
            set_target_properties(glog::glog PROPERTIES
                IMPORTED_LOCATION "${LOCAL_GLOG_DIR}/lib/glog.lib"
                INTERFACE_INCLUDE_DIRECTORIES "${LOCAL_GLOG_DIR}/include"
            )
        else()
            message(FATAL_ERROR
                "glog::glog was not found and local Windows library "
                "${LOCAL_GLOG_DIR}/lib/glog.lib does not exist.")
        endif()
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GLOG REQUIRED IMPORTED_TARGET libglog)
        add_library(glog::glog ALIAS PkgConfig::GLOG)
    endif()
endif()

# ------------------------------------------------------------
# Подключение модулей
# ------------------------------------------------------------
add_subdirectory(List)
add_subdirectory(ListSerializer)

# ------------------------------------------------------------
# Главный исполняемый файл
# ------------------------------------------------------------
add_executable(main
    "${CMAKE_SOURCE_DIR}/main.cpp"
    "${CMAKE_SOURCE_DIR}/ListNode.hpp"
)

target_link_libraries(main PRIVATE
    List
    ListSerializer
    glog::glog
)

target_include_directories(main PRIVATE
    "${CMAKE_SOURCE_DIR}"
)

target_compile_definitions(main PRIVATE
    APP_NAME="${APP_NAME}"
    PROJECT_VERSION_STRING="${PROJECT_VERSION_STRING}"
)

set_target_properties(main PROPERTIES
    OUTPUT_NAME "${APP_NAME}"
)

if(MSVC)
    target_compile_options(main PRIVATE /bigobj)
elseif(MINGW)
    target_compile_options(main PRIVATE -Wa,-mbig-obj)
endif()