/* Copyright (C) neoliant.com - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary
 * Written by neoliant - info@neoliant.com, February 2021
 */

#include "DataReaderListenerImpl.h"
#include "MessageEventDataReaderListenerImpl.h"
#include "Printer.h"

// Generated at build time
#include <SimpleDataTypeTypeSupportImpl.h>
#include <MessageEventTypeSupportImpl.h>

// Needed to use the libopcddsservices library
#include "libopcddsservices/headers/Client.h"
#include "libopcddsservices/headers/ClientParams.h"

// Logging provided by ACE
#include <ace/Log_Msg.h>

// Boost is used to manage asynchronous calls, using futures
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread/future.hpp>
#include <boost/filesystem/path.hpp>

#include <exception>
//#include <filesystem>
#include <stdlib.h>
#include <string>
#include <thread>

// OpenDDS provides the DDS infrastructure
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DdsDcpsCoreC.h>

using namespace boost;
using namespace std;
using namespace OpenDDS::DCPS;
using namespace OMG::DDSOPCUA::OPCUA2DDS;

std::mutex mtx;

#ifndef UNUSED
#define UNUSED( expr ) do { ( void )( expr ); } while ( 0 )
#endif

bool running = true;
static void stopHandler ( int sign )
{
    UNUSED ( sign );
    cout << "Received Ctrl-C" << endl;
    running = false;
}

#ifdef OPENDDS_SECURITY
#include <dds/DCPS/security/framework/Properties.h>
const char auth_ca_file_from_tests[] = "certs/identity/identity_ca_cert.pem";
const char perm_ca_file_from_tests[] = "certs/identity/identity_ca_cert.pem";
const char id_cert_file_from_tests[] = "certs/identity/ddstestsubscriber_cert.pem";
const char id_key_file_from_tests[] = "certs/identity/ddstestsubscriber_private_key.pem";
const char * home = getenv( "OWD" );
const char governance_file[] = "governance_signed.p7s";
const char permissions_file[] = "permissions_ddstestsubscriber_signed.p7s";


void append ( DDS::PropertySeq& props, const char* name, const char* value, bool propagate = false )
{
    const DDS::Property_t prop = {name, value, propagate};
    const unsigned int len = props.length();
    props.length ( len + 1 );
    props[len] = prop;
}

bool fileExists ( const std::string & fileName )
{
    struct stat buffer;
    return (stat (fileName.c_str(), &buffer) == 0);
}

void setSecureEnvironment ( DDS::PropertySeq & props )
{
    if (!home) {
        throw std::runtime_error("OWD environment variable not set");
    }
    const std::string base_path = std::string(home) + "/config/security/DDS/";

    if ( !fileExists( base_path + auth_ca_file_from_tests ) ) {
        throw std::runtime_error("File does not exist: " + base_path + auth_ca_file_from_tests );
    }
    if ( !fileExists( base_path + perm_ca_file_from_tests ) ) {
        throw std::runtime_error("File does not exist: " + base_path + perm_ca_file_from_tests );
    }
    if ( !fileExists( base_path + id_cert_file_from_tests ) ) {
        throw std::runtime_error("File does not exist: " + base_path + id_cert_file_from_tests );
    }
    if ( !fileExists( base_path + id_key_file_from_tests ) ) {
        throw std::runtime_error("File does not exist: " + base_path + id_key_file_from_tests );
    }
    if ( !fileExists( base_path + governance_file ) ) {
        throw std::runtime_error("File does not exist: " + base_path + governance_file );
    }
    if ( !fileExists( base_path + permissions_file ) ) {
        throw std::runtime_error("File does not exist: " + base_path + permissions_file );
    }

    const OPENDDS_STRING auth_ca_file = string( "file:" ) + base_path + auth_ca_file_from_tests;
    const OPENDDS_STRING perm_ca_file = string( "file:" ) + base_path + perm_ca_file_from_tests;
    const OPENDDS_STRING id_cert_file = string( "file:" ) + base_path + id_cert_file_from_tests;
    const OPENDDS_STRING id_key_file = string( "file:" ) + base_path + id_key_file_from_tests;
    const OPENDDS_STRING gov_file = string( "file:" ) + base_path + governance_file;
    const OPENDDS_STRING perm_file = string( "file:" ) + base_path + permissions_file;

    if ( TheServiceParticipant->get_security() ) {
        append ( props, DDS::Security::Properties::AuthIdentityCA, auth_ca_file.c_str() );
        append ( props, DDS::Security::Properties::AuthIdentityCertificate, id_cert_file.c_str() );
        append ( props, DDS::Security::Properties::AuthPrivateKey, id_key_file.c_str() );
        append ( props, DDS::Security::Properties::AccessPermissionsCA, perm_ca_file.c_str() );
        append ( props, DDS::Security::Properties::AccessGovernance, gov_file.c_str() );
        append ( props, DDS::Security::Properties::AccessPermissions, perm_file.c_str() );
    }
}
#endif

