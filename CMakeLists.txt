cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)
set (BUILD_SHARED_LIBRARIES OFF)
include (ExternalProject)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs -Werror")

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phosphor-dbus-interfaces
)

project (biosconfig-manager CXX)

set (EXTERNAL_PACKAGES Boost sdbusplus-project)
set (CR_LINK_LIBS -lsystemd stdc++fs sdbusplus)

add_definitions (-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions (-DBOOST_SYSTEM_NO_DEPRECATED)
add_definitions (-DBOOST_ALL_NO_LIB)
add_definitions (-DBOOST_NO_RTTI)
add_definitions (-DBOOST_NO_TYPEID)
add_definitions (-DBOOST_ASIO_DISABLE_THREADS)

link_directories (${EXTERNAL_INSTALL_LOCATION}/lib)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories (${SDBUSPLUSPLUS_INCLUDE_DIRS})
link_directories (${SDBUSPLUSPLUS_LIBRARY_DIRS})

include_directories (${DBUSINTERFACE_INCLUDE_DIRS})
link_directories (${DBUSINTERFACE_LIBRARY_DIRS})

add_executable (${PROJECT_NAME} src/BIOSConfigManager.cpp)
add_dependencies (${PROJECT_NAME} sdbusplus-project)
target_link_libraries (${PROJECT_NAME} ${CR_LINK_LIBS})
target_link_libraries (${PROJECT_NAME} phosphor_logging)
target_link_libraries (${PROJECT_NAME} phosphor_dbus)

include_directories (${LOGGING_INCLUDE_DIRS})
link_directories (${LOGGING_LIBRARY_DIRS})

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
set (
    SERVICE_FILES
    ${PROJECT_SOURCE_DIR}/service_files/xyz.openbmc_project.biosconfig_manager.service
)
install (FILES ${SERVICE_FILES} DESTINATION /lib/systemd/system/)


