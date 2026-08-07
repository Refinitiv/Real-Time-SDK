/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceLoad;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.BitSet;
import java.util.Iterator;

import static com.refinitiv.ema.access.DirectoryValidators.isValidFilterEntryAction;

/**
 * Default implementation of {@link DirectoryServiceLoad} for the RDM Source
 * Directory service load filter.
 * <p>
 * This filter conveys optional service-capacity and workload information,
 * including the maximum number of open items, the maximum number of
 * outstanding requests, and the current load factor.
 *
 * @see DirectoryServiceLoad
 */
final class DirectoryServiceLoadImpl implements DirectoryServiceLoad
{
    private long openLimit;
    private long openWindow;
    private long loadFactor;
    private int action;
    private final BitSet flags = new BitSet();
    private final StringBuilder stringBuilder = new StringBuilder();

    private static final int HAS_OPEN_LIMIT_FLAG = 0;
    private static final int HAS_OPEN_WINDOW_FLAG = 1;
    private static final int HAS_LOAD_FACTOR_FLAG = 2;
    private final static long MAX_UINT = 4294967295L;
    private final static long MAX_LOAD_FACTOR = 65535;
    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    /**
     * Creates a service load filter initialized to the same defaults as
     * {@link #clear()}.
     */
    public DirectoryServiceLoadImpl()
    {
        clear();
    }

    /**
     * Resets this filter to its default state.
     * <p>
     * After this call, all optional load elements are marked as absent,
     * {@link #action()} is {@link FilterEntry.FilterAction#SET}, and the backing
     * numeric fields are restored to their default sentinel values.
     *
     * @return this directory service load filter instance
     */
    @Override
    public DirectoryServiceLoad clear()
    {
        flags.clear();
        action = FilterEntry.FilterAction.SET;
        openLimit = MAX_UINT;
        openWindow = MAX_UINT;
        loadFactor = MAX_LOAD_FACTOR;
        return this;
    }

    /**
     * Returns the filter entry action associated with this service load.
     *
     * @return one of the {@link FilterEntry.FilterAction} values
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Sets the filter entry action associated with this service load.
     *
     * @param action one of {@link FilterEntry.FilterAction#SET},
     *               {@link FilterEntry.FilterAction#UPDATE}, or
     *               {@link FilterEntry.FilterAction#CLEAR}
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code action} is not one of the
     *                                  supported filter-entry actions
     */
    @Override
    public DirectoryServiceLoad action(int action)
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
     * Returns the maximum number of items a consumer is allowed to open from
     * this service.
     * <p>
     * This is an optional field. Call {@link #checkHasOpenLimit()} before
     * calling this method.
     *
     * @return open limit
     * @throws OmmInvalidUsageException if {@link #checkHasOpenLimit()} returns
     *                                  {@code false}
     */
    @Override
    public long openLimit()
    {
        if (!checkHasOpenLimit())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_OPEN_LIMIT + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return openLimit;
    }

    /**
     * Sets the maximum number of items a consumer is allowed to open from this
     * service.
     * <p>
     * Calling this method marks the optional {@code openLimit} field as present.
     *
     * @param openLimit the open limit
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code openLimit} is outside the range
     *                                  {@code 0} to {@code 4294967295L}
     */
    @Override
    public DirectoryServiceLoad openLimit(long openLimit)
    {
        if (openLimit >= 0 && openLimit <= MAX_UINT)
        {
            this.openLimit = openLimit;
            applyHasOpenLimit();
            return this;
        }
        else
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                    EmaRdm.ENAME_OPEN_LIMIT, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    /**
     * Indicates whether the optional {@code openLimit} field is present.
     *
     * @return {@code true} if {@code openLimit} is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasOpenLimit()
    {
        return flags.get(HAS_OPEN_LIMIT_FLAG);
    }

    /**
     * Marks the optional {@code openLimit} field as present.
     */
    private void applyHasOpenLimit()
    {
        flags.set(HAS_OPEN_LIMIT_FLAG);
    }

    /**
     * Returns the maximum number of item requests a consumer may have
     * outstanding, meaning they are still waiting for a refresh, from this
     * service.
     * <p>
     * This is an optional field. Call {@link #checkHasOpenWindow()} before
     * calling this method.
     *
     * @return open window
     * @throws OmmInvalidUsageException if {@link #checkHasOpenWindow()} returns
     *                                  {@code false}
     */
    @Override
    public long openWindow()
    {
        if (!checkHasOpenWindow())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_OPEN_WINDOW + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return openWindow;
    }

    /**
     * Sets the maximum number of item requests a consumer may have
     * outstanding, meaning they are still waiting for a refresh, from this
     * service.
     * <p>
     * Calling this method marks the optional {@code openWindow} field as
     * present.
     *
     * @param openWindow the open window
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code openWindow} is outside the
     *                                  range {@code 0} to {@code 4294967295L}
     */
    @Override
    public DirectoryServiceLoad openWindow(long openWindow)
    {
        if (openWindow >= 0 && openWindow <= MAX_UINT)
        {
            this.openWindow = openWindow;
            applyHasOpenWindow();
            return this;
        }
        else
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                    EmaRdm.ENAME_OPEN_WINDOW, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    /**
     * Indicates whether the optional {@code openWindow} field is present.
     *
     * @return {@code true} if {@code openWindow} is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasOpenWindow()
    {
        return flags.get(HAS_OPEN_WINDOW_FLAG);
    }

    /**
     * Marks the optional {@code openWindow} field as present.
     */
    private void applyHasOpenWindow()
    {
        flags.set(HAS_OPEN_WINDOW_FLAG);
    }

    /**
     * Returns the load factor, which indicates the current workload of the
     * source providing the data.
     * <p>
     * This is an optional field. Call {@link #checkHasLoadFactor()} before
     * calling this method.
     *
     * @return load factor
     * @throws OmmInvalidUsageException if {@link #checkHasLoadFactor()} returns
     *                                  {@code false}
     */
    @Override
    public long loadFactor()
    {
        if (!checkHasLoadFactor())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_LOAD_FACT + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return loadFactor;
    }

    /**
     * Sets the load factor, which indicates the current workload of the source
     * providing the data.
     * <p>
     * Calling this method marks the optional {@code loadFactor} field as
     * present.
     *
     * @param loadFactor the load factor
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code loadFactor} is outside the
     *                                  range {@code 0} to {@code 65535}
     */
    @Override
    public DirectoryServiceLoad loadFactor(long loadFactor)
    {
        if (loadFactor >= 0 && loadFactor <= MAX_LOAD_FACTOR)
        {
            this.loadFactor = loadFactor;
            applyHasLoadFactor();
            return this;
        }
        else
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                    EmaRdm.ENAME_LOAD_FACT, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    /**
     * Indicates whether the optional {@code loadFactor} field is present.
     *
     * @return {@code true} if {@code loadFactor} is present; otherwise,
     *         {@code false}
     */
    @Override
    public boolean checkHasLoadFactor()
    {
        return flags.get(HAS_LOAD_FACTOR_FLAG);
    }

    /**
     * Marks the optional {@code loadFactor} field as present.
     */
    private void applyHasLoadFactor()
    {
        flags.set(HAS_LOAD_FACTOR_FLAG);
    }

    /**
     * Replaces the contents of this object with a deep copy of another service
     * load filter.
     * <p>
     * If {@code sourceServiceLoad} is this object, the call succeeds without
     * modifying the current state. Otherwise, this object is reset before
     * values are copied from the source.
     *
     * @param sourceServiceLoad source service load filter to copy from
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceLoad} is {@code null}
     */
    @Override
    public DirectoryServiceLoad copy(DirectoryServiceLoad sourceServiceLoad)
    {
        if (sourceServiceLoad == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceServiceLoad can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceServiceLoad == this)
        {
            return this;
        }

        clear();

        if (sourceServiceLoad.checkHasLoadFactor())
        {
            loadFactor(sourceServiceLoad.loadFactor());
        }

        if (sourceServiceLoad.checkHasOpenLimit())
        {
            openLimit(sourceServiceLoad.openLimit());
        }

        if (sourceServiceLoad.checkHasOpenWindow())
        {
            openWindow(sourceServiceLoad.openWindow());
        }

        action(sourceServiceLoad.action());
        return this;
    }

    /**
     * Returns the RDM filter identifier for the service load filter.
     *
     * @return {@link EmaRdm#SERVICE_LOAD_ID}
     */
    @Override
    public int filterId()
    {
        return EmaRdm.SERVICE_LOAD_ID;
    }

    /**
     * Encodes this service load filter as an {@link ElementList}.
     * <p>
     * The encoded payload includes only the optional elements that are present
     * in this object.
     *
     * @return encoded service load filter
     */
    @Override
    public ElementList encode()
    {
        ElementList elementList = EmaFactory.createElementList();

        if (checkHasOpenWindow())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_OPEN_WINDOW, openWindow));
        }

        if (checkHasOpenLimit())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_OPEN_LIMIT, openLimit));
        }

