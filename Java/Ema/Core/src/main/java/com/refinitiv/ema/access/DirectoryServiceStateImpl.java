/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceState;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.*;

import java.util.BitSet;
import java.util.Iterator;

import static com.refinitiv.ema.access.DirectoryValidators.*;

/**
 * Default implementation of {@link DirectoryServiceState} for the RDM Source
 * Directory service state filter.
 * <p>
 * The filter conveys whether a service is up or down and may also include the
 * optional {@code acceptingRequests} and {@code status} elements.
 *
 * @see DirectoryServiceState
 */
final class DirectoryServiceStateImpl implements DirectoryServiceState
{
    private final BitSet flags = new BitSet();
    private int action;
    private final State rsslState = CodecFactory.createState();
    private final Buffer stateText = CodecFactory.createBuffer();
    private final OmmStateImpl status = new OmmStateImpl();
    private final StringBuilder stringBuilder = new StringBuilder();

    private static final int SERVICE_STATE_UP_FLAG = 0;
    private static final int ACCEPTING_REQUESTS_YES_FLAG = 1;
    private static final int HAS_ACCEPTING_REQUESTS_FLAG = 2;
    private static final int HAS_STATUS_FLAG = 3;
    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    /**
     * Creates a service state filter initialized to the same defaults as
     * {@link #clear()}.
     */
    public DirectoryServiceStateImpl()
    {
        clear();
    }

    /**
     * Resets this filter to its default state.
     * <p>
     * After this call, {@link #serviceState()} is {@link EmaRdm#SERVICE_UP},
     * {@link #action()} is {@link FilterEntry.FilterAction#SET}, and the
     * optional {@code acceptingRequests} and {@code status} elements are marked
     * as absent.
     *
     * @return this directory service state filter instance
     */
    @Override
    public DirectoryServiceState clear()
    {
        flags.clear();
        flags.set(SERVICE_STATE_UP_FLAG); // serviceState = SERVICE_UP
        flags.set(ACCEPTING_REQUESTS_YES_FLAG); // acceptingRequests = SERVICE_YES
        action = FilterEntry.FilterAction.SET;

        rsslState.clear();
        stateText.data("");
        rsslState.text(stateText);
        return this;
    }

    /**
     * Replaces the contents of this object with a deep copy of another service
     * state filter.
     * <p>
     * If {@code sourceServiceState} is this object, the call succeeds without
     * modifying the current state. Otherwise, this object is reset before values
     * are copied from the source.
     *
     * @param sourceServiceState source service state filter to copy from
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceState} is {@code null}
     */
    @Override
    public DirectoryServiceState copy(DirectoryServiceState sourceServiceState)
    {
        if (sourceServiceState == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceServiceState can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceServiceState == this)
        {
            return this;
        }

        clear();

        action(sourceServiceState.action());
        serviceState(sourceServiceState.serviceState());

        if (sourceServiceState.checkHasAcceptingRequests())
        {
            acceptingRequests(sourceServiceState.acceptingRequests());
        }

        if (sourceServiceState.checkHasStatus())
        {
            status(sourceServiceState.status());
        }
        return this;
    }

