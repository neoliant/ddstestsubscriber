cmake_minimum_required(VERSION 3.9.4)
project(ddstestsubscriber CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OPENDDS_CMAKE_VERBOSE ON)
set(CMAKE_PREFIX_PATH "/usr/local")

add_compile_definitions(USE_OPENDDS)
add_compile_definitions(OMG_DDS_RPC_REFERENCE_IMPLEMENTATION)
add_compile_definitions(OMG_DDS_RPC_BASIC_PROFILE)

include(CTest)

#
# Add OpenDDS
#
find_package(OpenDDS REQUIRED)
set(CMAKE_CXX_COMPILER ${OPENDDS_COMPILER})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -DBOOST_THREAD_VERSION=4 -DOPENDDS_SECURITY -DXML_USE_PTHREADS")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -DBOOST_THREAD_VERSION=4 -DXML_USE_PTHREADS")

#
# Add Boost
#
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost COMPONENTS REQUIRED system thread)
include_directories(${Boost_INCLUDE_DIR})

#
# Add libopcddsservices supplied with the gateway
#
find_library(OpcDdsServices opcddsservices libopcddsservices/lib/)
include_directories(libopcddsservices/headers)

configure_file(libopcddsservices/IDLs/dds-opcua_builtin_types.idl ${CMAKE_CURRENT_SOURCE_DIR}/dds-opcua_builtin_types.idl COPYONLY)
configure_file(libopcddsservices/IDLs/dds-opcua_services.idl ${CMAKE_CURRENT_SOURCE_DIR}/dds-opcua_services.idl COPYONLY) 
configure_file(libopcddsservices/IDLs/dds-opcua_rpc.idl ${CMAKE_CURRENT_SOURCE_DIR}/dds-opcua_rpc.idl COPYONLY) 

# This helps with finding the dds-opcua_* idl generated files for all projects
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#add_executable(ddstestsubscriber main.cpp printer.cpp)
add_executable(ddstestsubscriber main.cpp DataReaderListenerImpl.cpp MessageEventDataReaderListenerImpl.cpp Printer.cpp)

set_target_properties(ddstestsubscriber PROPERTIES OUTPUT_NAME ddstestsubscriber)
opendds_target_sources(ddstestsubscriber PUBLIC
    "dds-opcua_builtin_types.idl"
    "dds-opcua_services.idl"
    "dds-opcua_rpc.idl"
    "SimpleDataType.idl"
    "MessageEvent.idl"
    TAO_IDL_OPTIONS --idl-version 4 --unknown-annotations ignore -I/usr/local/include
    OPENDDS_IDL_OPTIONS --unknown-annotations ignore -Lc++11
)

target_compile_features(ddstestsubscriber PUBLIC cxx_std_17)
target_link_libraries(ddstestsubscriber OpenDDS::OpenDDS ${OpcDdsServices})

# add boost to target
if(Boost_FOUND)
    include_directories(${BOOST_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(${Boost_DEFINITIONS})
    include_directories(${Boost_INCLUDE_DIRS})  
    target_link_libraries(ddstestsubscriber ${Boost_LIBRARIES})
endif()

# Package ddstestsubscriber into an AppImage
option(BUILD_APPIMAGE "Build AppImage in CI" OFF)
if(BUILD_APPIMAGE)
