package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.LibraryVersionInfo;

public class LibraryVersionInfoImpl implements LibraryVersionInfo
{
    private String _productVersion;
    private String _productInternalVersion;
    private String _productDate;

    public LibraryVersionInfoImpl()
    {
    }

    @Override
    public String toString()
    {
        return "Ultra Performance API (UPA), Java Edition, LibraryVersionInfo" + "\n" + 
               "\tproductVersion: " + _productVersion + "\n" + 
               "\tproductInternalVersion: " + _productInternalVersion + "\n" + 
               "\tproductDate: " + _productDate;
    }

    public void productVersion(String productVersion)
    {
        _productVersion = productVersion;
    }

    @Override
    public String productVersion()
    {
        return _productVersion;
    }

    public void productInternalVersion(String internalVersion)
    {
        _productInternalVersion = internalVersion;
    }

    @Override
    public String productInternalVersion()
    {
        return _productInternalVersion;
    }

    public void productDate(String productDate)
    {
        _productDate = productDate;
    }

    @Override
    public String productDate()
    {
        return _productDate;
    }
}
