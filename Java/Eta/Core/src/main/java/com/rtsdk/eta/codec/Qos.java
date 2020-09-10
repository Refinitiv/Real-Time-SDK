package com.rtsdk.eta.codec;

/**
 * <p>
 * UPA Quality of Service class contains information rate and/or timeliness
 * information. Timeliness conveys information about the age of data. Rate
 * conveys information about the data's period of change. Some timeliness or
 * rate values may allow for additional time or rate information to be provided.
 * 
 * A consumer can use {@link RequestMsg} to indicate the desired QoS for its
 * streams. This can be a request for a specific QoS or a range of qualities of
 * service, where any value within the range will satisfy the request. The
 * {@link RefreshMsg} includes QoS used to indicate the QoS being provided for a
 * stream. When issuing a request, the QoS specified on the request typically
 * matches the advertised QoS of the service, as conveyed via the Source
 * Directory domain model.
 *
 * <ul>
 * <li>
 * An initial request containing only {@link RequestMsg#qos()} indicates a
 * request for the specified QoS. If a provider cannot satisfy this QoS, the
 * request should be rejected.</li>
 * <li>An initial request containing both {@link RequestMsg#qos()} and
 * {@link RequestMsg#worstQos()} sets the range of acceptable QoSs.</li>
 * </ul>
 * <p>
 * Any QoS within the range, inclusive of the specified qos and worstQos, will
 * satisfy the request. If a provider cannot provide a QoS within the range, the
 * provider should reject the request. When a provider responds to an initial
 * request, the {@link RefreshMsg#qos()} should contain the actual QoS being
 * provided to the stream. Subsequent requests should not specify a range as the
 * QoS has been established for the stream. Because QoS information is mostly
 * optional on a {@link RequestMsg} (Some components may require qos on initial
 * request and reissue messages):
 *
 * <ul>
 * <li>If neither qos nor worstQos are specified on an initial request to open a
 * stream, it is assumed that any QoS will satisfy the request. Additionally, it
 * will have a timeliness of {@link QosTimeliness#REALTIME} and a rate of
 * {@link QosRates#TICK_BY_TICK}</li>
 * <li>If QoS information is absent on a subsequent reissue request, it is
 * assumed that QoS, timeliness, and rate conform to the stream's currently
 * established settings</li>
 * </ul>.
 * 
 * @see QosRates
 * @see QosTimeliness
 */
public interface Qos
{
    /**
     * Clears {@link Qos}. Useful for object reuse.
     */
    void clear();
    
    /**
     * Is Qos blank.
     *
     * @return true, if is blank
     */
    public boolean isBlank();

    /**
     * This method will perform a deep copy into the passed in parameter's 
     *          members from the Object calling this method.
     * 
     * @param destQos the value getting populated with the values of the calling Object
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destQos is null. 
     */
    public int copy(Qos destQos);
    
    /**
     * Used to encode Qos into a buffer.
     * 
     * @param iter {@link EncodeIterator} with buffer to encode into. Iterator
     *            should also have appropriate version information set.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Decode Qos.
     * 
     * @param iter {@link DecodeIterator} with buffer to decode from and
     *            appropriate version information set
     * 
     * @return {@link CodecReturnCodes} . {@link CodecReturnCodes#SUCCESS} if
     *         success, {@link CodecReturnCodes#INCOMPLETE_DATA} if failure,
     *         {@link CodecReturnCodes#BLANK_DATA} if data is blank value.
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Provide string representation for a {@link Qos} value.
     * 
     * @return string representation for a {@link Qos} value
     */
    public String toString();

    /**
     * Checks if the two Qos values are equal.
     * 
     * @param thatQos the other Qos to compare to this one
     * 
     * @return true if the two Qos values are equal, false otherwise
     */
    public boolean equals(Qos thatQos);

    /**
     * Checks if Qos is better than another Qos.
     * 
     * @param thatQos the other Qos to compare to this one
     * 
     * @return true if Qos is better, false otherwise
     */
    public boolean isBetter(Qos thatQos);

    /**
     * Checks if Qos is in the range of best and worse.
     * 
     * @param bestQos the best Qos to compare to this one
     * @param worstQos the worst Qos to compare to this one
     * 
     * @return true if Qos is in range, false otherwise
     */
    public boolean isInRange(Qos bestQos, Qos worstQos);

    /**
     * Information timeliness enum, from {@link QosTimeliness}. Used to convey
     * information about the age of the data. Must be in the range of 0 - 7.
     * 
     * @param timeliness the timeliness to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if timeliness is invalid. 
     */
    public int timeliness(int timeliness);

    /**
     * Information timeliness enum, from {@link QosTimeliness}. Used to convey
     * information about the age of the data.
     * 
     * @return the timeliness
     */
    public int timeliness();

    /**
     * Information rate enum, from {@link QosRates}. Used to convey information
     * about the data's period of change. Must be in the range of 0 - 15.
     * 
     * @param rate the rate to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if rate is invalid. 
     */
    public int rate(int rate);

    /**
     * Information rate enum, from {@link QosRates}. Used to convey information
     * about the data's period of change.
     * 
     * @return the rate
     */
    public int rate();

    /**
     * If true, Qos is dynamic. Used to describe the changeability of the
     * quality of service, typically over the life of a data stream.
     * 
     * @param dynamic the dynamic to set
     */
    public void dynamic(boolean dynamic);

    /**
     * If true, Qos is dynamic. Used to describe the changeability of the
     * quality of service, typically over the life of a data stream.
     * 
     * @return the dynamic
     */
    public boolean isDynamic();

    /**
     * Specific timeliness information. Only present when timeliness is set to
     * {@link QosTimeliness#DELAYED}. Must be in the range of 0 - 65535.
     * 
     * @param timeInfo the timeInfo to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if timeInfo is invalid. 
     */
    public int timeInfo(int timeInfo);

    /**
     * Specific timeliness information. Only present when timeliness is set to
     * {@link QosTimeliness#DELAYED}.
     * 
     * @return the timeInfo
     */
    public int timeInfo();

    /**
     * Specific rate information in milliseconds. Only present when rate is set
     * to {@link QosRates#TIME_CONFLATED}. Must be in the range of 0 - 65535.
     * 
     * @param rateInfo the rateInfo to set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if rateInfo is invalid. 
     */
    public int rateInfo(int rateInfo);

    /**
     * Specific rate information in milliseconds. Only present when rate is set
     * to {@link QosRates#TIME_CONFLATED}.
     * 
     * @return the rateInfo
     */
    public int rateInfo();
}
