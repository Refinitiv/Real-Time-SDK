/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceGroup;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.*;

import java.nio.ByteBuffer;
import java.util.Iterator;

import static com.refinitiv.ema.access.DirectoryValidators.*;

/**
 * Default implementation of {@link DirectoryServiceGroup} for one RDM Source
 * Directory service group-state entry.
 * <p>
 * A group-state entry applies to all items whose {@code ItemGroup} matches the
 * required {@code Group} element. The optional {@code MergedToGroup} and
 * {@code Status} elements are exposed through
 * {@link #checkHasMergedToGroup()} and {@link #checkHasStatus()}.
 * <p>
 * This implementation stores the entry action separately from the encoded
 * {@link ElementList} payload. After {@link #clear()}, optional elements are
 * absent, {@link #action()} is reset to
 * {@link FilterEntry.FilterAction#SET}, and {@link #group()} returns an empty
 * buffer until a group value is provided.
 *
 * @see DirectoryServiceGroup
 */
final class DirectoryServiceGroupImpl implements DirectoryServiceGroup
{
    private final Buffer group = CodecFactory.createBuffer();
    private final Buffer mergedToGroup = CodecFactory.createBuffer();
    private final State rsslState = CodecFactory.createState();
    private final Buffer stateText = CodecFactory.createBuffer();
    private final OmmStateImpl status = new OmmStateImpl();
    private final StringBuilder stringBuilder = new StringBuilder();
    private int action;
    private boolean hasMergedToGroup;
    private boolean hasStatus;

    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    /**
     * Creates a group-state entry initialized to the same defaults as
     * {@link #clear()}.
     */
    public DirectoryServiceGroupImpl()
    {
        clear();
    }

    /**
     * Resets this group-state entry to its default state.
     * <p>
     * After this call, optional fields are marked absent, the stored group and
     * merged-to-group buffers are cleared, the cached status state is reset, and
     * {@link #action()} becomes {@link FilterEntry.FilterAction#SET}.
     *
     * @return this directory service group filter instance
     */
    @Override
    public DirectoryServiceGroup clear()
    {
        hasMergedToGroup = false;
        hasStatus = false;

        group.clear();
        mergedToGroup.clear();
        action = FilterEntry.FilterAction.SET;

        rsslState.clear();
        stateText.data("");
        rsslState.text(stateText);
        return this;
    }

    /**
     * Applies status presence flag.
     *
     */
    private void applyHasStatus()
    {
       hasStatus = true;
    }

    /**
     * Checks the presence of the status field.
     *
     * @return true - if info field exists, false - if not.
     */
    @Override
    public boolean checkHasStatus()
    {
        return hasStatus;
    }

    /**
     * Apply has merged to group flag.
     *
     */
    private void applyHasMergedToGroup()
    {
        hasMergedToGroup = true;
    }

    /**
     * Checks the presence of the mergedToGroup field.
     *
     * @return true - if info field exists, false - if not.
     */
    @Override
    public boolean checkHasMergedToGroup()
    {
        return hasMergedToGroup;
    }

    /**
     * Returns the filter-entry action associated with this group-state entry.
     *
     * @return one of the {@link FilterEntry.FilterAction} values
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Sets the filter-entry action associated with this group-state entry.
     *
     * @param action one of {@link FilterEntry.FilterAction#SET},
     *               {@link FilterEntry.FilterAction#UPDATE}, or
     *               {@link FilterEntry.FilterAction#CLEAR}
     * @return this directory service group filter instance
     * @throws OmmInvalidUsageException if {@code action} is not one of the
     *                                  supported filter-entry actions
     */
    @Override
    public DirectoryServiceGroup action(int action)
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
     * Returns the item-group identifier associated with this entry.
     * <p>
     * This method always returns an {@link OmmBuffer}. If no group has been set,
     * the returned buffer is empty.
     *
     * @return group buffer
     */
    @Override
    public OmmBuffer group()
    {
        OmmBuffer buffer = new OmmBufferImpl();
        if (group.data() == null)
        {
            return buffer;
        }

        Utilities.copy(this.group, ((DataImpl) buffer).encodedData());

        return buffer;
    }

