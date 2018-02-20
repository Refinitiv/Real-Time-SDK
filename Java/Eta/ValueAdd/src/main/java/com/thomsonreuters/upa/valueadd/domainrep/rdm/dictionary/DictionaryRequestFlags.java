package com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary;

/**
 * The RDM Dictionary Request Flags.
 * 
 * @see DictionaryRequest
 */
public class DictionaryRequestFlags
{
    /**
     * (0x0) No flags set.
     */
    public static final int NONE = 0x00; 
    
    
    /**
     * (0x1) Indicates this is to be a streaming request.
     */
    public static final int STREAMING = 0x01;

    private DictionaryRequestFlags()
    {
        throw new AssertionError();
    }
}
