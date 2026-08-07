/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.EmaRdm;

import java.nio.ByteBuffer;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.LocalTime;

/**
 * Represents the RDM Source Directory service data filter.
 * <p>
 * This filter conveys an optional directory-data payload that applies to all
 * items provided by a service. When present, the payload is represented by a
 * required pair of members:
 * <ul>
 *     <li>{@link #type()} - the RDM directory data type</li>
 *     <li>{@link #data()} - the encoded EMA payload associated with that type</li>
 * </ul>
 * Callers should use {@link #checkHasData()} before accessing {@link #data()} or
 * {@link #dataType()}.
 *
 * @see DirectoryServiceFilter
 */
public interface DirectoryServiceData extends DirectoryServiceFilter<ElementList>
{
    /**
     * Clears this service-data filter and resets it to its default state.
     * <p>
     * This removes any current payload, restores the default directory data type,
     * and resets the filter-entry action.
     *
     * @return this directory service data filter instance
     */
    @Override
    DirectoryServiceData clear();

    /**
     * Sets the filter-entry action associated with this service-data filter.
     * <p>
     * Valid values are defined by the concrete implementation and typically come
     * from {@link com.refinitiv.ema.access.FilterEntry.FilterAction}.
     *
     * @param action the filter-entry action
     * @return this directory service data filter instance
     */
    @Override
    DirectoryServiceData action(int action);

    /**
     * Decodes an {@link ElementList} payload into this service-data filter.
     * <p>
     * Implementations replace the current content with the decoded directory-data
     * payload.
     *
     * @param struct encoded {@link ElementList} representing the service-data filter
     * @return this directory service data filter instance
     */
    @Override
    DirectoryServiceData decode(ElementList struct);

    /**
     * Indicates whether this filter currently contains a data payload.
     * <p>
     * When this method returns {@code true}, both {@link #type()} and
     * {@link #data()} describe the current payload. When it returns {@code false},
     * {@link #data()} and {@link #dataType()} must not be called.
     *
     * @return {@code true} if a data payload is present; otherwise, {@code false}
     */
    boolean checkHasData();

    /**
     * Replaces the contents of this object with a deep copy of another service
     * data filter.
     * <p>
     * On success, this object is cleared and then populated with the source
     * filter's {@link #action()}, and, when present, its {@link #type()} and
     * deep-copied {@link #data()} payload.
     *
     * @param sourceServiceData source service data filter to copy from
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceData} is {@code null}
     */
    DirectoryServiceData copy(DirectoryServiceData sourceServiceData);

    /**
     * Returns the EMA data type of the current payload.
     * <p>
     * The returned value corresponds to the runtime type of {@link #data()} and is
     * populated from {@link DataType.DataTypes}.
     *
     * @return the current EMA payload type from {@link DataType.DataTypes}
     * @throws OmmInvalidUsageException if {@link #checkHasData()} returns {@code false}
     */
    int dataType();

    /**
     * Returns the current RDM directory data type for this filter.
     * <p>
     * Valid values are in the inclusive range {@code 0..1023}, which includes
     * the currently defined {@link EmaRdm.DataTypes} constants.
     * After {@link #clear()}, the default value is {@link EmaRdm.DataTypes#NONE}.
     *
     * @return current directory data type in the inclusive range {@code 0..1023}
     */
    int type();

    /**
     * Sets the RDM directory data type for this filter.
     * <p>
     * Valid values are in the inclusive range {@code 0..1023}, which includes
     * the currently defined {@link EmaRdm.DataTypes} constants.
     *
     * @param type directory data type value in the inclusive range {@code 0..1023}
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code type} is outside the inclusive range {@code 0..1023}
     */
    DirectoryServiceData type(int type);

    /**
     * Returns the current payload to be applied to all items provided by this service.
     * <p>
     * The returned object is the EMA representation of the payload currently held by
     * this filter. Its runtime type is described by {@link #dataType()}.
     *
     * @return the current payload
     * @throws OmmInvalidUsageException if {@link #checkHasData()} returns {@code false}
     */
    Data data();

    /**
     * Sets encoded data that represents int type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     */
    DirectoryServiceData dataAsInt(long data);

    /**
     * Sets encoded data that represents unsigned int type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     */
    DirectoryServiceData dataAsUInt(long data);

    /**
     * Sets encoded data that represents float type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     */
    DirectoryServiceData dataAsFloat(float data);

    /**
     * Sets encoded data that represents double type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     */
    DirectoryServiceData dataAsDouble(double data);

    /**
     * Sets encoded data that represents buffer type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if ByteBuffer parameter is null
     */
    DirectoryServiceData dataAsBuffer(ByteBuffer data);

