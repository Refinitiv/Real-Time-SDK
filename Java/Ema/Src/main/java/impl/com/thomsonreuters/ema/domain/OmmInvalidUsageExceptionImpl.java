package com.thomsonreuters.ema.domain;

import com.thomsonreuters.ema.access.OmmInvalidUsageException;


class OmmInvalidUsageExceptionImpl extends OmmInvalidUsageException
{
    private static final long serialVersionUID = 8058642359690884128L;

    private static final String OMMINVALIDUSAGE_EXCEPTION_STRING = "OmmInvalidUsageException";
    
    @Override
    public String exceptionTypeAsString()
    {
        return OMMINVALIDUSAGE_EXCEPTION_STRING;
    }

    @Override
    public int exceptionType()
    {
        return ExceptionType.OmmInvalidUsageException;
    }

    OmmInvalidUsageException message(String exceptMessage)
    {
        _exceptMessage = exceptMessage;
        return this;
    }

}