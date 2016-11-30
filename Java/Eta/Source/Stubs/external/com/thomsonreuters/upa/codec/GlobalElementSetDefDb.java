package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.transport.Error;

/**
 * Global Message Field List Set Definitions Database that can groups ElementListSet definitions together.
 * <p>
 * Using a database can be helpful when the content leverages multiple
 * definitions; the database provides an easy way to pass around all set
 * definitions necessary to encode or decode information. For instance, a
 * {@link Vector} can contain multiple set definitions via a set definition
 * database with the contents of each {@link VectorEntry} requiring a different
 * definition from the database.
 * 
 * @see ElementSetDef
 */
public interface GlobalElementSetDefDb extends ElementSetDefDb
{
    /**
     * Clears {@link GlobalElementSetDefDb} and all entries in it. Useful for object reuse.
     */
    public void clear();

    /**
     * Decode the element set definition information contained in an encoded field
     * set def dictionary according to the domain model.
     * 
     * @param iter An iterator to use. Must be set to the encoded buffer.
     * @param verbosity The desired verbosity to decode.
     * @param error UPA Error, to be populated in event of an error.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     * @see com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues
     */
    public int decode(DecodeIterator iter, int verbosity, Error error);

    /**
     * Encode Element List set definitions database.
     * 
     * @param iter encode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter, Int currentSetDef, int verbosity, Error error);

    /**
     * Deep copies the given set definition into the database.
     * 
     * @param setDef    Set Defininition to be copied in.
     * @param error
     * 
     * @return {@link CodecReturnCodes}
     */
    public int addSetDef(ElementSetDef setDef, Error error);
    
    /**
     * The info_version
     * 
     * @return the Buffer info_version
     */
    Buffer info_version();

    /**
     * Set info_version
     * 
     * @param setInfo_version set Buffer info_version
     */
    void info_version(Buffer setInfo_version);

    /**
     * The info_DictionaryID
     * 
     * @return the info_DictionaryID
     */
    int info_DictionaryID();

    /**
     * Set info_DictionaryID
     * 
     * @param setInfo_DictionaryID set info_DictionaryID
     */
    void info_DictionaryID(int setInfo_DictionaryID);
}
