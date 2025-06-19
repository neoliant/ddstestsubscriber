/* Copyright (C) neoliant.com - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary
 * Written by neoliant - info@neoliant.com, February 2021
 */

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include "DataReaderListenerImpl.h"
#include <SimpleDataTypeTypeSupportC.h>
#include <SimpleDataTypeTypeSupportImpl.h>
#include "Printer.h"

#include <iostream>

void
SimpleDataTypeDataReaderListenerImpl::on_requested_deadline_missed (
    DDS::DataReader_ptr /*reader*/,
    const DDS::RequestedDeadlineMissedStatus& /*status*/ )
{
}

void
SimpleDataTypeDataReaderListenerImpl::on_requested_incompatible_qos (
    DDS::DataReader_ptr /*reader*/,
    const DDS::RequestedIncompatibleQosStatus& /*status*/ )
{
}

void
SimpleDataTypeDataReaderListenerImpl::on_sample_rejected (
    DDS::DataReader_ptr /*reader*/,
    const DDS::SampleRejectedStatus& /*status*/ )
{
}

void
SimpleDataTypeDataReaderListenerImpl::on_liveliness_changed (
    DDS::DataReader_ptr /*reader*/,
    const DDS::LivelinessChangedStatus& /*status*/ )
{
}

void
SimpleDataTypeDataReaderListenerImpl::on_data_available ( DDS::DataReader_ptr reader )
{
    SimpleDataTypeDataReader_var reader_dp = SimpleDataTypeDataReader::_narrow ( reader );

    if ( !reader_dp ) {
        ACE_ERROR ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: on_data_available() - _narrow failed!\n" ) ) );
        ACE_OS::exit ( -1 );
    }

    SimpleDataType simpleDataType;
    DDS::SampleInfo info;

    DDS::ReturnCode_t error = reader_dp->take_next_sample ( simpleDataType, info );

    if ( error == DDS::RETCODE_OK ) {
        std::cout << "SampleInfo.sample_rank = " << info.sample_rank << std::endl;
        std::cout << "SampleInfo.instance_state = " << info.instance_state << std::endl;

        Printer printer;
        if ( info.valid_data ) {
            std::cout << "SimpleDataType.testBoolean = " << simpleDataType.testBoolean()     << std::endl
                      << "SimpleDataType.testUInt8 = "   << (int) simpleDataType.testUInt8()  << std::endl
                      << "SimpleDataType.testInt8 = "    << (int) simpleDataType.testInt8()   << std::endl
                      << "SimpleDataType.testUInt16 = "  << simpleDataType.testUInt16()      << std::endl
                      << "SimpleDataType.testInt16 = "   << simpleDataType.testInt16()       << std::endl
                      << "SimpleDataType.testUInt32 = "  << simpleDataType.testUInt32()      << std::endl
                      << "SimpleDataType.testInt32 = "   << simpleDataType.testInt32()       << std::endl
                      << "SimpleDataType.testUInt64 = "  << simpleDataType.testUInt64()      << std::endl
                      << "SimpleDataType.testInt64 = "   << simpleDataType.testInt64()       << std::endl
                      << "SimpleDataType.testFloat = "   << printer.PrintFloat( simpleDataType.testFloat() )       << std::endl
                      << "SimpleDataType.testDouble = "  << printer.PrintDouble( simpleDataType.testDouble() )      << std::endl
                      << "SimpleDataType.testString = "  << simpleDataType.testString().data() << std::endl
                      << "SimpleDataType.testDateTime = "           << printer.PrintDateTime( simpleDataType.testDateTime() )                 << std::endl
                      << "SimpleDataType.testGuid = "               << printer.PrintGuid( simpleDataType.testGuid() )                         << std::endl
                      << "SimpleDataType.testByteString = "         << printer.PrintByteString( simpleDataType.testByteString() )             << std::endl
                      << "SimpleDataType.testXmlElement = "         << simpleDataType.testXmlElement().data()                                   << std::endl
                      << "SimpleDataType.testNodeId = "             << printer.PrintNodeId( simpleDataType.testNodeId() )                     << std::endl
                      << "SimpleDataType.testExpandedNodeId = "     << printer.PrintExpandedNodeId( simpleDataType.testExpandedNodeId() )     << std::endl
                      << "SimpleDataType.testStatusCode = "         << printer.PrintStatusCode( simpleDataType.testStatusCode() )             << std::endl
                      << "SimpleDataType.testQualifiedName = "      << printer.PrintQualifiedName( simpleDataType.testQualifiedName() )       << std::endl
                      << "SimpleDataType.testLocalizedText = "      << printer.PrintLocalizedText( simpleDataType.testLocalizedText() )       << std::endl
                      << "SimpleDataType.testExtensionObject = "    << printer.PrintExtensionObject( simpleDataType.testExtensionObject() )   << std::endl;

            std::cout << "SimpleDataType.testBooleanArray = " << printer.PrintArray( simpleDataType.testBooleanArray() )           << std::endl
                      << "SimpleDataType.testUInt8Array = "   << printer.PrintArray<ByteArray>( simpleDataType.testByteArray() )               << std::endl
                      << "SimpleDataType.testInt8Array = "    << printer.PrintArray<SByteArray>( simpleDataType.testSByteArray() )                 << std::endl
                      << "SimpleDataType.testUInt16Array = "  << printer.PrintArray<UInt16Array>( simpleDataType.testUInt16Array() )             << std::endl
                      << "SimpleDataType.testInt16Array = "   << printer.PrintArray<Int16Array>( simpleDataType.testInt16Array() )       << std::endl
                      << "SimpleDataType.testUInt32Array = "  << printer.PrintArray<UInt32Array>( simpleDataType.testUInt32Array() )      << std::endl
                      << "SimpleDataType.testInt32Array = "   << printer.PrintArray<Int32Array>( simpleDataType.testInt32Array() )    << std::endl
                      << "SimpleDataType.testUInt64Array = "  << printer.PrintArray<UInt64Array>( simpleDataType.testUInt64Array() )      << std::endl
                      << "SimpleDataType.testInt64Array = "   << printer.PrintArray<Int64Array>( simpleDataType.testInt64Array() )       << std::endl
                      << "SimpleDataType.testFloatArray = "   << printer.PrintArray<FloatArray>( simpleDataType.testFloatArray() )       << std::endl
                      << "SimpleDataType.testDoubleArray = "  << printer.PrintArray<DoubleArray>( simpleDataType.testDoubleArray() )      << std::endl
                      << "SimpleDataType.testStringArray = "  << printer.PrintStringArray( simpleDataType.testStringArray() ) << std::endl
                      << "SimpleDataType.testDateTimeArray = "           << printer.PrintDateTimeArray( simpleDataType.testDateTimeArray() )                 << std::endl
                      << "SimpleDataType.testGuidArray = "               << printer.PrintGuidArray( simpleDataType.testGuidArray() )                         << std::endl
                      << "SimpleDataType.testByteStringArray = "         << printer.PrintByteStringArray( simpleDataType.testByteStringArray() )             << std::endl
                      << "SimpleDataType.testXmlElementArray = "         << printer.PrintXmlElementArray( simpleDataType.testXmlElementArray() )                                   << std::endl
                      << "SimpleDataType.testNodeIdArray = "             << printer.PrintNodeIdArray( simpleDataType.testNodeIdArray() )                     << std::endl
                      << "SimpleDataType.testExpandedNodeIdArray = "     << printer.PrintExpandedNodeIdArray( simpleDataType.testExpandedNodeIdArray() )     << std::endl
                      << "SimpleDataType.testStatusCodeArray = "         << printer.PrintStatusCodeArray( simpleDataType.testStatusCodeArray() )             << std::endl
                      << "SimpleDataType.testQualifiedNameArray = "      << printer.PrintQualifiedNameArray( simpleDataType.testQualifiedNameArray() )       << std::endl
                      << "SimpleDataType.testLocalizedTextArray = "      << printer.PrintLocalizedTextArray( simpleDataType.testLocalizedTextArray() )       << std::endl
                      << "SimpleDataType.testExtensionObjectArray = "    << printer.PrintExtensionObjectArray( simpleDataType.testExtensionObjectArray() )   << std::endl;

            std::cout << "SimpleDataType.testBooleanMatrix = " << printer.PrintMatrix( simpleDataType.testBooleanMatrix() )     << std::endl
                      << "SimpleDataType.testUInt8Matrix = "   << printer.PrintMatrix( simpleDataType.testByteMatrix() )  << std::endl
                      << "SimpleDataType.testInt8Matrix = "    << printer.PrintMatrix( simpleDataType.testSByteMatrix() )   << std::endl
                      << "SimpleDataType.testUInt16Matrix = "  << printer.PrintMatrix( simpleDataType.testUInt16Matrix() )      << std::endl
                      << "SimpleDataType.testInt16Matrix = "   << printer.PrintMatrix( simpleDataType.testInt16Matrix() )      << std::endl
                      << "SimpleDataType.testUInt32Matrix = "  << printer.PrintMatrix( simpleDataType.testUInt32Matrix() )     << std::endl
                      << "SimpleDataType.testInt32Matrix = "   << printer.PrintMatrix( simpleDataType.testInt32Matrix() )      << std::endl
                      << "SimpleDataType.testUInt64Matrix = "  << printer.PrintMatrix( simpleDataType.testUInt64Matrix() )     << std::endl
                      << "SimpleDataType.testInt64Matrix = "   << printer.PrintMatrix( simpleDataType.testInt64Matrix() )      << std::endl
                      << "SimpleDataType.testFloatMatrix = "   << printer.PrintMatrix( simpleDataType.testFloatMatrix() )      << std::endl
                      << "SimpleDataType.testDoubleMatrix = "  << printer.PrintMatrix( simpleDataType.testDoubleMatrix() )     << std::endl
                      << "SimpleDataType.testStringMatrix = "  << printer.PrintStringMatrix( simpleDataType.testStringMatrix() ) << std::endl
                      << "SimpleDataType.testDateTimeMatrix = "           << printer.PrintDateTimeMatrix( simpleDataType.testDateTimeMatrix() )                 << std::endl
                      << "SimpleDataType.testGuidMatrix = "               << printer.PrintGuidMatrix( simpleDataType.testGuidMatrix() )                         << std::endl
                      << "SimpleDataType.testByteStringMatrix = "         << printer.PrintByteStringMatrix( simpleDataType.testByteStringMatrix() )             << std::endl
                      << "SimpleDataType.testXmlElementMatrix = "         << printer.PrintXmlElementMatrix( simpleDataType.testXmlElementMatrix() )             << std::endl
                      << "SimpleDataType.testNodeIdMatrix = "             << printer.PrintNodeIdMatrix( simpleDataType.testNodeIdMatrix() )                     << std::endl
                      << "SimpleDataType.testExpandedNodeIdMatrix = "     << printer.PrintExpandedNodeIdMatrix( simpleDataType.testExpandedNodeIdMatrix() )     << std::endl
                      << "SimpleDataType.testStatusCodeMatrix = "         << printer.PrintMatrix( simpleDataType.testStatusCodeMatrix() )             << std::endl
                      << "SimpleDataType.testQualifiedNameMatrix = "      << printer.PrintQualifiedNameMatrix( simpleDataType.testQualifiedNameMatrix() )       << std::endl
                      << "SimpleDataType.testLocalizedTextMatrix = "      << printer.PrintLocalizedTextMatrix( simpleDataType.testLocalizedTextMatrix() )       << std::endl
                      << "SimpleDataType.testExtensionObjectMatrix = "    << printer.PrintExtensionObjectMatrix( simpleDataType.testExtensionObjectMatrix() )   << std::endl;
        }
    } else {
        ACE_ERROR ( ( LM_ERROR, ACE_TEXT ( "ERROR: %N:%l: on_data_available() - take_next_sample failed!\n" ) ) );
    }
}

void
SimpleDataTypeDataReaderListenerImpl::on_subscription_matched (
    DDS::DataReader_ptr /*reader*/,
    const DDS::SubscriptionMatchedStatus& /*status*/ )
{
}

void
SimpleDataTypeDataReaderListenerImpl::on_sample_lost (
    DDS::DataReader_ptr /*reader*/,
    const DDS::SampleLostStatus& /*status*/ )
{
}


