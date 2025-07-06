/* Copyright (C) neoliant.com - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary
 * Written by neoliant - info@neoliant.com, February 2021
 */

#ifndef PRINTER_H
#define PRINTER_H

#include <string>
using namespace std;
#include <dds-opcua_builtin_typesC.h>
using namespace OMG::DDSOPCUA::OPCUA2DDS;
#include <tao/Basic_Types.h>

class Printer {
public:
    string PrintBoolean( const CORBA::Boolean& object );
    
    string PrintChar( const CORBA::Char & object );
    
    string PrintOctet( const CORBA::Octet & object );
    
    string PrintFloat( const CORBA::Float& object );

    string PrintDouble( const CORBA::Double& object );

    string PrintDateTime( DateTime datetime );

    string PrintGuid( Guid guid );

    string PrintByteString( ByteString bytestring );

    string PrintXmlElement( XmlElement xmlelement );

    string PrintNodeId( NodeId nodeid );

    string PrintExpandedNodeId( ExpandedNodeId expandednodeid );

    string PrintStatusCode( StatusCode statuscode );

    string PrintQualifiedName( QualifiedName qualifiedname );

    string PrintLocalizedText( LocalizedText localizeddtext );

    string PrintExtensionObject( ExtensionObject extensionobject );

    string PrintULongInHexaString( CORBA::ULong l );

    string PrintUShortInHexaString( CORBA::UShort s );

    string PrintOctetInHexaString( CORBA::Octet o );
    
    string PrintVariant( const Variant & variant );

    template<typename T>
    string PrintArray( T array )
    {
        string output = "";

        for (int i = 0; i < array.size(); i++)
        {
            if ( std::is_same<T, FloatArray>::value )
                output += PrintFloat( array[ i ] );
            else if ( std::is_same<T, DoubleArray>::value )
                output += PrintDouble( array[ i ] );
            else
                output += std::to_string( array[ i ] );
            output += " / ";
        }

        return output;
    }

    template<typename T>
    string PrintMatrix( T matrix )
    {
        string output = "";

        int uaDimsSize = matrix.array_dimensions().size();
        int currentIndex[ uaDimsSize ];
        for (int i = 0; i < uaDimsSize; i++)
            currentIndex[i] = 0;
        bool nextLevel = false;

        for (int i = 0; i < matrix.array().size(); i++) {
            string currentIndexString = "";
            for (int j = 0; j < uaDimsSize; j++) {
                currentIndexString += std::to_string( currentIndex[ j ] );
                currentIndexString += " ";
            }

            output += currentIndexString;
            output += ": ";
            if ( std::is_same<T, BooleanMatrix>::value )
                output += PrintBoolean( matrix.array()[ i ] );
            else if ( std::is_same<T, SByteMatrix>::value )
                output += PrintChar( matrix.array()[ i ] );
            else if ( std::is_same<T, ByteMatrix>::value )
                output += PrintOctet( matrix.array()[ i ] );
            else if ( std::is_same<T, Int16Matrix>::value )
                output += std::to_string( matrix.array()[ i ] );
            else if ( std::is_same<T, UInt16Matrix>::value )
                output += std::to_string( matrix.array()[ i ] );
            else if ( std::is_same<T, Int32Matrix>::value )
                output += std::to_string( matrix.array()[ i ] );
            else if ( std::is_same<T, UInt32Matrix>::value )
                output += std::to_string( matrix.array()[ i ] );
            else if ( std::is_same<T, Int64Matrix>::value )
                output += std::to_string( matrix.array()[ i ] );
            else if ( std::is_same<T, UInt64Matrix>::value )
                output += std::to_string( matrix.array()[ i ] );
            else if ( std::is_same<T, FloatMatrix>::value )
                output += PrintFloat( matrix.array()[ i ] );
            else if ( std::is_same<T, DoubleMatrix>::value )
                output += PrintDouble( matrix.array()[ i ] );
            else if ( std::is_same<T, DateTimeMatrix>::value )
                output += PrintDateTime( matrix.array()[ i ] );
            else if ( std::is_same<T, StatusCodeMatrix>::value )
                output += std::to_string( matrix.array()[ i ] );

            nextLevel = true;

            for (int j = uaDimsSize - 1; j >= 0; j--)
            {
                if (nextLevel)
                {
                    currentIndex[ j ] += 1;
                    nextLevel = false;
                }
                if (currentIndex[ j ] == matrix.array_dimensions()[ j ])
                {
                    currentIndex[ j ] = 0;
                    nextLevel = true;
                }
            }

            if (i == matrix.array().size() - 1)
                break;
            else
                output += " / ";
        }

        return output;
    }

    string PrintStringArray( StringArray array );

    string PrintDateTimeArray( DateTimeArray array );

    string PrintGuidArray( GuidArray array );

    string PrintByteStringArray( ByteStringArray array );

    string PrintXmlElementArray( XmlElementArray array );

    string PrintNodeIdArray( NodeIdArray array );

    string PrintExpandedNodeIdArray( ExpandedNodeIdArray array );

    string PrintStatusCodeArray( StatusCodeArray array );

    string PrintQualifiedNameArray( QualifiedNameArray array );

    string PrintLocalizedTextArray( LocalizedTextArray array );

    string PrintExtensionObjectArray( ExtensionObjectArray array );
    
    string PrintStringMatrix( StringMatrix matrix );
    
    string PrintDateTimeMatrix( DateTimeMatrix matrix );

    string PrintGuidMatrix( GuidMatrix matrix );

    string PrintByteStringMatrix( ByteStringMatrix matrix );

    string PrintXmlElementMatrix( XmlElementMatrix matrix );

    string PrintNodeIdMatrix( NodeIdMatrix matrix );

    string PrintExpandedNodeIdMatrix( ExpandedNodeIdMatrix matrix );

    string PrintStatusCodeMatrix( StatusCodeMatrix matrix );

    string PrintQualifiedNameMatrix( QualifiedNameMatrix matrix );

    string PrintLocalizedTextMatrix( LocalizedTextMatrix matrix );

    string PrintExtensionObjectMatrix( ExtensionObjectMatrix matrix );
    
    string PrintDataValue( DataValue value );
};

#endif
