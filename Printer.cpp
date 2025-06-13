/* Copyright (C) neoliant.com - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary
 * Written by neoliant - info@neoliant.com, February 2021
 */

#include "Printer.h"

#include <string>
#include <sstream>
#include <iomanip>
using namespace::std;

#include <chrono>
using namespace std::chrono;

#include "date/date.h"
using namespace date;

string Printer::PrintBoolean ( const CORBA::Boolean& object ) {
    if ( object == true )
        return "true";
    else
        return "false";
}

string Printer::PrintChar ( const CORBA::Char& object ) {
    return std::to_string ( ( int ) object );
}

string Printer::PrintOctet ( const CORBA::Octet& object ) {
    return std::to_string ( ( int ) object );
}

string Printer::PrintFloat ( const CORBA::Float& object ) {
    char buffer[64];
    memset ( buffer, 0, sizeof ( buffer ) );
    snprintf ( buffer, sizeof ( buffer ), "%g", object );
    std::string strObj ( buffer );
    return strObj;
}

string Printer::PrintDouble ( const CORBA::Double& object ) {
    char buffer[64];
    memset ( buffer, 0, sizeof ( buffer ) );
    snprintf ( buffer, sizeof ( buffer ), "%g", object );
    std::string strObj ( buffer );
    return strObj;
}

string Printer::PrintDateTime ( DateTime datetime ) {
    if ( datetime == 0 )
        return "";

    // Decoding
    auto uaStartOfSeventies = 116444736000000000LL;
    auto epoch = time_point<nanoseconds>();
    auto since_epoch = nanoseconds ( 100 * ( datetime - uaStartOfSeventies ) );
    auto uaTp = epoch + since_epoch;

    nanoseconds ns = duration_cast<nanoseconds> ( uaTp.time_since_epoch() );
    milliseconds ms = round<milliseconds> ( ns );
    seconds s = duration_cast<seconds> ( ms );
    time_t t = s.count();
    size_t fractional_seconds = ms.count() % 1000;

    struct tm * ptm;
    ptm = gmtime ( &t );
    stringstream output;
    output << put_time ( ptm, "%FT%T" ) << "." << fractional_seconds << "Z";

    return output.str();
}

string Printer::PrintULongInHexaString ( CORBA::ULong l ) {
    stringstream stream;
    stream << setfill ( '0' ) << setw ( 8 ) << hex << l;

    return stream.str();
}

string Printer::PrintUShortInHexaString ( CORBA::UShort s ) {
    stringstream stream;
    stream << setfill ( '0' ) << setw ( 4 ) << hex << s;

    return stream.str();
}

string Printer::PrintOctetInHexaString ( CORBA::Octet o ) {
    stringstream stream;
    stream << setfill ( '0' ) << setw ( 2 ) << hex << ( int ) o;

    return stream.str();
}

string Printer::PrintGuid ( Guid guid ) {
    string output = "";

    output = "{";
    output += PrintULongInHexaString ( guid.data1() );
    output += "-";
    output += PrintUShortInHexaString ( guid.data2() );
    output += "-";
    output += PrintUShortInHexaString ( guid.data3() );
    output += "-";
    output += PrintOctetInHexaString ( guid.data4()[0] );
    output += PrintOctetInHexaString ( guid.data4()[1] );
    output += "-";
    output += PrintOctetInHexaString ( guid.data4()[2] );
    output += PrintOctetInHexaString ( guid.data4()[3] );
    output += PrintOctetInHexaString ( guid.data4()[4] );
    output += PrintOctetInHexaString ( guid.data4()[5] );
    output += PrintOctetInHexaString ( guid.data4()[6] );
    output += PrintOctetInHexaString ( guid.data4()[7] );
    output += "}";

    return output;
}

string Printer::PrintByteString ( ByteString byteString ) {
    stringstream stream;

    if ( byteString.size() > 0 ) {
        for ( int i = 0; i < byteString.size(); i ++ ) {
            stream << ( char ) byteString[ i ];
        }
    }

    return stream.str();
}

string Printer::PrintXmlElement ( XmlElement xmlelement ) {
    string output = xmlelement;

    return output;
}

