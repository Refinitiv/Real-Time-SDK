package com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.State;

/**
 * The RDM Dictionary Refresh. Used by an OMM Provider to provide the content of
 * a Dictionary.
 * 
 * @see DictionaryMsg
 * @see DictionaryRefreshFlags
 */
public interface DictionaryRefresh extends DictionaryMsg
{
    /**
     * Performs a deep copy of {@link DictionaryRefresh} object.
     *
     * @param destRefreshMsg Message to copy dictionary refresh object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(DictionaryRefresh destRefreshMsg);

    /**
     * The RDM Dictionary refresh flags. Populated by {@link DictionaryRefreshFlags}.
     * 
     * @return RDM Dictionary refresh flag
     */
    public int flags();

    /**
     * The RDM Dictionary refresh flags. Populated by {@link DictionaryRefreshFlags}.
     * 
     * @param flags - RDM Dictionary refresh flags
     */
    public void flags(int flags);

    /**
     * The verbosity of the dictionary being provided. Populated by
     * DictionaryVerbosityValues.
     * 
     * @return verbosity of the dictionary
     * 
     * @see com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues
     */
    public int verbosity();

    /**
     * The verbosity of the dictionary being provided. Populated by
     * DictionaryVerbosityValues.
     * 
     * @param verbosity - verbosity of the dictionary
     * 
     * @see com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues
     */
    public void verbosity(int verbosity);

    /**
     * Checks presence of the dictionaryId, version, and type fields .
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     *
     * @return true - if exists; false if does not exist.
     * @see #flags()
     */
    public boolean checkHasInfo();

    /**
     * Applies the flag for presence of the dictionaryId, version, and type
     * members.<br>
     * 
     * This flag is typically not used as this information is automatically
     * added by the encode method when appropriate.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     * 
     */
    public void applyHasInfo();

    /**
     * When encoding the message, this points to the dictionary object that is
     * being encoded. When decoding, this is not used.
     * 
     * @return DataDictionary
     */
    public DataDictionary dictionary();

    /**
     * encDictionary - When encoding the message, this points to the dictionary object that is
     * being encoded. When decoding, this is not used.
     *
     * @param encDictionary the enc dictionary
     */
    public void dictionary(DataDictionary encDictionary);

    /**
     * dictionaryName - The name of the dictionary being provided.
     * 
     * @return - dictionaryName
     */
    public Buffer dictionaryName();
    
    /**
     * Sets the the dictionaryName field for this message to the user specified
     * buffer. Buffer used by this object's dictionaryName field will be set to
     * passed in buffer's data and position. Note that this creates garbage if
     * buffer is backed by String object.
     *
     * @param dictionaryName the dictionary name
     */
    public void dictionaryName(Buffer dictionaryName);

    /**
     * The sequence number of this message.
     * 
     * @return sequenceNumber
     */
    public long sequenceNumber();

    /**
     * sequenceNumber - The sequence number of this message.
     *
     * @param sequenceNumber the sequence number
     */
    public void sequenceNumber(long sequenceNumber);

    /**
     * Checks the presence of sequence number field.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     *
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSequenceNumber();

    /**
     * Applies the sequence number flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     */
    public void applyHasSequenceNumber();

    /**
     * serviceId - The ID of the service providing the dictionary.
     * 
     * @return serviceId
     */
    public int serviceId();

    /**
     * serviceId - The ID of the service providing the dictionary.
     *
     * @param serviceId the service id
     */
    public void serviceId(int serviceId);

    /**
     * state - The current state of the stream.
     *
     * @return state
     */
    public State state();
    
    /**
     * Sets the state field for the dictionary refresh message.
     *
     * @param state the state
     */
    public void state(State state);

    /**
     * The type of the dictionary. Populated by  {@link com.thomsonreuters.upa.rdm.Dictionary.Types}.
     * 
     * @return dictionaryType
     */
    public int dictionaryType();

    /**
     * The type of the dictionary. Populated by  {@link com.thomsonreuters.upa.rdm.Dictionary.Types}.
     *
     * @param dictionaryType the dictionary type
     */
    public void dictionaryType(int dictionaryType);

    /**
     * When decoding, points to the payload of the message. The application
     * should set the iterator to this buffer and call the appropriate decode
     * method. This will add the data present to the {@link DataDictionary}
     * object. When encoding, this member is not used.
     * 
     * @return - payload buffer.
     */
    public Buffer dataBody();

    /**
     * This field is initialized with dictionary-&gt;minFid and after encoding each
     * part, updated with the start Fid for next encoded part. When decoding,
     * this is not used.
     *
     * @param startFid the start fid
     */
    public void startFid(int startFid);

    /**
     * This field is initialized with 0 and after encoding each
     * part, updated with the start Enum Table Count for next encoded part. 
     * When decoding, this is not used.
     *
     * @param startEnumTableCount the start enum table count
     */
    public void startEnumTableCount(int startEnumTableCount);    

    /**
     * This field is initialized with dictionary-&gt;minFid and after encoding each
     * part, updated with the start Fid for next encoded part. When decoding,
     * this is not used.
     * 
     * @return startFid
     */
    public int startFid();
    
    /**
     * This field is initialized with 0 and after encoding each
     * part, updated with the start Enum Table Count for next encoded part. 
     * When decoding, this is not used.
     * 
     * @return startEnumTableCount
     */
    public int startEnumTableCount();    

    /**
     * Checks the presence of clear cache flag.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkClearCache();

    /**
     * Applies clear cache flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     */
    public void applyClearCache();

    /**
     * Checks the presence of refresh complete flag.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     *
     * @return true - if exists; false if does not exist.
     */
    public boolean checkRefreshComplete();

    /**
     * Checks the presence of solicited flag.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkSolicited();

    /**
     * Applies solicited flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     */
    public void applySolicited();

    /**
     * Applies refresh complete flag.
     * 
     * Flags may also be bulk-set via {@link #flags(int)}.
     */
    public void applyRefreshComplete();
}