vector<std::chrono::duration<long, std::ratio<1, 1000000>>::rep> methodLatencies;

void test_synchronous_method ( METHOD::Client & method_client )
{
    try {
        ResponseHeader respHeader;
        string serverId = "TestServer";
        vector<METHOD::CallMethodResult> results;
        vector<DiagnosticInfo> diagnosticInfos;
        vector<METHOD::CallMethodRequest> methodsToCall;

        METHOD::CallMethodRequest request;
        // The server itself exposes the method
        NodeId objectNodeId;
        objectNodeId.identifier_type().numeric_id ( 85 );
        objectNodeId.namespace_index( 0 );
        request.object_id( objectNodeId );

        // Method that adds delta to each value of an array of 5 Int32
        NodeId methodNodeId;
        methodNodeId.identifier_type().string_id ( "IncInt32ArrayValues" );
        methodNodeId.namespace_index( 1 );
        request.method_id( methodNodeId );

        int array[ 5 ] = { 0, 1, 2, 3, 4 };
        CORBA::Long delta = 3;
        BaseDataTypeList baseDataTypeList;
        baseDataTypeList.reserve ( 2 );

        Variant inputArray;
        UInt32Seq dim;
        dim.reserve( 1 );
        dim.push_back( 5 );
        inputArray.array_dimensions( dim );
        VariantValueSeq seq;
        seq.reserve ( 5 );
        for ( int i = 0; i < 5; i++ ) {
            VariantValue variantValue;
            variantValue.int32_value ( array[i] );
            seq.push_back( variantValue );
        }
        inputArray.value( seq );

        Variant inputDelta;
        UInt32Seq dimDelta;
        dimDelta.reserve ( 1 );
        dimDelta.push_back( 1 );
        inputDelta.array_dimensions( dimDelta );
        VariantValueSeq alone;
        alone.reserve ( 1 );
        VariantValue variantValue;
        variantValue.int32_value ( delta );
        alone.push_back( variantValue );
        inputDelta.value( alone );

        baseDataTypeList.push_back( inputArray );
        baseDataTypeList.push_back( inputDelta );
        request.input_arguments( baseDataTypeList );

        methodsToCall.push_back ( request );

        method_client.call ( respHeader, serverId, results, diagnosticInfos, methodsToCall );

        if ( respHeader.service_result() == DDS::RETCODE_TIMEOUT ) {
            cout << "main() : Timeout reached while performing synchronous call" << endl;
        }

        for ( auto result : results ) {
            if ( result.status_code() != 0 ) {
                cout << "status_code != 0 while calling the OPC Method" << endl;
            } else {
                for ( int j = 0; j < result.output_arguments().size(); j++ ) {
                    cout << "Back in test_synchronous, let's display the result : ";
                    Variant variant = result.output_arguments()[ j ];
                    Printer printer;
                    cout << printer.PrintVariant ( variant ) << endl;
                }
            }
        }
    } catch ( std::exception & ex ) {
        cout << "Exception in METHOD::Client::call() : " << ex.what() << endl;
    } catch ( ... ) {
        cout << "Unknown exception in METHOD::Client::call()" << endl;
    }
}

int kounter = 0;

void test_asynchronous_method ( METHOD::Client & method_client )
{
    try {
        string serverId = "TestServer";

        vector<METHOD::CallMethodRequest> methodsToCall;
        METHOD::CallMethodRequest request;

        // The server itself exposes the method
        NodeId objectNodeId;
        objectNodeId.identifier_type().numeric_id ( 85 );
        objectNodeId.namespace_index( 0 );
        request.object_id( objectNodeId );

        // Method that adds delta to each value of an array of 5 Int32
        NodeId methodNodeId;
        methodNodeId.identifier_type().string_id ( "IncInt32ArrayValues" );
        methodNodeId.namespace_index( 1 );
        request.method_id( methodNodeId );

        int array[5] = { 10, 11, 12, 13, 14 };
        CORBA::Long delta = 5;
        request.input_arguments().reserve( 2 );

        Variant inputArray;
        UInt32Seq dim;
        dim.reserve( 1 );
        dim.push_back( 5 );
        inputArray.array_dimensions( dim );
        VariantValueSeq seq;
        seq.reserve( 5 );
        for ( int i = 0; i < 5; i++ ) {
            VariantValue variantValue;
            seq.push_back( variantValue );
            seq[i].int32_value ( array[i] );
        }
        inputArray.value( seq );

        Variant inputDelta;
        UInt32Seq dimDelta;
        dimDelta.reserve( 1 );
        dimDelta.push_back( 1 );
        inputDelta.array_dimensions( dimDelta );
        VariantValueSeq alone;
        alone.reserve( 1 );
        alone.push_back( *( new VariantValue ) );
        alone[0].int32_value ( delta );
        inputDelta.value( alone );

        request.input_arguments().push_back( inputArray );
        request.input_arguments().push_back( inputDelta );

        methodsToCall.push_back ( request );

        for ( int i = 0; i < 3; i++ ) {

            method_client.call_async ( serverId, methodsToCall )
                .then ( [] ( boost::future<METHOD::Method_call_Out> && method_fut ) {
                    kounter++;

                    auto results = method_fut.get();
                    if ( results.results().size() > 0 ) {
                        for ( int j = 0; j < results.results().size(); j++ ) {
                            if ( results.results()[j].status_code() != 0 ) {
                                cout << "status_code != 0 while calling the OPC Method" << endl;
                            } else {
                                for ( int k = 0; k < results.results()[ j ].output_arguments().size(); k++ ) {
                                    mtx.lock();
                                    cout << "Back in test_asynchronous, let's display the result : ";
                                    Variant variant = results.results()[ j ].output_arguments()[ k ];
                                    Printer printer;
                                    cout << printer.PrintVariant ( variant ) << endl;
                                    mtx.unlock();
                                }

                            }
                        }
                    } else
                        cerr << "no data in method_fut.get()" << endl;

            } );
        }
    } catch ( std::exception & ex ) {
        cout << "Exception in test_asynchronous_method() : " << ex.what() << endl;
    } catch ( ... ) {
        cout << "Unknown exception in test_asynchronous_method()" << endl;
    }
}