        if (checkHasLoadFactor())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LOAD_FACT, loadFactor));
        }

        return elementList;
    }

    /**
     * Decodes an EMA service load filter from an {@link ElementList}.
     * <p>
     * Unknown elements are ignored. For non-null input, decoding starts by
     * resetting this object; if decoding later fails, this object remains in
     * inconsistent state and should be cleared for further usage.
     *
     * @param elementList encoded service load filter to decode
     * @return this directory service load filter instance
     * @throws OmmInvalidUsageException if {@code elementList} is {@code null} or
     *                                  an element contains an unsupported type
     *                                  or value
     */
    @Override
    public DirectoryServiceLoad decode(ElementList elementList)
    {
        if (elementList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("elementList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        Iterator<ElementEntry> iterator = elementList.iteratorByRef();
        while (iterator.hasNext())
        {
            ElementEntry elementEntry = iterator.next();
            String elementName = elementEntry.name();

            switch (elementName)
            {
                case EmaRdm.ENAME_OPEN_LIMIT:
                    long openLimit = elementEntry.uintValue();
                    openLimit(openLimit);
                    break;
                case EmaRdm.ENAME_OPEN_WINDOW:
                    long openWindow = elementEntry.uintValue();
                    openWindow(openWindow);
                    break;
                case EmaRdm.ENAME_LOAD_FACT:
                    long loadFactor = elementEntry.uintValue();
                    loadFactor(loadFactor);
                    break;
                default:
                    break;
            }
        }
        return this;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("LoadFilter:");
        stringBuilder.append(EOL);

        if (checkHasOpenLimit())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("openLimit: ");
            stringBuilder.append(openLimit());
            stringBuilder.append(EOL);
        }

        if (checkHasOpenWindow())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("openWindow: ");
            stringBuilder.append(openWindow());
            stringBuilder.append(EOL);
        }

        if(checkHasLoadFactor())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("loadFactor: ");
            stringBuilder.append(loadFactor());
            stringBuilder.append(EOL);
        }

        return stringBuilder.toString();
    }
}

