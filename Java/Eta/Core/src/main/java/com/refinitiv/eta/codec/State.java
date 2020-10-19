package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * State conveys data and stream health information. When present in a message
 * header, {@link State} applies to the state of the stream and data. When
 * present in a data payload, the meaning of {@link State} should be defined by the DMM.
 * 
 * @see DataStates
 * @see StreamStates
 * @see StateCodes
 */
public interface State
{
    /**
     * Clears {@link State}. Useful for object reuse.
     */
    public void clear();
    
    /**
     * Is State blank.
     *
     * @return true, if is blank
     */
    public boolean isBlank();

    /**
     * Perform a deep copy into destState.
     *
     * @param destState the value getting populated with the values of the calling Object.
     *         If Copying destState.text, the user must provide a {@link java.nio.ByteBuffer ByteBuffer}
     *         large enough to the hold the contents of this.text 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destState is null. 
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if destination buffer is too small
     *         
     * @see CopyMsgFlags
     */
    public int copy(State destState);
    
    /**
     * Used to encode State into a buffer.
     * 
     * @param iter {@link EncodeIterator} with buffer to encode into.
     *            Iterator should also have appropriate version information set
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Decode State.
     * 
     * @param iter {@link DecodeIterator} with buffer to decode from and
     *            appropriate version information set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if success,
     *         {@link CodecReturnCodes#INCOMPLETE_DATA} if failure,
     *         {@link CodecReturnCodes#BLANK_DATA} if data is blank value.
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Provides string representation for a {@link State} value, using the state info methods.
     * 
     * @return string representation for a {@link State} value
     */
    public String toString();

    /**
     * Checks if the two State values are equal.
     * 
     * @param thatState the other State to compare to this one
     * 
     * @return true if the two State values are equal, false otherwise
     */
    public boolean equals(State thatState);

    /**
     * Is Final State.
     * 
     * @return true if the state is a final state, false otherwise.
     */
    public boolean isFinal();

    /**
     * An enumerated value providing information about the state of the stream, populated from {@link StreamStates}.
     * Must be in the range of 0 - 31.
     * 
     * @param streamState the streamState to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if streamState is invalid. 
     */
    public int streamState(int streamState);

    /**
     * An enumerated value providing information about the state of the stream,
     * populated from {@link StreamStates}.
     * 
     * @return the streamState
     */
    public int streamState();

    /**
     * An enumerated value providing information about the state of data, populated from {@link DataStates}.
     * Must be in the range of 0 - 7.
     * 
     * @param dataState the dataState to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if dataState is invalid. 
     */
    public int dataState(int dataState);

    /**
     * An enumerated value providing information about the state of data, populated from {@link DataStates}.
     * 
     * @return the dataState
     */
    public int dataState();

    /**
     * An enumerated code providing additional state information. Typically
     * indicates more specific information (e.g., pertaining to a condition
     * occurring upstream causing current data and stream states).
     * code is typically used for informational purposes.
     * Populated from {@link StateCodes}.
     * Must be in the range of {@link StateCodes#NONE} - 127.
     * 
     * @param code the code to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if code is invalid. 
     */
    public int code(int code);

    /**
     * An enumerated code providing additional state information. Typically
     * indicates more specific information (e.g., pertaining to a condition
     * occurring upstream causing current data and stream states).
     * code is typically used for informational purposes.
     * Populated from * {@link StateCodes}.
     * 
     * @return the code
     */
    public int code();

    /**
     * Text describing the state or state code.Typically used for informational purposes.
     * 
     * @param text the text to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if text is invalid. 
     */
    public int text(Buffer text);

    /**
     * Text describing the state or state code.Typically used for informational purposes.
     * 
     * @return the text
     */
    public Buffer text();
}