void test_synchronous_method_timeout ( DDS::DomainParticipant_var participant )
{
    RPC::ClientParams clientParams;
    // 100us timeout
    DDS::Duration_t maxWait;
    maxWait.sec = 0;
    maxWait.nanosec = 100000;
    clientParams.SetTimeoutForSynchronousCalls ( maxWait );
    clientParams.domain_participant ( participant );
    auto client = new METHOD::Client ( clientParams );

    test_synchronous_method ( *client );

    return;
}

ATTRIBUTE::Client * test_asynchronous_attribute_read ( DDS::DomainParticipant_var participant )
{
    try {
        RPC::ClientParams clientParams;
        clientParams.domain_participant ( participant );
        auto client = new ATTRIBUTE::Client ( clientParams );

        //Let's prepare the request
        ResponseHeader responseHeader;
        string serverId = "TestServer";

        NodeId idOfNodeToRead;
        idOfNodeToRead.identifier_type().string_id ( "Int64TestNode" );
        idOfNodeToRead.namespace_index( 1 );

        ReadValueId readValueId;
        readValueId.attribute_id( 13 ); //(UA_ATTRIBUTEID_VALUE)
        readValueId.node_id( idOfNodeToRead );
        vector<ReadValueId> nodesToRead;

        nodesToRead.push_back ( readValueId );

        // 2 minutes
        Duration duration { 120000 };

        auto ttr = TimestampsToReturn::BOTH_TIMESTAMPS_TO_RETURN;

        client->
        read_async ( serverId, duration, ttr, nodesToRead )
        .then ( [] ( boost::future< ATTRIBUTE::Attribute_read_Out> && read_fut ) {
            auto results = read_fut.get();

            cout << "Back in test_asynchronous_attribute_read, let's display the results : ";

            for ( int i = 0; i < results.results().size(); i++ ) {
                cout << "Node index in request = " << i << ", Value = ";
                Printer printer;
                cout << printer.PrintVariant ( results.results()[ i ].value() ) << endl;
            }
        } );

        cout << "Done with asynchronously sending request in main()" << endl;
        std::this_thread::sleep_for ( std::chrono::seconds ( 2 ) );
        return client;
    } catch ( std::exception & ex ) {
        cout << "Exception in test_asynchronous_read() : " << ex.what() << endl;
    } catch ( ... ) {
        cout << "Unknown exception in test_asynchronous_read()" << endl;
    }

    return nullptr;
}

ATTRIBUTE::Client * test_asynchronous_attribute_write ( DDS::DomainParticipant_var participant )
{
    try {
        RPC::ClientParams clientParams;
        clientParams.domain_participant ( participant );
        auto client = new ATTRIBUTE::Client ( clientParams );

        //Let's prepare the request
        ResponseHeader responseHeader;
        string serverId = "TestServer";

        NodeId idOfNodeToWrite;
        idOfNodeToWrite.identifier_type().string_id ( "UInt16TestNode" );
        idOfNodeToWrite.namespace_index( 1 );

        CORBA::UShort input = 22;
        Variant inputVariant;
        UInt32Seq dimDelta;
        dimDelta.reserve( 1 );
        dimDelta.push_back( 1 );
        inputVariant.array_dimensions( dimDelta );
        VariantValueSeq alone;
        alone.reserve( 1 );
        VariantValue variantValue;
        variantValue.short_value( input );
        alone.push_back( variantValue );
        inputVariant.value( alone );

        ATTRIBUTE::WriteValue writeValue;
        writeValue.attribute_id( 13 ); //(UA_ATTRIBUTEID_VALUE)
        writeValue.node_id( idOfNodeToWrite );
        writeValue.value().value( inputVariant );

        vector<ATTRIBUTE::WriteValue> nodesToWrite;
        nodesToWrite.push_back ( writeValue );

        Printer printer;
        cout << "Writing node : " << printer.PrintNodeId ( idOfNodeToWrite ) << endl;

        client->
        write_async ( serverId, nodesToWrite )
        .then ( [] ( future<ATTRIBUTE::Attribute_write_Out> && write_fut ) {
            auto results = write_fut.get();

            cout << "Back in test_asynchronous_attribute_write, let's display the results : ";

            for ( int i = 0; i < results.results().size(); i++ ) {
                cout << "Status Code of writing node " << i << " : ";
                if ( results.results()[ i ] == 0 )
                    cout << "GOOD" << endl;
                else
                    cout << "BAD" << endl;
            }
        } );

        cout << "Done with asynchronously sending request in main()" << endl;
        std::this_thread::sleep_for ( std::chrono::seconds ( 2 ) );
        return client;
    } catch ( std::exception & ex ) {
        cout << "Exception in test_asynchronous_write() : " << ex.what() << endl;
    } catch ( ... ) {
        cout << "Unknown exception in test_asynchronous_write()" << endl;
    }

    return nullptr;
}

