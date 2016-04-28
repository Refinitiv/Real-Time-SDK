package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.RwfDataConstants;
import com.thomsonreuters.upa.transport.LibraryVersionInfo;
import com.thomsonreuters.upa.transport.LibraryVersionInfoImpl;

/**
 * Collection of interfaces to query supported RWF versions for encoder/decoder
 * as well as RWF protocol type being used by the connection.
 */
public class Codec
{
    /** The RWF protocol type. */
    static public final int RWF_PROTOCOL_TYPE = 0;
    static private Package _package = Package.getPackage("com.thomsonreuters.upa.codec");
    static private LibraryVersionInfoImpl _libVersionInfo = new LibraryVersionInfoImpl();

    static
    {
        _libVersionInfo.productDate(_package.getImplementationVendor());
        _libVersionInfo.productInternalVersion(_package.getImplementationVersion());
        _libVersionInfo.productVersion(_package.getSpecificationVersion());
    }

    /**
     * Protocol type definition. This can be used in conjunction transport layer
     * to indicate protocol being used on the connection. Codec only supports RWF
     * protocol type.
     * 
     * @return protocol type.
     */
    public static int protocolType()
    {
        return RWF_PROTOCOL_TYPE;
    }

    /**
     * Version Major number for the version of RWF supported.
     * 
     * @return RWF major version
     */
    public static int majorVersion()
    {
        return RwfDataConstants.MAJOR_VERSION_1;
    }

    /**
     * Version Minor number for the version of RWF supported.
     * 
     * @return RWF minor version
     */
    public static int minorVersion()
    {
        return RwfDataConstants.MINOR_VERSION_1;
    }

    /**
     * Programmatically extracts library and product version information that is
     * compiled into this library.<BR>
     * 
     * User can call this method to programmatically extract version
     * information.<BR>
     * 
     * @see com.thomsonreuters.upa.transport.LibraryVersionInfo
     */
    public static LibraryVersionInfo queryVersion()
    {
        return _libVersionInfo;
    }
}