    /**
     * Returns the filter entry action associated with this service state.
     *
     * @return one of the {@link FilterEntry.FilterAction} values
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Sets the filter entry action associated with this service state.
     *
     * @param action one of {@link FilterEntry.FilterAction#SET},
     *               {@link FilterEntry.FilterAction#UPDATE}, or
     *               {@link FilterEntry.FilterAction#CLEAR}
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code action} is not one of the
     *                                  supported filter-entry actions
     */
    @Override
    public DirectoryServiceState action(int action)
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
     * Returns the current service state.
     * <p>
     * This field is always available. After {@link #clear()}, the default value
     * is {@link EmaRdm#SERVICE_UP}.
     *
     * @return {@link EmaRdm#SERVICE_UP} if the service is up, or
     *         {@link EmaRdm#SERVICE_DOWN} if the service is down
     */
    @Override
    public int serviceState()
    {
        return flags.get(SERVICE_STATE_UP_FLAG) ? EmaRdm.SERVICE_UP : EmaRdm.SERVICE_DOWN;
    }

    /**
     * Sets the current service state.
     *
     * @param serviceState service state value; valid values are
     *                     {@link EmaRdm#SERVICE_UP} and
     *                     {@link EmaRdm#SERVICE_DOWN}
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code serviceState} is not supported
     */
    @Override
    public DirectoryServiceState serviceState(int serviceState)
    {
        if (serviceState != EmaRdm.SERVICE_UP && serviceState != EmaRdm.SERVICE_DOWN)
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid serviceState value of " + serviceState,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (serviceState == EmaRdm.SERVICE_UP)
        {
            flags.set(SERVICE_STATE_UP_FLAG);
        }
        else
        {
            flags.clear(SERVICE_STATE_UP_FLAG);
        }
        return this;
    }

    /**
     * Returns whether the service is accepting new item requests.
     * <p>
     * This is an optional field. Call {@link #checkHasAcceptingRequests()}
     * before calling this method.
     *
     * @return {@link EmaRdm#SERVICE_YES} if the service accepts new requests, or
     *         {@link EmaRdm#SERVICE_NO} otherwise
     * @throws OmmInvalidUsageException if
     *                                  {@link #checkHasAcceptingRequests()}
     *                                  returns {@code false}
     */
    @Override
    public int acceptingRequests()
    {
        if (!checkHasAcceptingRequests())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_ACCEPTING_REQS + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return flags.get(ACCEPTING_REQUESTS_YES_FLAG) ? EmaRdm.SERVICE_YES : EmaRdm.SERVICE_NO;
    }

    /**
     * Sets whether the service is accepting new item requests.
     * <p>
     * Calling this method marks the optional {@code acceptingRequests} element
     * as present.
     *
     * @param acceptingRequests accepting requests flag; valid values are
     *                          {@link EmaRdm#SERVICE_YES} and
     *                          {@link EmaRdm#SERVICE_NO}
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code acceptingRequests} is not
     *                                  supported
     */
    @Override
    public DirectoryServiceState acceptingRequests(int acceptingRequests)
    {
        if (acceptingRequests != EmaRdm.SERVICE_YES && acceptingRequests != EmaRdm.SERVICE_NO)
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid acceptingRequests value of " + acceptingRequests,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (acceptingRequests == EmaRdm.SERVICE_YES)
        {
            flags.set(ACCEPTING_REQUESTS_YES_FLAG);
        }
        else
        {
            flags.clear(ACCEPTING_REQUESTS_YES_FLAG);
        }
        applyHasAcceptingRequests();
        return this;
    }

    /**
     * Indicates whether the optional {@code acceptingRequests} element is
     * present.
     * <p>
     * Although the internal default value is initialized to
     * {@link EmaRdm#SERVICE_YES}, the field is considered absent until it is
     * explicitly set or decoded from input.
     *
     * @return {@code true} if {@code acceptingRequests} is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasAcceptingRequests()
    {
        return flags.get(HAS_ACCEPTING_REQUESTS_FLAG);
    }

    /**
     * Applies acceptingRequests presence flag.
     *
     */
    private void applyHasAcceptingRequests()
    {
        flags.set(HAS_ACCEPTING_REQUESTS_FLAG);
    }

    /**
     * Returns the status that applies to all items provided by this service.
     * <p>
     * This is an optional field. Call {@link #checkHasStatus()} before calling
     * this method.
     *
     * @return service status
     * @throws OmmInvalidUsageException if {@link #checkHasStatus()} returns
     *                                  {@code false}
     */
    @Override
    public OmmState status()
    {
        if (!checkHasStatus())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_STATUS + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        status.decode(rsslState);

        return status;
    }


    /**
     * Sets the status that applies to all items provided by this service.
     * <p>
     * Calling this method marks the optional {@code status} element as present.
     *
     * @param status service status to copy from
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code status} is {@code null} or
     *                                  contains an unsupported state value
     */
    @Override
    public DirectoryServiceState status(OmmState status)
    {
        if (status == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("status can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        return status(status.streamState(), status.dataState(), status.statusCode(), status.statusText());
    }

    /**
     * Sets the status that applies to all items provided by this service.
     * <p>
     * Calling this method marks the optional {@code status} element as present.
     *
     * @param streamState {@link OmmState.StreamState} value
     * @param dataState {@link OmmState.DataState} value
     * @param statusCode {@link OmmState.StatusCode} value
     * @param statusText status text
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null} or
     *                                  any state component is not supported
     */
    @Override
    public DirectoryServiceState status(int streamState, int dataState, int statusCode, String statusText)
    {
        if (statusText == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("statusText can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        validateStatus(streamState, dataState, statusCode);

        rsslState.streamState(streamState);
        rsslState.dataState(dataState);
        rsslState.code(statusCode);
        stateText.data(statusText);
        rsslState.text(stateText);

        applyHasStatus();
        return this;
    }

    /**
     * Indicates whether the optional {@code status} element is present.
     *
     * @return {@code true} if {@code status} is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasStatus()
    {
        return flags.get(HAS_STATUS_FLAG);
    }

    /**
     * Applies status presence flag.
     *
     */
    private void applyHasStatus()
    {
        flags.set(HAS_STATUS_FLAG);
    }

    /**
     * Returns the RDM filter identifier for the service state filter.
     *
     * @return {@link EmaRdm#SERVICE_STATE_ID}
     */
    @Override
    public int filterId()
    {
        return EmaRdm.SERVICE_STATE_ID;
    }

    /**
     * Encodes this service state filter as an {@link ElementList}.
     * <p>
     * The encoded payload always contains {@code SVC_STATE}. The optional
     * {@code ACCEPTING_REQS} and {@code STATUS} elements are included only when
     * they are present in this object.
     *
     * @return encoded service state filter
     */
    @Override
    public ElementList encode()
    {
        ElementList elementList = EmaFactory.createElementList();

        elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, serviceState()));

        if (checkHasAcceptingRequests())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_REQS,
                    acceptingRequests()));
        }

        if (checkHasStatus())
        {
            elementList.add(EmaFactory.createElementEntry().state(EmaRdm.ENAME_STATUS, rsslState.streamState(),
                    rsslState.dataState(), rsslState.code(), rsslState.text().toString()));
        }

        return elementList;
    }

    /**
     * Decodes an EMA service state filter from an {@link ElementList}.
     * <p>
     * Unknown elements are ignored. {@code SVC_STATE} is required. A blank
     * {@code STATUS} element is treated as absent. For non-null input, decoding
     * starts by resetting this object; if decoding later fails, this object
     * remains in inconsistent state and should be cleared for further usage.
     *
     * @param elementList encoded service state filter to decode
     * @return this directory service state filter instance
     * @throws OmmInvalidUsageException if {@code elementList} is {@code null},
     *                                  required data is missing, or an element
     *                                  contains an unsupported value or type
     */
    @Override
    public DirectoryServiceState decode(ElementList elementList)
    {
        if (elementList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("elementList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        Iterator<ElementEntry> iterator = elementList.iteratorByRef();
        boolean foundServiceState = false;
        while (iterator.hasNext())
        {
            ElementEntry elementEntry = iterator.next();
            String elementName = elementEntry.name();

            switch (elementName)
            {
                case EmaRdm.ENAME_SVC_STATE:
                    long serviceState = elementEntry.uintValue();
                    if (serviceState != EmaRdm.SERVICE_DOWN && serviceState != EmaRdm.SERVICE_UP)
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }

                    serviceState((int) serviceState);
                    foundServiceState = true;
                    break;
                case EmaRdm.ENAME_ACCEPTING_REQS:
                    long acceptingRequests = elementEntry.uintValue();
                    if (acceptingRequests != EmaRdm.SERVICE_NO && acceptingRequests != EmaRdm.SERVICE_YES)
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }

                    acceptingRequests((int) acceptingRequests);
                    break;
                case EmaRdm.ENAME_STATUS:
                    if (elementEntry.code() == Data.DataCode.BLANK)
                    {
                        break;
                    }

                    OmmState status = elementEntry.state();
                    if (!isValidStatusStreamState(status.streamState()))
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }

                    status(status);
                    break;
                default:
                    break;
            }
        }

        if (!foundServiceState)
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_SVC_STATE + " element is absent",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
        return this;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("StateFilter:");
        stringBuilder.append(EOL);

        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("serviceState: ");
        stringBuilder.append(serviceState() == EmaRdm.SERVICE_UP ? "up" : "down");
        stringBuilder.append(EOL);

        if (checkHasAcceptingRequests())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("acceptingRequests: ");
            stringBuilder.append(acceptingRequests() == EmaRdm.SERVICE_YES ? "yes" : "no");
            stringBuilder.append(EOL);
        }

        if (checkHasStatus())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("status: ");
            stringBuilder.append(status());
            stringBuilder.append(EOL);
        }

        return stringBuilder.toString();
    }
}
