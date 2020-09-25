package com.refinitiv.eta.codec;

import com.refinitiv.eta.transport.Error;

/**
 * Global Message Field List Set Definitions Database that can groups
 * FieldListSet definitions together.
 * <p>
 * Using a database can be helpful when the content leverages multiple
 * definitions; the database provides an easy way to pass around all set
 * definitions necessary to encode or decode information. For instance, a
 * {@link Vector} can contain multiple set definitions via a set definition
 * database with the contents of each {@link VectorEntry} requiring a different
 * definition from the database.
 * 
 * @see FieldSetDef
 */
public interface GlobalFieldSetDefDb extends FieldSetDefDb
{
    /**
     * Clears {@link GlobalFieldSetDefDb} and all entries in it.
     * Useful for object reuse.
     */
    public void clear();

    /**
     * Decode the field set definition information contained in an encoded field
     * set def dictionary according to the domain model.
     * 
     * @param iter An iterator to use. Must be set to the encoded buffer.
     * @param verbosity The desired verbosity to decode.
     * @param error UPA Error, to be populated in event of an error.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     * @see com.refinitiv.eta.rdm.Dictionary.VerbosityValues
     */
    public int decode(DecodeIterator iter, int verbosity, Error error);

    /**
     * Encode FieldList set definitions database.
     *
     * @param iter encode iterator
     * @param currentSetDef the current set def
     * @param verbosity the verbosity
     * @param error the error
     * @return {@link CodecReturnCodes}
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter, Int currentSetDef, int verbosity, Error error);

    /**
     * Deep copies the given set definition into the database.
     *
     * @param setDef    Set Defininition to be copied in.
     * @param error the error
     * @return {@link CodecReturnCodes}
     */
    public int addSetDef(FieldSetDef setDef, Error error);
    
    /**
     * The info_version.
     *
     * @return the Buffer info_version
     */
    public Buffer info_version();

    /**
     * Set info_version.
     *
     * @param setInfo_version set Buffer info_version
     */
    public void info_version(Buffer setInfo_version);

    /**
     * The info_DictionaryID.
     *
     * @return the info_DictionaryID
     */
    public int info_DictionaryID();

    /**
     * Set info_DictionaryID.
     *
     * @param setInfo_DictionaryID set info_DictionaryID
     */
    public void info_DictionaryID(int setInfo_DictionaryID);
}
