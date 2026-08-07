/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceData;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.codec.Double;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Float;

import java.nio.ByteBuffer;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.util.Iterator;

import static com.refinitiv.ema.access.DirectoryValidators.*;
import static com.refinitiv.ema.access.OmmReal.MagnitudeType.*;

/**
 * Default implementation of {@link DirectoryServiceData} for the RDM Source
 * Directory service data filter.
 * <p>
 * This filter carries an optional payload described by the required pair of
 * elements {@link EmaRdm#ENAME_TYPE TYPE} and {@link EmaRdm#ENAME_DATA DATA}.
 * When a payload is present, {@code TYPE} contains one of the supported
 * {@link EmaRdm.DataTypes} values and {@code DATA} contains the corresponding
 * EMA payload. When no payload is present, {@link #encode()} returns an empty
 * {@link ElementList}.
 * <p>
 * The filter {@link #action()} uses {@link FilterEntry.FilterAction} semantics.
 * It is metadata for the enclosing service filter entry and is therefore not
 * encoded by {@link #encode()} or populated by {@link #decode(ElementList)}.
 *
 * @see DirectoryServiceData
 */
final class DirectoryServiceDataImpl implements DirectoryServiceData
{
    private Data data;
    private boolean hasData;
    private int type;
    private int action;
    private final EncodeIterator encIterator = CodecFactory.createEncodeIterator();
    private final DecodeIterator decIterator = CodecFactory.createDecodeIterator();
    private final Buffer buffer = CodecFactory.createBuffer();
    private final EmaObjectManager objManager = new EmaObjectManager();
    private final StringBuilder stringBuilder = new StringBuilder();

    private static final int ENCODE_BUFFER_SIZE = 15;
    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    /**
     * Creates a service data filter initialized to the same defaults as
     * {@link #clear()}.
     */
    public DirectoryServiceDataImpl()
    {
        clear();
    }

    /**
     * Resets this filter to its default state.
     * <p>
     * After this call, no payload is present, {@link #type()} is
     * {@link EmaRdm.DataTypes#NONE}, and {@link #action()} is
     * {@link FilterEntry.FilterAction#SET}.
     *
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData clear()
    {
        hasData = false;
        data = null;
        action = FilterEntry.FilterAction.SET;
        type = EmaRdm.DataTypes.NONE;

        encIterator.clear();
        decIterator.clear();
        buffer.clear();
        return this;
    }

    /**
     * Applies data presence flag.
     *
     */
    private void applyHasData()
    {
        hasData = true;
    }