    /**
     * Sets encoded data that represents ascii type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if String parameter is null
     */
    DirectoryServiceData dataAsAscii(String data);

    /**
     * Sets encoded data that represents Utf8 string type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if String parameter is null
     */
    DirectoryServiceData dataAsUtf8(String data);

    /**
     * Sets encoded data that represents Rmtes string type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if RmtesBuffer parameter is null
     */
    DirectoryServiceData dataAsRmtes(RmtesBuffer data);

    /**
     * Sets encoded data that represents real type for this service.
     *
     * @param mantissa the mantissa
     * @param magnitudeType the magnitude type
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if MagnitudeType parameter is out of range {@link OmmReal.MagnitudeType}
     */
    DirectoryServiceData dataAsReal(long mantissa, int magnitudeType);

    /**
     * Sets encoded data that represents date type for this service.
     *
     * @param year the year
     * @param month the month
     * @param day the day
     * @return this directory service data filter instance
     */
    DirectoryServiceData dataAsDate(int year, int month, int day);

    /**
     * Sets encoded data that represents date type for this service.
     *
     * @param date the date
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if LocalDate parameter is null
     */
    DirectoryServiceData dataAsDate(LocalDate date);

    /**
     * Sets encoded data that represents time type for this service.
     *
     * @param hour the hour
     * @param minute the minute
     * @param second the second
     * @param millisecond the millisecond
     * @param microsecond the microsecond
     * @param nanosecond the nanosecond
     * @return this directory service data filter instance
     */
    DirectoryServiceData dataAsTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);

    /**
     * Sets encoded data that represents time type for this service.
     *
     * @param time the time
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if LocalTime parameter is null
     */
    DirectoryServiceData dataAsTime(LocalTime time);

    /**
     * Sets encoded data that represents date and time type for this service.
     *
     * @param year the year
     * @param month the month
     * @param day the day
     * @param hour the hour
     * @param minute the minute
     * @param second the second
     * @param millisecond the millisecond
     * @param microsecond the microsecond
     * @param nanosecond the nanosecond
     * @return this directory service data filter instance
     */
    DirectoryServiceData dataAsDateTime(int year, int month, int day, int hour, int minute, int second,
                               int millisecond, int microsecond, int nanosecond);

    /**
     * Sets encoded data that represents date and time type for this service.
     *
     * @param dateTime the dateTime
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if LocalDateTime parameter is null
     */
    DirectoryServiceData dataAsDateTime(LocalDateTime dateTime);

    /**
     * Sets encoded data that represents Qos type for this service.
     *
     * @param timeliness the timeliness
     * @param rate the rate
     * @return this directory service data filter instance
     */
    DirectoryServiceData dataAsQos(int timeliness, int rate);

    /**
     * Sets encoded data that represents State type for this service.
     *
     * @param streamState represents OmmState StreamState
     * @param dataState represents OmmState DataState
     * @param statusCode represents OmmState Status
     * @param statusText represents OmmState Text
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if String parameter is null
     */
    DirectoryServiceData dataAsState(int streamState, int dataState, int statusCode, String statusText);

    /**
     * Sets encoded data that represents enum type for this service.
     *
     * @param data the data
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if MagnitudeType parameter is out of range from 0 to 65,535
     */
    DirectoryServiceData dataAsEnum(int data);

    /**
     * Sets this filter payload from an EMA {@code ARRAY} value.
     * <p>
     * The supplied array's contents are copied into a newly created
     * {@link OmmArray} owned by this object.
     *
     * @param array array payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code array} is {@code null} or is
     *                                  not an EMA-created data instance
     */
    DirectoryServiceData dataAsArray(OmmArray array);

    /**
     * Sets this filter's payload from a {@link ComplexType} value.
     * <p>
     * The supplied payload's encoded representation is copied into this object.
     * Supported values include EMA complex/container types such as
     * {@link FieldList}, {@link Map}, {@link ElementList}, {@link FilterList},
     * {@link Vector}, {@link Series}, {@link OmmOpaque}, {@link OmmAnsiPage},
     * {@link OmmXml}, {@link OmmJson}, message types, and a no-data payload.
     * Array payloads must be supplied through {@link #dataAsArray(OmmArray)}.
     *
     * @param data the complex payload to copy into this filter
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code data} is {@code null}, if its
     *                                  type is not supported by this method, or
     *                                  if a non-{@code NO_DATA} payload is not an
     *                                  EMA-created data instance
     */
    DirectoryServiceData dataAsComplexType(ComplexType data);
}