void test_synchronous_view_translate_browse_path_to_nodeid ( VIEW::Client * client )
{
    ResponseHeader responseHeader;
    string serverId = "TestServer";

    cout << "Translating BrowsePath to NodeId..." << endl;

    vector<BrowsePath> browsePaths;
    BrowsePath browsePath1, browsePath2;
    NodeId startingNode1;
    startingNode1.namespace_index( 0 );
    startingNode1.identifier_type().numeric_id ( 85 ); // Objects folder
    browsePath1.starting_node( startingNode1 );
    RelativePath relativePath1;
    relativePath1.elements().reserve( 3 );
    RelativePathElement element1, element2, element3;
    NodeId NS0ID_ORGANIZES;
    NS0ID_ORGANIZES.namespace_index( 0 );
    NS0ID_ORGANIZES.identifier_type().numeric_id ( 35 );
    NodeId NS0ID_HASCOMPONENT;
    NS0ID_HASCOMPONENT.namespace_index( 0 );
    NS0ID_HASCOMPONENT.identifier_type().numeric_id ( 47 );
    element1.reference_type_id( NS0ID_ORGANIZES );
    element2.reference_type_id( NS0ID_HASCOMPONENT );
    element3.reference_type_id( NS0ID_HASCOMPONENT );
    QualifiedName targetName1, targetName2, targetName3;
    targetName1.namespace_index( 0 );
    targetName2.namespace_index( 0 );
    targetName3.namespace_index( 0 );
    targetName1.name( "Server" );
    targetName2.name( "ServerStatus" );
    targetName3.name( "State" );
    element1.include_subtypes( false );
    element1.is_inverse( false );
    element1.target_name( targetName1 );
    element2.include_subtypes( false );
    element2.is_inverse( false );
    element2.target_name( targetName2 );
    element3.include_subtypes( false );
    element3.is_inverse( false );
    element3.target_name( targetName3 );
    relativePath1.elements().push_back( element1 );
    relativePath1.elements().push_back( element2 );
    relativePath1.elements().push_back( element3 );
    browsePath1.relative_path( relativePath1 );
    browsePaths.push_back ( browsePath1 );

    vector<BrowsePathResult> results;
    vector<DiagnosticInfo> diagnosticInfos;

    client->translate_browse_paths_to_node_ids ( responseHeader, serverId, results, diagnosticInfos, browsePaths );

    mtx.lock();

    cout << "Back in test_synchronous_view_translate_browse_path_to_nodeid, service return code = " << responseHeader.service_result() << "; let's display the results : " << endl;
    cout << "results.size() = " << results.size() << "; ";
    if ( results.size() > 0 ) {
        for ( int i = 0; i < results.size(); i++ ) {
            cout << "Result " << i << " status code = " << results[ i ].status_code() << ", nodes size = " << results[ i ].targets().size() << " : ";
            for ( int j = 0; j < results[ i ].targets().size(); j++ ) {
                cout << results[ i ].targets()[ j ].target_id().node_id().identifier_type().numeric_id() << " / ";
            }
            cout << endl;
        }
    }

    mtx.unlock();
}

void test_synchronous_view_register_nodes ( VIEW::Client * client )
{
    ResponseHeader responseHeader;
    string serverId = "TestServer";

    cout << "Registering nodes..." << endl;

    vector<NodeId> registeredNodes;
    vector<NodeId> nodesToRegister;
    NodeId id1, id2;
    id1.namespace_index( 1 );
    id2.namespace_index( 1 );
    id1.identifier_type().string_id ( "ByteMatrixTestNode" );
    id2.identifier_type().string_id ( "ExpandedNodeIdArrayTestNode" );
    nodesToRegister.push_back ( id1 );
    nodesToRegister.push_back ( id2 );

    client->register_nodes ( responseHeader, serverId, registeredNodes, nodesToRegister );

    mtx.lock();

    cout << "Back in test_synchronous_view_register_nodes, service return code = " << responseHeader.service_result() << "; let's display the results : " << endl;
    cout << "registeredNodes.size() = " << registeredNodes.size() << "; ";
    if ( registeredNodes.size() > 0 ) {
        for ( int i = 0; i < registeredNodes.size(); i++ ) {
            cout << "ns = " << registeredNodes[ i ].namespace_index();
            cout << "; s = " << registeredNodes[ i ].identifier_type().string_id() << " / ";
        }
    }
    cout << endl;

    mtx.unlock();
}