string Printer::PrintNodeId ( NodeId nodeId ) {
    string output = "";

    if ( nodeId.identifier_type()._d() == NodeIdentifierKind::STRING_NODE_ID )
        output = "ns=" + to_string ( nodeId.namespace_index() ) + ";s=" + nodeId.identifier_type().string_id();
    else if ( nodeId.identifier_type()._d() == NodeIdentifierKind::NUMERIC_NODE_ID )
        output = "ns=" + to_string ( nodeId.namespace_index() ) + ";s=" + std::to_string ( nodeId.identifier_type().numeric_id() );
    else if ( nodeId.identifier_type()._d() == NodeIdentifierKind::OPAQUE_NODE_ID )
        output = "ns=" + to_string ( nodeId.namespace_index() ) + ";s=--Opaque--";
    else if ( nodeId.identifier_type()._d() == NodeIdentifierKind::GUID_NODE_ID )
        output = "ns=" + to_string ( nodeId.namespace_index() ) + ";s=" + PrintGuid ( nodeId.identifier_type().guid_id() );

    return output;
}

string Printer::PrintExpandedNodeId ( ExpandedNodeId expandedNodeId ) {
    string output = "";

    output = PrintNodeId ( expandedNodeId.node_id() );
    output += ";nu=";
    output += expandedNodeId.namespace_uri();
    output += ";si=";
    output += to_string ( expandedNodeId.server_index() );

    return output;
}

string Printer::PrintStatusCode ( StatusCode statusCode ) {
    string output = to_string ( statusCode );

    return output;
}

string Printer::PrintQualifiedName ( QualifiedName qName ) {
    string output = "";

    if ( qName.name() != "" ) {
        output = "ns=";
        output += to_string ( qName.namespace_index() );
        output += ";s=";
        output += qName.name();
    }

    return output;
}

string Printer::PrintLocalizedText ( LocalizedText lText ) {
    string output = "";

    if ( lText.locale() != "" ) {
        output = "lo=";
        output += lText.locale();
        output += ";s=";
        output += lText.text();
    }

    return output;
}

string Printer::PrintExtensionObject ( ExtensionObject eObject ) {
    string output = "";

    output = PrintNodeId ( eObject.type_id() );
    output += ";c=";
    if ( eObject.body()._d() == BodyEncoding::BYTESTRING_BODY_ENCODING ) {
        for ( int i = 0; i < eObject.body().bytestring_encoding().size(); i++ ) {
            output += eObject.body().bytestring_encoding() [i];
        }
    } else if ( eObject.body()._d() == BodyEncoding::NONE_BODY_ENCODING ) {
        output += eObject.body().none_encoding();
    } else if ( eObject.body()._d() == BodyEncoding::XMLELEMENT_BODY_ENCODING ) {
        for ( int i = 0; i < strlen ( eObject.body().xmlelement_encoding().c_str() ); i++ ) {
            output += eObject.body().xmlelement_encoding() [i];
        }
    }

    return output;
}

string Printer::PrintDataValue( DataValue value )
{
    string output { "" };
    
    output = "Source Timestamp = ";
    output += std::to_string( value.source_timestamp() );
    output += "; Source pico = ";
    output += std::to_string( value.source_pico_sec() );
    output += "; Server Timestamp = ";
    output += std::to_string( value.server_timestamp() );
    output += "; Server pico = ";
    output += std::to_string( value.server_pico_sec() );
    output += "; Status = ";
    if ( value.status() == 0 )
        output += "GOOD";
    else
        output += "BAD";
    output += "; Value = ";
    output += PrintVariant( value.value() );
    
    return output;
}

string Printer::PrintStringArray ( StringArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += array[ i ].data();
        output += " / ";
    }

    return output;
}