    /**
     * Sets the item-group identifier for this entry.
     * <p>
     * The supplied buffer content is copied into this object.
     *
     * @param group the group buffer
     * @return this directory service group filter instance
     * @throws OmmInvalidUsageException if {@code group} is {@code null}
     */
    @Override
    public DirectoryServiceGroup group(ByteBuffer group)
    {
        if (group == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("group can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        Utilities.copy(group, this.group);
        return this;
    }

    /**
     * Returns the optional merged-to-group value.
     *
     * @return mergedToGroup buffer
     * @throws OmmInvalidUsageException if
     *                                  {@link #checkHasMergedToGroup()} returns
     *                                  {@code false}
     */
    @Override
    public OmmBuffer mergedToGroup()
    {
        if (!checkHasMergedToGroup())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_MERG_TO_GRP + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        OmmBuffer buffer = new OmmBufferImpl();
        if (mergedToGroup.data() == null)
        {
            return buffer;
        }

        Utilities.copy(this.mergedToGroup, ((DataImpl) buffer).encodedData());

        return buffer;
    }

    /**
     * Sets the optional merged-to-group value.
     * <p>
     * The supplied buffer content is copied into this object and marks the
     * {@code mergedToGroup} field as present.
     *
     * @param mergedToGroup the merged to group buffer
     * @return this directory service group filter instance
     * @throws OmmInvalidUsageException if {@code mergedToGroup} is {@code null}
     */
    @Override
    public DirectoryServiceGroup mergedToGroup(ByteBuffer mergedToGroup)
    {
        if (mergedToGroup == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("mergedToGroup can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        Utilities.copy(mergedToGroup, this.mergedToGroup);
        applyHasMergedToGroup();
        return this;
    }

    /**
     * Returns the status applied to all items whose {@code ItemGroup} matches
     * {@link #group()}.
     *
     * @return status
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
     * Sets the status applied to all items whose {@code ItemGroup} matches
     * {@link #group()}.
     * <p>
     * The supplied state is copied into this object and marks the optional
     * {@code status} element as present.
     *
     * @param status the status
     * @return this directory service group filter instance
     * @throws OmmInvalidUsageException if {@code status} is {@code null}
     */
    @Override
    public DirectoryServiceGroup status(OmmState status)
    {
        if (status == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("status can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        return status(status.streamState(), status.dataState(), status.statusCode(), status.statusText());
    }

    /**
     * Sets the status applied to all items whose {@code ItemGroup} matches
     * {@link #group()}.
     * <p>
     * Calling this method marks the optional {@code status} element as present.
     *
     * @param streamState represents OmmState StreamState
     * @param dataState represents OmmState DataState
     * @param statusCode represents OmmState Status
     * @param statusText String representing OmmState Text
     * @return this directory service group filter instance
     * @throws OmmInvalidUsageException if {@code statusText} is {@code null} or
     *                                  any state component is not supported
     */
    @Override
    public DirectoryServiceGroup status(int streamState, int dataState, int statusCode, String statusText)
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
     * Returns the RDM filter identifier for the service group-state filter.
     *
     * @return {@link EmaRdm#SERVICE_GROUP_ID}
     */
    @Override
    public int filterId()
    {
        return EmaRdm.SERVICE_GROUP_ID;
    }

    /**
     * Encodes this group-state entry as an {@link ElementList}.
     * <p>
     * The encoded payload always contains the {@code GROUP} element. Optional
     * {@code MERG_TO_GRP} and {@code STATUS} elements are included only when
     * present in this object.
     *
     * @return encoded service group-state entry
     */
    @Override
    public ElementList encode()
    {
        ElementList elementList = EmaFactory.createElementList();

        elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, group().buffer()));

        if (checkHasMergedToGroup())
        {
            elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_MERG_TO_GRP, mergedToGroup().buffer()));
        }

        if (checkHasStatus())
        {
            elementList.add(EmaFactory.createElementEntry().state(EmaRdm.ENAME_STATUS, rsslState.streamState(),
                    rsslState.dataState(), rsslState.code(), rsslState.text().toString()));
        }

        return elementList;
    }

    /**
     * Decodes an EMA service group-state entry from an {@link ElementList}.
     * <p>
     * Unknown elements are ignored. {@code GROUP} is required and blank
     * {@code STATUS} is treated as absent. For non-null input, decoding begins
     * by clearing this object; if decoding later fails, this object remains in
     * inconsistent state and should be cleared for further usage.
     *
     * @param elementList encoded group-state entry to decode
     * @return this directory service group filter instance
     * @throws OmmInvalidUsageException if {@code elementList} is {@code null},
     *                                  required data is missing, or an element
     *                                  contains an unsupported value or type
     */
    @Override
    public DirectoryServiceGroup decode(ElementList elementList)
    {
        if (elementList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("elementList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        Iterator<ElementEntry> iterator = elementList.iteratorByRef();
        boolean foundGroup = false;
        while (iterator.hasNext())
        {
            ElementEntry elementEntry = iterator.next();
            String elementName = elementEntry.name();

            switch (elementName)
            {
                case EmaRdm.ENAME_GROUP:
                    if (elementEntry.code() != Data.DataCode.BLANK)
                    {
                        OmmBuffer group = elementEntry.buffer();
                        group(group.buffer());
                        foundGroup = true;
                    }
                    break;
                case EmaRdm.ENAME_MERG_TO_GRP:
                    if (elementEntry.code() != Data.DataCode.BLANK)
                    {
                        OmmBuffer mergedToGroup = elementEntry.buffer();
                        mergedToGroup(mergedToGroup.buffer());
                    }
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

        if (!foundGroup)
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_GROUP + " element is absent or blank",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
        return this;
    }

    /**
     * Replaces this object with a deep copy of another group-state entry.
     * <p>
     * If {@code sourceServiceGroup} is this object, the call succeeds without
     * modifying state. Otherwise, this object is cleared before copying values
     * from the source.
     *
     * @param sourceServiceGroup source group-state entry to copy from
     * @return this directory service group filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceGroup} is {@code null}
     */
    @Override
    public DirectoryServiceGroup copy(DirectoryServiceGroup sourceServiceGroup)
    {
        if (sourceServiceGroup == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceServiceGroup can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceServiceGroup == this)
        {
            return this;
        }

        clear();

        action(sourceServiceGroup.action());

        if (sourceServiceGroup.checkHasMergedToGroup())
        {
            mergedToGroup(sourceServiceGroup.mergedToGroup().buffer());
        }

        if (sourceServiceGroup.checkHasStatus())
        {
            status(sourceServiceGroup.status());
        }

        group(sourceServiceGroup.group().buffer());
        return this;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("GroupFilter:");
        stringBuilder.append(EOL);

        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("group: ");
        stringBuilder.append(group.toHexString());
        stringBuilder.append(EOL);

        if(checkHasMergedToGroup())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("mergedToGroup: ");
            stringBuilder.append(mergedToGroup.toHexString());
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