    /**
     * Indicates whether this filter currently contains a payload.
     * <p>
     * When this method returns {@code true}, both {@link #type()} and
     * {@link #data()} describe the currently stored directory-data payload.
     *
     * @return {@code true} if payload data has been assigned; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasData()
    {
        return hasData;
    }

    /**
     * Encodes this filter as an {@link ElementList}.
     * <p>
     * When payload data is present, the returned list contains exactly the
     * required {@code TYPE} and {@code DATA} elements. When no payload is
     * present, the returned list is empty.
     * <p>
     * The filter {@link #action()} is not encoded here; it is handled by the
     * enclosing {@link FilterEntry} managed by {@link DirectoryServiceImpl}.
     *
     * @return encoded element list representing this service data filter
     */
    @Override
    public ElementList encode()
    {
        ElementList elementList = EmaFactory.createElementList();

        if (checkHasData())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, type));

            ElementEntry elementEntry = EmaFactory.createElementEntry();
            switch (data.dataType())
            {
                case DataType.DataTypes.INT :
                    elementEntry.intValue(EmaRdm.ENAME_DATA, ((OmmInt)data).intValue());
                    break;
                case DataType.DataTypes.UINT :
                    elementEntry.uintValue(EmaRdm.ENAME_DATA, ((OmmUInt)data).longValue());
                    break;
                case DataType.DataTypes.FLOAT :
                    elementEntry.floatValue(EmaRdm.ENAME_DATA, ((OmmFloat)data).floatValue());
                    break;
                case DataType.DataTypes.DOUBLE :
                    elementEntry.doubleValue(EmaRdm.ENAME_DATA, ((OmmDouble)data).doubleValue());
                    break;
                case DataType.DataTypes.BUFFER :
                    elementEntry.buffer(EmaRdm.ENAME_DATA, ((OmmBuffer)data).buffer());
                    break;
                case DataType.DataTypes.ASCII :
                    elementEntry.ascii(EmaRdm.ENAME_DATA, ((OmmAscii)data).ascii());
                    break;
                case DataType.DataTypes.UTF8 :
                    elementEntry.utf8(EmaRdm.ENAME_DATA, ((OmmUtf8)data).buffer());
                    break;
                case DataType.DataTypes.RMTES :
                    elementEntry.rmtes(EmaRdm.ENAME_DATA, ((OmmRmtes) data).rmtes().asHex().flip());
                    break;
                case DataType.DataTypes.REAL :
                    elementEntry.real(EmaRdm.ENAME_DATA, ((OmmReal)data).mantissa(), ((OmmReal)data).magnitudeType());
                    break;
                case DataType.DataTypes.DATE :
                    elementEntry.date(EmaRdm.ENAME_DATA, ((OmmDate)data).year(), ((OmmDate)data).month(),
                            ((OmmDate)data).day());
                    break;
                case DataType.DataTypes.TIME :
                    elementEntry.time(EmaRdm.ENAME_DATA,  ((OmmTime)data).hour(), ((OmmTime)data).minute(),
                            ((OmmTime)data).second(), ((OmmTime)data).millisecond(), ((OmmTime)data).microsecond(),
                            ((OmmTime)data).nanosecond());
                    break;
                case DataType.DataTypes.DATETIME :
                    elementEntry.dateTime(EmaRdm.ENAME_DATA, ((OmmDateTime)data).year(), ((OmmDateTime)data).month(),
                            ((OmmDateTime)data).day(), ((OmmDateTime)data).hour(), ((OmmDateTime)data).minute(),
                            ((OmmDateTime)data).second(), ((OmmDateTime)data).millisecond(),
                            ((OmmDateTime)data).microsecond(), ((OmmDateTime)data).nanosecond());
                    break;
                case DataType.DataTypes.QOS :
                    elementEntry.qos(EmaRdm.ENAME_DATA, ((OmmQos)data).timeliness(), ((OmmQos)data).rate());
                    break;
                case DataType.DataTypes.STATE :
                    elementEntry.state(EmaRdm.ENAME_DATA, ((OmmState)data).streamState(), ((OmmState)data).dataState(),
                            ((OmmState)data).statusCode(), ((OmmState)data).statusText());
                    break;
                case DataType.DataTypes.ENUM :
                    elementEntry.enumValue(EmaRdm.ENAME_DATA, ((OmmEnum)data).enumValue());
                    break;
                case DataType.DataTypes.ARRAY :
                    elementEntry.array(EmaRdm.ENAME_DATA, ((OmmArray)data));
                    break;
                case DataType.DataTypes.FIELD_LIST :
                    elementEntry.fieldList(EmaRdm.ENAME_DATA, ((FieldList)data));
                    break;
                case DataType.DataTypes.MAP :
                    elementEntry.map(EmaRdm.ENAME_DATA, ((Map)data));
                    break;
                case DataType.DataTypes.ELEMENT_LIST :
                    elementEntry.elementList(EmaRdm.ENAME_DATA, ((ElementList)data));
                    break;
                case DataType.DataTypes.FILTER_LIST :
                    elementEntry.filterList(EmaRdm.ENAME_DATA, ((FilterList)data));
                    break;
                case DataType.DataTypes.VECTOR :
                    elementEntry.vector(EmaRdm.ENAME_DATA, ((Vector)data));
                    break;
                case DataType.DataTypes.SERIES :
                    elementEntry.series(EmaRdm.ENAME_DATA, ((Series)data));
                    break;
                case DataType.DataTypes.OPAQUE :
                    elementEntry.opaque(EmaRdm.ENAME_DATA, ((OmmOpaque)data));
                    break;
                case DataType.DataTypes.ANSI_PAGE :
                    elementEntry.ansiPage(EmaRdm.ENAME_DATA, ((OmmAnsiPage)data));
                    break;
                case DataType.DataTypes.XML :
                    elementEntry.xml(EmaRdm.ENAME_DATA, ((OmmXml)data));
                    break;
                case DataType.DataTypes.JSON :
                    elementEntry.json(EmaRdm.ENAME_DATA, ((OmmJson)data));
                    break;
                case DataType.DataTypes.REQ_MSG :
                    elementEntry.reqMsg(EmaRdm.ENAME_DATA, ((ReqMsg)data));
                    break;
                case DataType.DataTypes.REFRESH_MSG :
                    elementEntry.refreshMsg(EmaRdm.ENAME_DATA, ((RefreshMsg)data));
                    break;
                case DataType.DataTypes.STATUS_MSG :
                    elementEntry.statusMsg(EmaRdm.ENAME_DATA, ((StatusMsg)data));
                    break;
                case DataType.DataTypes.UPDATE_MSG :
                    elementEntry.updateMsg(EmaRdm.ENAME_DATA, ((UpdateMsg)data));
                    break;
                case DataType.DataTypes.ACK_MSG :
                    elementEntry.ackMsg(EmaRdm.ENAME_DATA, ((AckMsg)data));
                    break;
                case DataType.DataTypes.POST_MSG :
                    elementEntry.postMsg(EmaRdm.ENAME_DATA, ((PostMsg)data));
                    break;
                case DataType.DataTypes.GENERIC_MSG :
                    elementEntry.genericMsg(EmaRdm.ENAME_DATA, ((GenericMsg)data));
                    break;
                case DataType.DataTypes.NO_DATA :
                default:
                    elementEntry.noData(EmaRdm.ENAME_DATA);
                    break;
            }
            elementList.add(elementEntry);
         }

        return elementList;
    }

    /**
     * Decodes an {@link ElementList} into this service data filter.
     * <p>
     * The supplied element list may either be empty, meaning no payload is
     * present, or contain the required pair of elements
     * {@link EmaRdm#ENAME_TYPE TYPE} and {@link EmaRdm#ENAME_DATA DATA}. If the
     * input is malformed, contains an unsupported directory data type, or contains
     * only one member of the required pair, an {@link OmmInvalidUsageException}
     * is thrown. If decoding fails, this object remains in inconsistent state and
     * should be cleared for further usage.
     * <p>
     * The filter {@link #action()} is unaffected by this method and is expected
     * to be managed by the enclosing filter entry.
     *
     * @param elementList encoded element list representing the service data filter
     * @return this directory service data filter instance
     *
     * @throws OmmInvalidUsageException if {@code elementList} is {@code null} or
     *                                  cannot be decoded into a valid service
     *                                  data payload
     */
    @Override
    public DirectoryServiceData decode(ElementList elementList)
    {
        if (elementList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("elementList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        Iterator<ElementEntry> iterator = elementList.iteratorByRef();
        ElementEntry elementEntry;
        String elementName;

        boolean foundType = false;
        boolean foundData = false;
        while (iterator.hasNext())
        {
            elementEntry = iterator.next();
            elementName = elementEntry.name();

            switch (elementName)
            {
                case EmaRdm.ENAME_TYPE:
                    long type = elementEntry.uintValue();
                    if (isValidDataType((int) type))
                    {
                        this.type = (int) type;
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    foundType = true;
                    break;
                case EmaRdm.ENAME_DATA:
                    Data data = elementEntry.load();
                    setData(data);
                    foundData = true;
                    break;
                default:
                    break;
            }
        }

        // If Data element is present, type must be too.
        if ((foundData && !foundType) || (foundType && !foundData))
        {
            throw new OmmInvalidUsageExceptionImpl().message("If " + EmaRdm.ENAME_DATA + " is present, " +
                    EmaRdm.ENAME_TYPE + " must be too", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);

        }
        return this;
    }

    private void setData(Data data)
    {
        switch (data.dataType())
        {
            case DataType.DataTypes.INT :
                dataAsInt(((OmmInt) data).intValue());
                break;
            case DataType.DataTypes.UINT :
                dataAsUInt(((OmmUInt) data).longValue());
                break;
            case DataType.DataTypes.FLOAT :
                dataAsFloat(((OmmFloat) data).floatValue());
                break;
            case DataType.DataTypes.DOUBLE :
                dataAsDouble(((OmmDouble) data).doubleValue());
                break;
            case DataType.DataTypes.BUFFER :
                dataAsBuffer(((OmmBuffer) data).buffer());
                break;
            case DataType.DataTypes.ASCII :
                dataAsAscii(((OmmAscii) data).ascii());
                break;
            case DataType.DataTypes.UTF8 :
                dataAsUtf8(((OmmUtf8) data).string());
                break;
            case DataType.DataTypes.RMTES :
                dataAsRmtes(((OmmRmtes) data).rmtes());
                break;
            case DataType.DataTypes.REAL :
                dataAsReal(((OmmReal) data).mantissa(), ((OmmReal) data).magnitudeType());
                break;
            case DataType.DataTypes.DATE :
                dataAsDate(((OmmDate) data).year(), ((OmmDate) data).month(), ((OmmDate) data).day());
                break;
            case DataType.DataTypes.TIME :
                dataAsTime(((OmmTime) data).hour(), ((OmmTime) data).minute(), ((OmmTime) data).second(),
                        ((OmmTime) data).millisecond(), ((OmmTime) data).microsecond(), ((OmmTime) data).nanosecond());
                break;
            case DataType.DataTypes.DATETIME :
                dataAsDateTime(((OmmDateTime) data).year(), ((OmmDateTime) data).month(), ((OmmDateTime) data).day(),
                        ((OmmDateTime) data).hour(), ((OmmDateTime) data).minute(), ((OmmDateTime) data).second(),
                        ((OmmDateTime) data).millisecond(), ((OmmDateTime) data).microsecond(),
                        ((OmmDateTime) data).nanosecond());
                break;
            case DataType.DataTypes.QOS :
                dataAsQos(((OmmQos) data).timeliness(), ((OmmQos) data).rate());
                break;
            case DataType.DataTypes.STATE :
                dataAsState(((OmmState)data).streamState(), ((OmmState)data).dataState(), ((OmmState)data).statusCode(),
                        ((OmmState)data).statusText());
                break;
            case DataType.DataTypes.ENUM :
                dataAsEnum(((OmmEnum) data).enumValue());
                break;
            case DataType.DataTypes.ARRAY :
                dataAsArray(((OmmArray) data));
                break;
            case DataType.DataTypes.FIELD_LIST :
            case DataType.DataTypes.MAP :
            case DataType.DataTypes.ELEMENT_LIST :
            case DataType.DataTypes.FILTER_LIST :
            case DataType.DataTypes.VECTOR :
            case DataType.DataTypes.SERIES :
            case DataType.DataTypes.OPAQUE :
            case DataType.DataTypes.ANSI_PAGE :
            case DataType.DataTypes.XML :
            case DataType.DataTypes.JSON :
            case DataType.DataTypes.REQ_MSG :
            case DataType.DataTypes.REFRESH_MSG :
            case DataType.DataTypes.STATUS_MSG :
            case DataType.DataTypes.UPDATE_MSG :
            case DataType.DataTypes.ACK_MSG :
            case DataType.DataTypes.POST_MSG :
            case DataType.DataTypes.GENERIC_MSG :
            case DataType.DataTypes.NO_DATA :
                dataAsComplexType((ComplexType) data);
                break;
            default:
                throw new OmmInvalidUsageExceptionImpl().message("Unsupported data type " + data.dataType(),
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    /**
     * Replaces the contents of this object with a deep copy of another service
     * data filter.
     * <p>
     * If {@code sourceServiceData} is this object, the call succeeds without
     * modifying the current state. Otherwise, this object is reset before values
     * are copied from the source.
     *
     * @param sourceServiceData source service data filter to copy from
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceData} is {@code null}
     */
    @Override
    public DirectoryServiceData copy(DirectoryServiceData sourceServiceData)
    {
        if (sourceServiceData == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceServiceData can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceServiceData == this)
        {
            return this;
        }

        clear();

        action(sourceServiceData.action());

        if (sourceServiceData.checkHasData())
        {
            type(sourceServiceData.type());
            setData(sourceServiceData.data());
        }
        return this;
    }

    /**
     * Sets the {@link FilterEntry.FilterAction} associated with this filter.
     * <p>
     * The action is metadata for the enclosing service filter entry. Valid
     * values are {@link FilterEntry.FilterAction#SET},
     * {@link FilterEntry.FilterAction#UPDATE}, and
     * {@link FilterEntry.FilterAction#CLEAR}.
     *
     * @param action filter entry action for this data filter
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code action} is not a supported
     *                                  filter-entry action
     */
    @Override
    public DirectoryServiceData action(int action)
    {
        if (!isValidFilterEntryAction(action))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + action,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.action = action;
        return this;
    }

    /**
     * Returns the {@link FilterEntry.FilterAction} associated with this filter.
     *
     * @return current filter-entry action metadata
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Returns the EMA data type of the current payload.
     *
     * @return current payload data type from {@link DataType.DataTypes}
     * @throws OmmInvalidUsageException if no payload is present
     */
    @Override
    public int dataType()
    {
        return data().dataType();
    }

    /**
     * Returns the current RDM directory data type for this filter.
     * <p>
     * Valid values are in the inclusive range {@code 0..1023}, which includes
     * the currently defined {@link EmaRdm.DataTypes} constants.
     *
     * @return current directory data type in the inclusive range {@code 0..1023}
     */
    @Override
    public int type()
    {
        return type;
    }

    /**
     * Sets the RDM directory data type for this filter.
     * <p>
     * Valid values are in the inclusive range {@code 0..1023}, which includes
     * the currently defined {@link EmaRdm.DataTypes} constants.
     *
     * @param type directory data type value in the inclusive range {@code 0..1023}
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code type} is outside the inclusive
     *                                  range {@code 0..1023}
     */
    @Override
    public DirectoryServiceData type(int type)
    {
        if (!isValidDataType(type))
        {
            throw new OmmInvalidUsageExceptionImpl().message("type must be in range 0..1023",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.type = type;
        return this;
    }

    /**
     * Returns the current payload for this filter.
     * <p>
     * The returned object is the EMA representation of the payload associated
     * with the current {@link #type()}.
     *
     * @return current payload data
     * @throws OmmInvalidUsageException if no payload is present
     */
    @Override
    public Data data()
    {
        if (!checkHasData())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_DATA + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return data;
    }

    /**
     * Sets this filter payload to an EMA {@code INT} value.
     * <p>
     * This implementation encodes the supplied value as ETA {@link Int},
     * decodes it back into an EMA {@link OmmInt}, stores it as this filter's
     * payload, and marks the payload as present.
     *
     * @param data signed integer payload value
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData dataAsInt(long data)
    {
        Int integer = CodecFactory.createInt();
        integer.value(data);

        this.data = decodeEncodedValue(integer::encode, new OmmIntImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code UINT} value.
     * <p>
     * This implementation encodes the supplied value as ETA {@link UInt},
     * decodes it back into an EMA {@link OmmUInt}, stores it as this filter's
     * payload, and marks the payload as present.
     *
     * @param data unsigned integer payload value
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData dataAsUInt(long data)
    {
        UInt uint = CodecFactory.createUInt();
        uint.value(data);

        this.data = decodeEncodedValue(uint::encode, new OmmUIntImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code FLOAT} value.
     * <p>
     * This implementation encodes the supplied value as ETA {@link Float},
     * decodes it back into an EMA {@link OmmFloat}, stores it as this filter's
     * payload, and marks the payload as present.
     *
     * @param data floating-point payload value
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData dataAsFloat(float data)
    {
        Float flt = CodecFactory.createFloat();
        flt.value(data);

        this.data = decodeEncodedValue(flt::encode, new OmmFloatImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code DOUBLE} value.
     * <p>
     * This implementation encodes the supplied value as ETA {@link Double},
     * decodes it back into an EMA {@link OmmDouble}, stores it as this filter's
     * payload, and marks the payload as present.
     *
     * @param data double-precision payload value
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData dataAsDouble(double data)
    {
        Double dbl = CodecFactory.createDouble();
        dbl.value(data);

        this.data = decodeEncodedValue(dbl::encode, new OmmDoubleImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code BUFFER} value.
     * <p>
     * The supplied buffer is copied into this object's encoded payload storage,
     * so later changes to the source buffer do not affect the stored payload.
     *
     * @param data buffer payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code data} is {@code null}
     */
    @Override
    public DirectoryServiceData dataAsBuffer(ByteBuffer data)
    {
        if (data == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("data can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        buffer.data(data);

        this.data = new OmmBufferImpl();
        Utilities.copy(buffer, ((DataImpl) this.data)._rsslBuffer);
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code ASCII} string value.
     * <p>
     * The supplied text is copied into this object's encoded payload storage
     * and the payload-presence flag is set.
     *
     * @param data ASCII payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code data} is {@code null}
     */
    @Override
    public DirectoryServiceData dataAsAscii(String data)
    {
        if (data == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("data can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        buffer.data(data);

        this.data = new OmmAsciiImpl();
        Utilities.copy(buffer, ((DataImpl) this.data)._rsslBuffer);
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code UTF8} string value.
     * <p>
     * The supplied text is copied into this object's encoded payload storage
     * and the payload-presence flag is set.
     *
     * @param data UTF-8 payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code data} is {@code null}
     */
    @Override
    public DirectoryServiceData dataAsUtf8(String data)
    {
        if (data == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("data can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        buffer.data(data);

        this.data = new OmmUtf8Impl();
        Utilities.copy(buffer, ((DataImpl) this.data)._rsslBuffer);
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code RMTES} string value.
     * <p>
     * This implementation creates a new {@link OmmRmtes} instance, applies the
     * supplied {@link RmtesBuffer} to its internal buffer, stores it as this
     * filter's payload, and marks the payload as present.
     *
     * @param data RMTES payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code data} is {@code null}
     */
    @Override
    public DirectoryServiceData dataAsRmtes(RmtesBuffer data)
    {
        if (data == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("data can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        OmmRmtesImpl ommRmtes = new OmmRmtesImpl();
        RmtesBuffer rmtesBuffer = ommRmtes.rmtes();
        rmtesBuffer.apply(data);
        this.data = ommRmtes;

        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code REAL} value.
     * <p>
     * The supplied mantissa and magnitude are validated, encoded as ETA
     * {@link Real}, decoded back into an EMA {@link OmmReal}, stored as this
     * filter's payload, and marked as present.
     *
     * @param mantissa real-value mantissa
     * @param magnitudeType real-value magnitude from
     *                      {@link OmmReal.MagnitudeType}
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code magnitudeType} is outside the
     *                                  supported {@link OmmReal.MagnitudeType}
     *                                  range
     */
    @Override
    public DirectoryServiceData dataAsReal(long mantissa, int magnitudeType)
    {
        if (magnitudeType < EXPONENT_NEG_14 || magnitudeType > NOT_A_NUMBER)
        {
            throw new OmmInvalidUsageExceptionImpl().message("magnitudeType should be in range of MagnitudeType values",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        Real real = CodecFactory.createReal();
        real.value(mantissa, magnitudeType);

        this.data = decodeEncodedValue(real::encode, new OmmRealImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code DATE} value.
     * <p>
     * This implementation encodes the supplied components as ETA {@link Date},
     * decodes them back into an EMA {@link OmmDate}, stores the result as this
     * filter's payload, and marks the payload as present.
     *
     * @param year year component
     * @param month month component
     * @param day day component
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData dataAsDate(int year, int month, int day)
    {
        Date date = CodecFactory.createDate();
        date.year(year);
        date.month(month);
        date.day(day);

        this.data = decodeEncodedValue(date::encode, new OmmDateImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code DATE} value.
     * <p>
     * The supplied {@link LocalDate} is converted to ETA {@link Date}
     * components, decoded back into an EMA {@link OmmDate}, and stored as this
     * filter's payload.
     *
     * @param date date payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code date} is {@code null}
     */
    @Override
    public DirectoryServiceData dataAsDate(LocalDate date)
    {
        if (date == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("date can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        Date etaDate = CodecFactory.createDate();
        etaDate.year(date.getYear());
        etaDate.month(date.getMonthValue());
        etaDate.day(date.getDayOfMonth());

        this.data = decodeEncodedValue(etaDate::encode, new OmmDateImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code TIME} value.
     * <p>
     * This implementation encodes the supplied time components as ETA
     * {@link Time}, decodes them back into an EMA {@link OmmTime}, stores the
     * result as this filter's payload, and marks the payload as present.
     *
     * @param hour hour component
     * @param minute minute component
     * @param second second component
     * @param millisecond millisecond component
     * @param microsecond microsecond component
     * @param nanosecond nanosecond component
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData dataAsTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
    {
        Time time = CodecFactory.createTime();
        time.hour(hour);
        time.minute(minute);
        time.second(second);
        time.millisecond(millisecond);
        time.microsecond(microsecond);
        time.nanosecond(nanosecond);

        this.data = decodeEncodedValue(time::encode, new OmmTimeImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code TIME} value.
     * <p>
     * The supplied {@link LocalTime} is split into ETA millisecond,
     * microsecond, and nanosecond components, decoded back into an EMA
     * {@link OmmTime}, and stored as this filter's payload.
     *
     * @param time time payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code time} is {@code null}
     */
    @Override
    public DirectoryServiceData dataAsTime(LocalTime time)
    {
        if (time == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("time can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        Time etaTime = CodecFactory.createTime();
        etaTime.hour(time.getHour());
        etaTime.minute(time.getMinute());
        etaTime.second(time.getSecond());
        int nanos = time.getNano() % 1000;
        int microsecond = (time.getNano() / 1000) % 1000;
        int millisecond = (time.getNano() / 1000000) % 1000;
        etaTime.millisecond(millisecond);
        etaTime.microsecond(microsecond);
        etaTime.nanosecond(nanos);

        this.data = decodeEncodedValue(etaTime::encode, new OmmTimeImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code DATETIME} value.
     * <p>
     * This implementation encodes the supplied date-time components as ETA
     * {@link DateTime}, decodes them back into an EMA {@link OmmDateTime},
     * stores the result as this filter's payload, and marks the payload as
     * present.
     *
     * @param year year component
     * @param month month component
     * @param day day component
     * @param hour hour component
     * @param minute minute component
     * @param second second component
     * @param millisecond millisecond component
     * @param microsecond microsecond component
     * @param nanosecond nanosecond component
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData dataAsDateTime(int year, int month, int day, int hour, int minute, int second,
                               int millisecond, int microsecond, int nanosecond)
    {
        DateTime dateTime = CodecFactory.createDateTime();
        dateTime.year(year);
        dateTime.month(month);
        dateTime.day(day);
        dateTime.hour(hour);
        dateTime.minute(minute);
        dateTime.second(second);
        dateTime.millisecond(millisecond);
        dateTime.microsecond(microsecond);
        dateTime.nanosecond(nanosecond);

        this.data = decodeEncodedValue(dateTime::encode, new OmmDateTimeImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code DATETIME} value.
     * <p>
     * The supplied {@link LocalDateTime} is converted to ETA date-time
     * components, with nanoseconds split into millisecond, microsecond, and
     * nanosecond parts, then decoded into an EMA {@link OmmDateTime} payload.
     *
     * @param dateTime date-time payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code dateTime} is {@code null}
     */
    @Override
    public DirectoryServiceData dataAsDateTime(LocalDateTime dateTime)
    {
        if (dateTime == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("dateTime can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        DateTime etaDateTime = CodecFactory.createDateTime();
        etaDateTime.year(dateTime.getYear());
        etaDateTime.month(dateTime.getMonthValue());
        etaDateTime.day(dateTime.getDayOfMonth());
        etaDateTime.hour(dateTime.getHour());
        etaDateTime.minute(dateTime.getMinute());
        etaDateTime.second(dateTime.getSecond());
        int nanos = dateTime.getNano() % 1000;
        int microsecond = (dateTime.getNano() / 1000) % 1000;
        int millisecond = (dateTime.getNano() / 1000000) % 1000;
        etaDateTime.millisecond(millisecond);
        etaDateTime.microsecond(microsecond);
        etaDateTime.nanosecond(nanos);

        this.data = decodeEncodedValue(etaDateTime::encode, new OmmDateTimeImpl());
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code QOS} value.
     * <p>
     * This implementation converts the supplied EMA timeliness and rate values
     * into the corresponding ETA {@link Qos} representation, decodes that ETA
     * value into an EMA {@link OmmQos}, stores it as this filter's payload, and
     * marks the payload as present.
     *
     * @param timeliness EMA timeliness value
     * @param rate EMA rate value
     * @return this directory service data filter instance
     */
    @Override
    public DirectoryServiceData dataAsQos(int timeliness, int rate)
    {
        Qos qos = CodecFactory.createQos();
        Utilities.toRsslQos(rate, timeliness, qos);

        OmmQosImpl ommQos = new OmmQosImpl();
        ommQos.decode(qos);

        this.data = ommQos;
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code STATE} value.
     * <p>
     * In addition to requiring a non-{@code null} {@code statusText}, this
     * implementation validates the supplied state components with
     * {@link DirectoryValidators#validateStatus(int, int, int)}, decodes the
     * corresponding ETA {@link State} into an EMA {@link OmmState}, and stores
     * it as this filter's payload.
     *
     * @param streamState EMA stream-state value
     * @param dataState EMA data-state value
     * @param statusCode EMA status-code value
     * @param statusText EMA status text
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null} or
     *                                  if any state component is invalid
     */
    @Override
    public DirectoryServiceData dataAsState(int streamState, int dataState, int statusCode, String statusText)
    {
        if (statusText == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("statusText can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        validateStatus(streamState, dataState, statusCode);

        State rsslState = CodecFactory.createState();
        Buffer stateText = CodecFactory.createBuffer();

        rsslState.streamState(streamState);
        rsslState.dataState(dataState);
        rsslState.code(statusCode);
        stateText.data(statusText);
        rsslState.text(stateText);

        OmmStateImpl ommState = new OmmStateImpl();
        ommState.decode(rsslState);

        this.data = ommState;
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload to an EMA {@code ENUM} value.
     * <p>
     * The supplied value must be in the inclusive range {@code 0..65535}. This
     * implementation encodes it as ETA {@link Enum}, decodes it back into an
     * EMA {@link OmmEnum}, stores it as this filter's payload, and marks the
     * payload as present.
     *
     * @param data enum payload value
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code data} is outside the inclusive
     *                                  range {@code 0..65535}
     */
    @Override
    public DirectoryServiceData dataAsEnum(int data)
    {
        if (data < 0 || data > 65535)
        {
            throw new OmmInvalidUsageExceptionImpl().message("data should be in range from 0 to 65,535",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        Enum etaEnum = CodecFactory.createEnum();
        etaEnum.value(data);

        this.data = decodeEncodedValue(etaEnum::encode, new OmmEnumImpl());
        applyHasData();
        return this;
    }

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
    @Override
    public DirectoryServiceData dataAsArray(OmmArray array)
    {
        if (array == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("array can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (!(array instanceof DataImpl))
        {
            throw new OmmInvalidUsageExceptionImpl().message("array must be an EMA data instance",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.data = new OmmArrayImpl(objManager);
        copyAndDecodeDataImpl((DataImpl) array, (DataImpl) this.data);
        applyHasData();
        return this;
    }

    /**
     * Sets this filter payload from a supported EMA {@link ComplexType}.
     * <p>
     * Supported values include EMA container types other than
     * {@link OmmArray}, message types, and {@link DataType.DataTypes#NO_DATA}.
     * Array payloads must be supplied through {@link #dataAsArray(OmmArray)}.
     * For non-{@code NO_DATA} values, the supplied payload's encoded
     * representation is copied into a new implementation instance owned by this
     * object. A {@code NO_DATA} payload is represented by a fresh
     * {@link NoDataImpl}.
     *
     * @param data complex payload to copy into this filter
     * @return this directory service data filter instance
     * @throws OmmInvalidUsageException if {@code data} is {@code null}, if its
     *                                  data type is not supported by this
     *                                  implementation, or if a non-{@code NO_DATA}
     *                                  payload is not an EMA-created data
     *                                  instance
     */
    @Override
    public DirectoryServiceData dataAsComplexType(ComplexType data)
    {
        if (data == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("data can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.data = createComplexTypeTarget(data.dataType());

        if (data.dataType() != DataType.DataTypes.NO_DATA)
        {
            if (!(data instanceof DataImpl))
            {
                throw new OmmInvalidUsageExceptionImpl().message("data must be an EMA data instance",
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
            }

            copyEncodedDataImpl((DataImpl) data, (DataImpl) this.data);
        }
        applyHasData();
        return this;
    }

    /**
     * Returns the fixed RDM filter identifier for the service data filter.
     *
     * @return {@link EmaRdm#SERVICE_DATA_ID}
     */
    @Override
    public int filterId()
    {
        return EmaRdm.SERVICE_DATA_ID;
    }

    /**
     * Returns a human-readable representation of this filter.
     * <p>
     * The returned text includes the directory {@link #type()} and, when a
     * payload is present, the payload value and EMA {@link #dataType()}.
     *
     * @return formatted string representation of this service data filter
     */
    @Override
    public String toString()
    {
        stringBuilder.setLength(0);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("DataFilter:");
        stringBuilder.append(EOL);

        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("type: ");
        stringBuilder.append(EmaRdm.DataTypes.asString(type()));
        stringBuilder.append(EOL);

        if (checkHasData())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("data: ");
            stringBuilder.append(data());
            stringBuilder.append(EOL);

            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("dataType: ");
            stringBuilder.append(DataType.asString(dataType()));
            stringBuilder.append(EOL);
        }
        return stringBuilder.toString();
    }

    @FunctionalInterface
    private interface EncodableValue
    {
        void encode(EncodeIterator iterator);
    }

    /**
     * Encodes an ETA primitive value into the shared buffer and decodes it into
     * the supplied EMA data implementation.
     *
     * @param encoder encodes the ETA value into {@link #encIterator}
     * @param target decoded EMA target instance
     * @param <T> EMA data implementation type
     * @return decoded {@code target}
     */
    private <T extends DataImpl> T decodeEncodedValue(EncodableValue encoder, T target)
    {
        encIterator.clear();
        buffer.data(ByteBuffer.allocate(ENCODE_BUFFER_SIZE));
        encIterator.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        encoder.encode(encIterator);

        decIterator.clear();
        decIterator.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        target.decode(buffer, decIterator);
        return target;
    }

    /**
     * Creates a destination implementation suitable for storing a copied complex
     * EMA payload.
     *
     * @param dataType EMA data type from {@link DataType.DataTypes}
     * @return destination implementation for the specified type
     */
    private DataImpl createComplexTypeTarget(int dataType)
    {
        switch (dataType)
        {
            case DataType.DataTypes.FIELD_LIST:
                return new FieldListImpl(objManager);
            case DataType.DataTypes.MAP:
                return new MapImpl(objManager);
            case DataType.DataTypes.ELEMENT_LIST:
                return new ElementListImpl(objManager);
            case DataType.DataTypes.FILTER_LIST:
                return new FilterListImpl(objManager);
            case DataType.DataTypes.VECTOR:
                return new VectorImpl(objManager);
            case DataType.DataTypes.SERIES:
                return new SeriesImpl(objManager);
            case DataType.DataTypes.OPAQUE:
                return new OmmOpaqueImpl();
            case DataType.DataTypes.ANSI_PAGE:
                return new OmmAnsiPageImpl();
            case DataType.DataTypes.XML:
                return new OmmXmlImpl();
            case DataType.DataTypes.JSON:
                return new OmmJsonImpl();
            case DataType.DataTypes.REQ_MSG:
                return new ReqMsgImpl(objManager);
            case DataType.DataTypes.REFRESH_MSG:
                return new RefreshMsgImpl(objManager);
            case DataType.DataTypes.STATUS_MSG:
                return new StatusMsgImpl(objManager);
            case DataType.DataTypes.UPDATE_MSG:
                return new UpdateMsgImpl(objManager);
            case DataType.DataTypes.ACK_MSG:
                return new AckMsgImpl(objManager);
            case DataType.DataTypes.POST_MSG:
                return new PostMsgImpl(objManager);
            case DataType.DataTypes.GENERIC_MSG:
                return new GenericMsgImpl(objManager);
            case DataType.DataTypes.NO_DATA:
                return new NoDataImpl();
            default:
                throw new OmmInvalidUsageExceptionImpl().message(dataType +
                        " data type is not supported", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    /**
     * Copies encoded data and eagerly decodes it using the source object's
     * version and decode context.
     * <p>
     * This helper is intended for payloads, such as {@link OmmArray}, that are
     * safe to materialize immediately after copying.
     *
     * @param source source EMA implementation
     * @param destination destination implementation that receives the copy
     */
    private void copyAndDecodeDataImpl(DataImpl source, DataImpl destination)
    {
        com.refinitiv.eta.codec.DataDictionary rsslDictionary = null;
        Object localSetDefDb = null;
        if (source instanceof CollectionDataImpl)
        {
            CollectionDataImpl sourceCollection = (CollectionDataImpl) source;
            rsslDictionary = sourceCollection._rsslDictionary;

            switch (sourceCollection.dataType())
            {
                case DataType.DataTypes.FIELD_LIST:
                    localSetDefDb = sourceCollection._rsslLocalFLSetDefDb;
                    break;
                case DataType.DataTypes.ELEMENT_LIST:
                    localSetDefDb = sourceCollection._rsslLocalELSetDefDb;
                    break;
                default:
                    localSetDefDb = sourceCollection._rsslLocalSetDefDb;
                    break;
            }
        }
        else if (source instanceof MsgImpl)
        {
            rsslDictionary = ((MsgImpl) source)._rsslDictionary;
        }

        copyEncodedDataImpl(source, destination);
        destination._rsslMajVer = source._rsslMajVer;
        destination._rsslMinVer = source._rsslMinVer;
        destination.decode(destination._rsslBuffer, source._rsslMajVer, source._rsslMinVer, rsslDictionary, localSetDefDb);
    }

    /**
     * Copies the encoded representation of one EMA implementation object into
     * another without decoding it.
     *
     * @param source source EMA implementation
     * @param destination destination EMA implementation
     */
    private void copyEncodedDataImpl(DataImpl source, DataImpl destination)
    {
        if (destination._rsslBuffer == null)
        {
            destination._rsslBuffer = CodecFactory.createBuffer();
        }

        Utilities.copy(source.encodedData(), destination._rsslBuffer);
        destination._rsslMajVer = source._rsslMajVer;
        destination._rsslMinVer = source._rsslMinVer;
    }
}
