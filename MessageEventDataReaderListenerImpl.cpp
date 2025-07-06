/* Copyright (C) neoliant.com - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary
 * Written by neoliant - info@neoliant.com, February 2021
 */

#include "MessageEventDataReaderListenerImpl.h"
/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include <MessageEventTypeSupportC.h>
#include <MessageEventTypeSupportImpl.h>

#include "Printer.h"

#include <iostream>

void
MessageEventDataReaderListenerImpl::on_requested_deadline_missed (
    DDS::DataReader_ptr /*reader*/,
    const DDS::RequestedDeadlineMissedStatus& /*status*/ )
{
}

void
MessageEventDataReaderListenerImpl::on_requested_incompatible_qos (
    DDS::DataReader_ptr /*reader*/,
    const DDS::RequestedIncompatibleQosStatus& /*status*/ )
{
}

void
MessageEventDataReaderListenerImpl::on_sample_rejected (
    DDS::DataReader_ptr /*reader*/,
    const DDS::SampleRejectedStatus& /*status*/ )
{
}

void
MessageEventDataReaderListenerImpl::on_liveliness_changed (
    DDS::DataReader_ptr /*reader*/,
    const DDS::LivelinessChangedStatus& /*status*/ )
{
}

void
MessageEventDataReaderListenerImpl::on_data_available ( DDS::DataReader_ptr reader )
{
    MessageEventDataReader_var reader_dp = MessageEventDataReader::_narrow ( reader );

    if ( !reader_dp ) {
        ACE_ERROR ( ( LM_ERROR,
                      ACE_TEXT ( "ERROR: %N:%l: on_data_available() -" )
                      ACE_TEXT ( " _narrow failed!\n" ) ) );
        ACE_OS::exit ( -1 );
    }

    MessageEvent messageEvent;
    DDS::SampleInfo info;

    DDS::ReturnCode_t error = reader_dp->take_next_sample ( messageEvent, info );

    if ( error == DDS::RETCODE_OK ) {
        std::cout << "SampleInfo.sample_rank = " << info.sample_rank << std::endl;
        std::cout << "SampleInfo.instance_state = " << info.instance_state << std::endl;

        Printer printer;
        if ( info.valid_data ) {
            std::cout << "MessageEvent::Time = " << printer.PrintDateTime(messageEvent.Time())
                      << "; SimpleDataType::SourceName = " << messageEvent.SourceName().data()
                      << "; SimpleDataType::Severity = " << messageEvent.Severity()
                      << "; SimpleDataType::Message = " << printer.PrintLocalizedText( messageEvent.Message() ) << std::endl;
        }
    } else {
        ACE_ERROR ( ( LM_ERROR,
                      ACE_TEXT ( "ERROR: %N:%l: on_data_available() -" )
                      ACE_TEXT ( " take_next_sample failed!\n" ) ) );
    }
}

void
MessageEventDataReaderListenerImpl::on_subscription_matched (
    DDS::DataReader_ptr /*reader*/,
    const DDS::SubscriptionMatchedStatus& /*status*/ )
{
}

void
MessageEventDataReaderListenerImpl::on_sample_lost (
    DDS::DataReader_ptr /*reader*/,
    const DDS::SampleLostStatus& /*status*/ )
{
}