void test_synchronous_view_unregister_nodes ( VIEW::Client * client )
{
    ResponseHeader responseHeader;
    string serverId = "TestServer";

    cout << "Unregistering nodes..." << endl;

    vector<NodeId> nodesToUnregister;
    NodeId id1, id2;
    id1.namespace_index( 1 );
    id2.namespace_index( 1 );
    id1.identifier_type().string_id ( "ByteMatrixTestNode" );
    id2.identifier_type().string_id ( "ExpandedNodeIdArrayTestNode" );
    nodesToUnregister.push_back ( id1 );
    nodesToUnregister.push_back ( id2 );

    client->unregister_nodes ( responseHeader, serverId, nodesToUnregister );

    mtx.lock();

    cout << "Back in test_synchronous_view_unregister_nodes, service return code : " << responseHeader.service_result() << endl;

    mtx.unlock();
}

VIEW::Client * test_synchronous_view_browse_next ( VIEW::Client * client )
{
    ResponseHeader responseHeader;
    string serverId = "TestServer";

    cout << "Browsing next in Server folder..." << endl;

    // Should do browse before browse_next.
    ViewDescription view;
    view.view_id().namespace_index( 0 );
    view.view_id().identifier_type().numeric_id ( 0 );

    Counter requested_max_references_per_node { 15 };

    std::vector<BrowseDescription> browse_descriptions;
    BrowseDescription browse_description;
    NodeId idOfNodeToBrowse;
    idOfNodeToBrowse.identifier_type().numeric_id ( 2253 ); // Server Folder
    idOfNodeToBrowse.namespace_index( 0 );
    browse_description.node_id( idOfNodeToBrowse );
    browse_description.result_mask( 63 ); // UA_BROWSERESULTMASK_ALL, return everything
    browse_description.browse_direction( BrowseDirection::FORWARD_BROWSE_DIRECTION ); // UA_BROWSEDIRECTION_BOTH
    browse_description.include_subtypes( true );
    browse_description.node_class_mask( 0 );
    browse_description.reference_type_id().namespace_index( 0 );
    browse_description.reference_type_id().identifier_type().numeric_id ( 0 );

    browse_descriptions.push_back ( browse_description );

    vector<BrowseResult> browseResults;
    vector<DiagnosticInfo> browse_diagnostic_infos;

    client->browse ( responseHeader, serverId, browseResults, browse_diagnostic_infos, view, requested_max_references_per_node, browse_descriptions );

    // Then we can browse_next
    if ( browseResults.size() == 0 ) {
        cout << "second browse did not work" << endl;
        return client;
    }

    vector<ContinuationPoint> continuationPoints;
    auto continuationPoint = browseResults[ 0 ].continuation_point();
    continuationPoints.push_back ( continuationPoint );

    cout << "\tCP: ";
    for ( int i = 0; i < continuationPoint.size(); i++ ) {
        cout << continuationPoint[ i ];
    }
    cout << endl;

    bool releaseContinuationPoints = false;
    vector<BrowseResult> browseNextResults;
    vector<DiagnosticInfo> browse_next_diagnostic_infos;

//    cout << "Browse OK, continuation point : " << browseResults[ 0 ].continuation_point.get_buffer() << endl;
    //sleep(1);
    client->browse_next ( responseHeader, serverId, browseNextResults, browse_next_diagnostic_infos, releaseContinuationPoints, continuationPoints );

    mtx.lock();

    cout << "Back in test_synchronous_view_browse_next, let's display the results : " << endl;
    cout << "resultSize = " << browseNextResults.size() << endl;
    if ( browseNextResults.size() > 0 ) {
        for ( int i = 0; i < browseNextResults.size(); i++ ) {
            if ( browseNextResults[ 0 ].references().size() == 0 )
                cout << "problem with the result of browse_next request" << endl;
            cout << "referencesSize = " << browseNextResults[ 0 ].references().size() << endl;
            for ( int j = 0; j < browseNextResults[ i ].references().size(); j++ ) {
                if ( browseNextResults[ i ].references()[ j ].node_id().node_id().identifier_type()._d() == NodeIdentifierKind::NUMERIC_NODE_ID ) {
                    cout << "Namespace: " << browseNextResults[ i ].references()[ j ].node_id().node_id().namespace_index() << ", nodeId: " << browseNextResults[ i ].references()[ j ].node_id().node_id().identifier_type().numeric_id();
                } else if ( browseNextResults[ i ].references()[ j ].node_id().node_id().identifier_type()._d() == NodeIdentifierKind::STRING_NODE_ID ) {
                    cout << "Namespace: " << browseNextResults[ i ].references()[ j ].node_id().node_id().namespace_index() << ", nodeId: " << browseNextResults[ i ].references()[ j ].node_id().node_id().identifier_type().string_id();
                }
                cout << ", browseName: "  << browseNextResults[ i ].references()[ j ].browse_name().name();
                cout << ", displayName: " << browseNextResults[ i ].references()[ j ].display_name().text() << endl;
            }
        }
    }

    mtx.unlock();

    return client;
}

