package com.thomsonreuters.upa.transport;

/**
 * Library Version Information to be populated with {@link Transport} library version info.
 * 
 * @see Transport
 */
public interface LibraryVersionInfo
{
    /**
     * Product Release and Load information.
     * 
     * @return the productVersion
     */
    public String productVersion();

    /**
     * Internal Node information, useful for raising questions or reporting issues.
     * 
     * @return the internalVersion
     */
    public String productInternalVersion();

    /**
     * Date library was produced for product release.
     * 
     * @return the productDate
     */
    public String productDate();
}