string Printer::PrintDateTimeArray ( DateTimeArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintDateTime ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintGuidArray ( GuidArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintGuid ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintByteStringArray ( ByteStringArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintByteString ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintXmlElementArray ( XmlElementArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += array[ i ].data();
        output += " / ";
    }

    return output;
}

string Printer::PrintNodeIdArray ( NodeIdArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintNodeId ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintExpandedNodeIdArray ( ExpandedNodeIdArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintExpandedNodeId ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintStatusCodeArray ( StatusCodeArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintStatusCode ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintQualifiedNameArray ( QualifiedNameArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintQualifiedName ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintLocalizedTextArray ( LocalizedTextArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintLocalizedText ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintExtensionObjectArray ( ExtensionObjectArray array ) {
    string output = "";

    for ( int i = 0; i < array.size(); i++ ) {
        output += PrintExtensionObject ( array[ i ] );
        output += " / ";
    }

    return output;
}

string Printer::PrintStringMatrix ( StringMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += matrix.array()[i].data();

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintDateTimeMatrix ( DateTimeMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += PrintDateTime ( matrix.array()[i] );

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintGuidMatrix ( GuidMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += PrintGuid ( matrix.array()[i] );

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintByteStringMatrix ( ByteStringMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += PrintByteString ( matrix.array()[i] );

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintXmlElementMatrix ( XmlElementMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += matrix.array()[i].data();

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintNodeIdMatrix ( NodeIdMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += PrintNodeId ( matrix.array()[i] );

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintExpandedNodeIdMatrix ( ExpandedNodeIdMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += PrintExpandedNodeId ( matrix.array()[i] );

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintQualifiedNameMatrix ( QualifiedNameMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += PrintQualifiedName ( matrix.array()[i] );

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintLocalizedTextMatrix ( LocalizedTextMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += PrintLocalizedText ( matrix.array()[i] );

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

string Printer::PrintExtensionObjectMatrix ( ExtensionObjectMatrix matrix ) {
    string output = "";

    int uaDimsSize = matrix.array_dimensions().size();
    int currentIndex[ uaDimsSize ];
    for ( int i = 0; i < uaDimsSize; i++ )
        currentIndex[i] = 0;
    bool nextLevel = false;

    for ( int i = 0; i < matrix.array().size(); i++ ) {
        string currentIndexString = "";
        for ( int j = 0; j < uaDimsSize; j++ ) {
            currentIndexString += std::to_string ( currentIndex[ j ] );
            currentIndexString += " ";
        }

        output += currentIndexString;
        output += ": ";
        output += PrintExtensionObject ( matrix.array()[i] );

        nextLevel = true;

        for ( int j = uaDimsSize - 1; j >= 0; j-- ) {
            if ( nextLevel ) {
                currentIndex[ j ] += 1;
                nextLevel = false;
            }
            if ( currentIndex[ j ] == matrix.array_dimensions()[ j ] ) {
                currentIndex[ j ] = 0;
                nextLevel = true;
            }
        }

        if ( i == matrix.array().size() - 1 )
            break;
        else
            output += " / ";
    }

    return output;
}

// So far we just print scalars and vectors.
// TODO Should be improved to print whatever matrix
string Printer::PrintVariant ( const Variant & variant ) {
    string output = "";

    if ( variant.array_dimensions().size() == 1 && variant.array_dimensions()[0] == 1 ) {
        switch ( variant.value()[0]._d() ) {
        case BuiltinTypeKind::BOOLEAN_TYPE:
            output += PrintBoolean ( variant.value()[0].bool_value() );
            break;
        case BuiltinTypeKind::BYTE_TYPE:
            output += PrintChar ( variant.value()[0].byte_value() );
            break;
        case BuiltinTypeKind::SBYTE_TYPE:
            output += PrintOctet ( variant.value()[0].sbyte_value() );
            break;
        case BuiltinTypeKind::INT16_TYPE:
            output += to_string( variant.value()[0].short_value() );
            break;
        case BuiltinTypeKind::UINT16_TYPE:
            output += to_string( variant.value()[0].int16_value() );
            break;
        case BuiltinTypeKind::INT32_TYPE:
            output += to_string( variant.value()[0].long_value() );
            break;
        case BuiltinTypeKind::UINT32_TYPE:
            output += to_string( variant.value()[0].int32_value() );
            break;
        case BuiltinTypeKind::INT64_TYPE:
            output+= to_string( variant.value()[0].int64_value() );
            break;
        case BuiltinTypeKind::UINT64_TYPE:
            output += to_string( variant.value()[0].uint64_value() );
            break;
        case BuiltinTypeKind::FLOAT_TYPE:
            output += to_string( variant.value()[0].float_value() );
            break;
        case BuiltinTypeKind::DOUBLE_TYPE:
            output += to_string( variant.value()[0].double_value() );
            break;
        case BuiltinTypeKind::STRING_TYPE:
            output += variant.value()[0].string_value();
            break;
        case BuiltinTypeKind::DATETIME_TYPE:
            output += to_string( variant.value()[0].datetime_value() );
            break;
        case BuiltinTypeKind::GUID_TYPE:
            output += PrintGuid ( variant.value()[0].guid_value() );
            break;
        case BuiltinTypeKind::BYTESTRING_TYPE:
            output += PrintByteString ( variant.value()[0].bytestring_value() );
            break;
        case BuiltinTypeKind::XMLELEMENT_TYPE:
            output += variant.value()[0].xmlelement_value();
            break;
        case BuiltinTypeKind::NODEID_TYPE:
            output += PrintNodeId ( variant.value()[0].nodeid_value() );
            break;
        case BuiltinTypeKind::EXPANDEDNODEID_TYPE:
            output += PrintExpandedNodeId ( variant.value()[0].expandednodeid_value() );
            break;
        case BuiltinTypeKind::STATUSCODE_TYPE:
            output += to_string( variant.value()[0].statuscode_value() );
            break;
        case BuiltinTypeKind::QUALIFIEDNAME_TYPE:
            output += PrintQualifiedName ( variant.value()[0].qualifiedname_value() );
            break;
        case BuiltinTypeKind::LOCALIZEDTEXT_TYPE:
            output += PrintLocalizedText ( variant.value()[0].localizedtext_value() );
            break;
        case BuiltinTypeKind::EXTENSIONOBJECT_TYPE:
            output += PrintExtensionObject ( variant.value()[0].extensionobject_value() );
            break;
        default:
            break;
        }
    } else if ( variant.array_dimensions().size() == 1 && variant.array_dimensions()[0] > 1 ) {
        for ( int i = 0; i < variant.array_dimensions()[0]; i++ ) {
            switch ( variant.value()[i]._d() ) {
            case BuiltinTypeKind::BOOLEAN_TYPE:
                output += PrintBoolean ( variant.value()[i].bool_value() );
                break;
            case BuiltinTypeKind::BYTE_TYPE:
                output += PrintChar ( variant.value()[i].byte_value() );
                break;
            case BuiltinTypeKind::SBYTE_TYPE:
                output += PrintOctet ( variant.value()[i].sbyte_value() );
                break;
            case BuiltinTypeKind::INT16_TYPE:
                output += to_string( variant.value()[i].short_value() );
                break;
            case BuiltinTypeKind::UINT16_TYPE:
                output += to_string( variant.value()[i].int16_value() );
                break;
            case BuiltinTypeKind::INT32_TYPE:
                output += to_string( variant.value()[i].long_value() );
                break;
            case BuiltinTypeKind::UINT32_TYPE:
                output += to_string( variant.value()[i].int32_value() );
                break;
            case BuiltinTypeKind::INT64_TYPE:
                output += to_string( variant.value()[i].int64_value() );
                break;
            case BuiltinTypeKind::UINT64_TYPE:
                output += to_string( variant.value()[i].uint64_value() );
                break;
            case BuiltinTypeKind::FLOAT_TYPE:
                output += to_string( variant.value()[i].float_value() );
                break;
            case BuiltinTypeKind::DOUBLE_TYPE:
                output += to_string( variant.value()[i].double_value() );
                break;
            case BuiltinTypeKind::STRING_TYPE:
                output += variant.value()[i].string_value();
                break;
            case BuiltinTypeKind::DATETIME_TYPE:
                output += to_string( variant.value()[i].datetime_value() );
                break;
            case BuiltinTypeKind::GUID_TYPE:
                output += PrintGuid ( variant.value()[i].guid_value() );
                break;
            case BuiltinTypeKind::BYTESTRING_TYPE:
                output += PrintByteString ( variant.value()[i].bytestring_value() );
                break;
            case BuiltinTypeKind::XMLELEMENT_TYPE:
                output+= variant.value()[i].xmlelement_value();
                break;
            case BuiltinTypeKind::NODEID_TYPE:
                output += PrintNodeId ( variant.value()[i].nodeid_value() );
                break;
            case BuiltinTypeKind::EXPANDEDNODEID_TYPE:
                output += PrintExpandedNodeId ( variant.value()[i].expandednodeid_value() );
                break;
            case BuiltinTypeKind::STATUSCODE_TYPE:
                output += to_string( variant.value()[i].statuscode_value() );
                break;
            case BuiltinTypeKind::QUALIFIEDNAME_TYPE:
                output += PrintQualifiedName ( variant.value()[i].qualifiedname_value() );
                break;
            case BuiltinTypeKind::LOCALIZEDTEXT_TYPE:
                output += PrintLocalizedText ( variant.value()[i].localizedtext_value() );
                break;
            case BuiltinTypeKind::EXTENSIONOBJECT_TYPE:
                output += PrintExtensionObject ( variant.value()[i].extensionobject_value() );
                break;
            default:
                break;
            }
            output += " ";
        }
    }

    return output;
}