void test_synchronous_view_browse ( VIEW::Client * client )
{
    ResponseHeader responseHeader;
    string serverId = "TestServer";

    cout << "Browsing nodes in objects folder:" << endl;

    ViewDescription view;
    view.view_id().namespace_index( 0 );
    view.view_id().identifier_type().numeric_id ( 0 );

    Counter requested_max_references_per_node {0};

    // TODO : find a way to set defaults for BrowseDescription
    vector<BrowseDescription> browse_descriptions;
    BrowseDescription browse_description;
    NodeId idOfNodeToBrowse;
    idOfNodeToBrowse.identifier_type().numeric_id ( 85 ); // Objects Folder
    idOfNodeToBrowse.namespace_index( 0 );
    browse_description.node_id( idOfNodeToBrowse );
    browse_description.result_mask( 63 ); // UA_BROWSERESULTMASK_ALL, return everything
    browse_description.browse_direction( BrowseDirection::BOTH_BROWSE_DIRECTION ); // UA_BROWSEDIRECTION_BOTH
    browse_description.include_subtypes( true );
    browse_description.node_class_mask( 0 );
    browse_description.reference_type_id().namespace_index( 0 );
    browse_description.reference_type_id().identifier_type().numeric_id ( 0 );

    browse_descriptions.push_back ( browse_description );

    vector<BrowseResult> results;
    vector<DiagnosticInfo> diagnostic_infos;

    client->browse ( responseHeader, serverId, results, diagnostic_infos, view, requested_max_references_per_node, browse_descriptions );

    mtx.lock();

    cout << "Back in test_synchronous_view_browse, let's display the results : " << endl;
    cout << "resultSize = " << results.size() << endl;
    if ( results.size() > 0 ) {
        cout << "referencesSize = " << results[ 0 ].references().size() << endl;
        for ( int i = 0; i < results.size(); i++ ) {
            for ( int j = 0; j < results[ i ].references().size(); j++ ) {
                if ( results[ i ].references()[ j ].node_id().node_id().identifier_type()._d() == NodeIdentifierKind::NUMERIC_NODE_ID ) {
                    cout << "Namespace: " << results[ i ].references()[ j ].node_id().node_id().namespace_index() << ", nodeId: " << results[ i ].references()[ j ].node_id().node_id().identifier_type().numeric_id();
                } else if ( results[ i ].references()[ j ].node_id().node_id().identifier_type()._d() == NodeIdentifierKind::STRING_NODE_ID ) {
                    cout << "Namespace: " << results[ i ].references()[ j ].node_id().node_id().namespace_index() << ", nodeId: " << results[ i ].references()[ j ].node_id().node_id().identifier_type().string_id();
                }
                cout << ", browseName: "  << results[ i ].references()[ j ].browse_name().name();
                cout << ", displayName: " << results[ i ].references()[ j ].display_name().text() << endl;
            }

            //cout << "\tContinuation Point = " << results[ i ].continuation_point.get_buffer() << endl;
        }
    }

    mtx.unlock();
}

ATTRIBUTE::Client * test_synchronous_attribute_read ( DDS::DomainParticipant_var participant )
{
    ResponseHeader responseHeader;
    string serverId = "TestServer";
    RPC::ClientParams clientParams;
    clientParams.domain_participant ( participant );
    auto client = new ATTRIBUTE::Client ( clientParams );

    NodeId idOfNodeToRead;
    idOfNodeToRead.identifier_type().string_id ( "UInt64TestNode" );
    idOfNodeToRead.namespace_index( 1 );

    ReadValueId readValueId;
    readValueId.attribute_id( 13 ); //(UA_ATTRIBUTEID_VALUE)
    readValueId.node_id( idOfNodeToRead );
    vector<ReadValueId> nodesToRead;

    nodesToRead.push_back ( readValueId );

    // 2 minutes
    Duration maxAge { 120000 };
    auto ttr = TimestampsToReturn::BOTH_TIMESTAMPS_TO_RETURN;

    vector<DataValue> results;
    vector<DiagnosticInfo> diagnostic_infos;

    client->read ( responseHeader, serverId, results, diagnostic_infos, maxAge, ttr, nodesToRead );

    if ( responseHeader.service_result() == DDS::RETCODE_TIMEOUT ) {
        cout << "main() : Timeout reached while performing synchronous call" << endl;
    }

    cout << "Back in test_synchronous_attribute_read, let's display the result : " << endl;
    Printer printer;
    for ( int i = 0; i < results.size(); i++ ) {
        cout << std::dec << "Node " << printer.PrintNodeId ( nodesToRead[ i ].node_id() ) << ", DataValue : " << printer.PrintDataValue ( results[ i ] ) << endl;
    }

    return client;
}

ATTRIBUTE::Client * test_synchronous_attribute_write ( DDS::DomainParticipant_var participant )
{
    ResponseHeader responseHeader;
    string serverId = "TestServer";
    RPC::ClientParams clientParams;
    clientParams.domain_participant ( participant );
    auto client = new ATTRIBUTE::Client ( clientParams );

    NodeId idOfNodeToWrite;
    idOfNodeToWrite.identifier_type().string_id ( "Int32TestNode" );
    idOfNodeToWrite.namespace_index( 1 );

    CORBA::Long input = 187;
    Variant inputVariant;
    UInt32Seq dimDelta;
    dimDelta.reserve( 1 );
    dimDelta.push_back( 1 );
    inputVariant.array_dimensions( dimDelta );
    VariantValueSeq alone;
    alone.reserve( 1 );
    VariantValue variantValue;
    variantValue.int32_value( input );
    alone.push_back( variantValue );
    inputVariant.value( alone );

    ATTRIBUTE::WriteValue writeValue;
    writeValue.attribute_id( 13 ); //(UA_ATTRIBUTEID_VALUE)
    writeValue.node_id( idOfNodeToWrite );
    writeValue.value().value( inputVariant );
    writeValue.value().server_pico_sec( 0 );
    writeValue.value().server_timestamp( 0 );
    // A améliorer
    writeValue.value().source_timestamp( 0 );
    writeValue.value().source_pico_sec( 0 );
    writeValue.value().status( 0 );

    vector<ATTRIBUTE::WriteValue> nodesToWrite;
    nodesToWrite.push_back ( writeValue );

    vector<StatusCode> results;
    vector<DiagnosticInfo> diagnostic_infos;

    client->write ( responseHeader, serverId, results, diagnostic_infos, nodesToWrite );

    if ( responseHeader.service_result() == DDS::RETCODE_TIMEOUT ) {
        cout << "main() : Timeout reached while performing synchronous call" << endl;
    }

    cout << "Back in test_synchronous_attribute_write, let's display the result : " << endl;
    Printer printer;
    for ( int i = 0; i < results.size(); i++ ) {
        if ( results[i] != 0 ) {
            cout << "Node to write : " << printer.PrintNodeId ( nodesToWrite[ i ].node_id() ) << ", result = BAD" << endl;
        } else if ( results[ i ] == 0 ) {
            cout << "Node to write : " << printer.PrintNodeId ( nodesToWrite[ i ].node_id() ) << ", result = GOOD" << endl;
        }
    }

    return client;
}

int ACE_TMAIN ( int argc, ACE_TCHAR *argv[] )
{
    ACE_LOG_MSG->priority_mask ( LM_DEBUG|LM_NOTICE|LM_WARNING|LM_ERROR|LM_CRITICAL|LM_ALERT|LM_EMERGENCY, ACE_Log_Msg::PROCESS );
    signal ( SIGINT, stopHandler );

    try {
        // Initialize DomainParticipantFactory
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs ( argc, argv );

        DDS::DomainParticipantQos_var dp_qos = new DDS::DomainParticipantQos;
        auto ret = dpf->get_default_participant_qos ( dp_qos );
        if ( ret != DDS::RETCODE_OK ) {
            cerr << "Domain participant factory could not get default participant qos..." << endl;
        }

        // Optional
        // For use with RtpsRelay in docker, see Object Computing Inc. documentation
        /*
        DDS::PropertySeq & props = dp_qos->property.value;
        const DDS::Property_t prop = { "OpenDDS.RtpsRelay.Groups", "Gateway", true };
        const unsigned int len = props.length();
        props.length ( len + 1 );
        props[ len ] = prop;
        */

#ifdef OPENDDS_SECURITY
        DDS::PropertySeq & props = dp_qos->property.value;

        setSecureEnvironment ( props );
#endif

        ::DDS::DomainParticipant_var participant = dpf->create_participant ( 4,
                dp_qos,
                0,
                DEFAULT_STATUS_MASK );

        if ( !participant ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - create_participant failed!\n" ) ), -1 );
        }

        // Register Types
        SimpleDataTypeTypeSupport_var simpleDataType = new SimpleDataTypeTypeSupportImpl;

        if ( simpleDataType->register_type ( participant, "SimpleDataType" ) != DDS::RETCODE_OK ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - register_type failed!\n" ) ), -1 );
        } else {
            cout << "SimpleDataType registered" << endl;
        }

        MessageEventTypeSupport_var messageEventType = new MessageEventTypeSupportImpl;

        if ( messageEventType->register_type ( participant, "MessageEvent" ) != DDS::RETCODE_OK ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - register_type failed!\n" ) ), -1 );
        } else {
            cout << "MessageEvent registered" << endl;
        }

        DDS::Topic_var constantsTopic = participant->create_topic( "GatewayConstantTest",
                                        "SimpleDataType",
                                        TOPIC_QOS_DEFAULT,
                                        0,
                                        OpenDDS::DCPS::DEFAULT_STATUS_MASK );
        
        if ( !constantsTopic ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - create_topic failed!\n" ) ), -1 );
        } else {
            cout << "GatewayConstantTest topic created" << endl;
        }

        DDS::Topic_var scalarTopic = participant->create_topic ( "SimpleScalarTest",
                               "SimpleDataType",
                               TOPIC_QOS_DEFAULT,
                               0,
                               OpenDDS::DCPS::DEFAULT_STATUS_MASK );

        if ( !scalarTopic ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - create_topic failed!\n" ) ), -1 );
        } else {
            cout << "SimpleScalarTest topic created" << endl;
        }

        DDS::Topic_var eventTopic = participant->create_topic ( "SimpleEventTest",
                                "MessageEvent",
                                TOPIC_QOS_DEFAULT,
                                0,
                                OpenDDS::DCPS::DEFAULT_STATUS_MASK );

        if ( !eventTopic ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - find_topic failed!\n" ) ), -1 );
        } else {
            cout << "SimpleEventTest topic created" << endl;
        }

        // Create Subscriber
        DDS::Subscriber_var subscriber =
            participant->create_subscriber ( SUBSCRIBER_QOS_DEFAULT,
                                             0,
                                             OpenDDS::DCPS::DEFAULT_STATUS_MASK );

        if ( !subscriber ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - create_subscriber failed!\n" ) ), -1 );
        }

        // Create DataReaders
        DDS::DataReaderListener_var constantsListener ( new SimpleDataTypeDataReaderListenerImpl );

        DDS::DataReader_var constantsReader =
            subscriber->create_datareader ( constantsTopic,
                                            DATAREADER_QOS_DEFAULT,
                                            constantsListener,
                                            OpenDDS::DCPS::DEFAULT_STATUS_MASK );

        if ( !constantsReader ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - create_datareader failed!\n" ) ), -1 );
        }
        
        DDS::DataReaderListener_var scalarListener ( new SimpleDataTypeDataReaderListenerImpl );

        DDS::DataReader_var scalarReader =
            subscriber->create_datareader ( scalarTopic,
                                            DATAREADER_QOS_DEFAULT,
                                            scalarListener,
                                            OpenDDS::DCPS::DEFAULT_STATUS_MASK );

        if ( !scalarReader ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - create_datareader failed!\n" ) ), -1 );
        }

        DDS::DataReaderListener_var eventListener ( new MessageEventDataReaderListenerImpl );

        DDS::DataReader_var eventReader =
            subscriber->create_datareader ( eventTopic,
                                            DATAREADER_QOS_DEFAULT,
                                            eventListener,
                                            OpenDDS::DCPS::DEFAULT_STATUS_MASK );

        if ( !eventReader ) {
            ACE_ERROR_RETURN ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: main() - create_datareader failed!\n" ) ), -1 );
        }
        
        cout << "DDS initialized" << endl;

        // Just there for E2E validation testing. The time for the opcserver and gateway to start
        sleep( 50 );

        // METHOD Test
        RPC::ClientParams clientParams;
        // 3s timeout
        DDS::Duration_t maxWait;
        maxWait.sec = 3;
        maxWait.nanosec = 0;
        clientParams.SetTimeoutForSynchronousCalls ( maxWait );
        clientParams.domain_participant ( participant );
        METHOD::Client client ( clientParams );
        
        test_synchronous_method ( client );

        test_asynchronous_method ( client );

        // Test Method synchronous call timeout triggering
        test_synchronous_method_timeout ( participant );

        test_asynchronous_attribute_read ( participant );

        test_synchronous_attribute_read ( participant );

        test_asynchronous_attribute_write ( participant );

        test_synchronous_attribute_write ( participant );

        RPC::ClientParams viewClientParams;
        clientParams.domain_participant ( participant );
        VIEW::Client viewClient ( clientParams );
        
        test_synchronous_view_browse ( &viewClient );

        test_synchronous_view_browse_next ( &viewClient );

        test_synchronous_view_translate_browse_path_to_nodeid ( &viewClient );

        test_synchronous_view_register_nodes ( &viewClient );

        test_synchronous_view_unregister_nodes ( &viewClient );

        while ( running ) {
            sleep ( 1000 );
        }

        // Clean-up!
        participant->delete_contained_entities();
        dpf->delete_participant ( participant );

        TheServiceParticipant->shutdown();

        ACE_OS::exit(0);
    } catch ( const CORBA::Exception& e ) {
        e._tao_print_exception ( "Exception caught in main():" );
        return -1;
    }
}
