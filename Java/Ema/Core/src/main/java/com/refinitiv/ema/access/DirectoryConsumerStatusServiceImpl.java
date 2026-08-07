/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryConsumerStatusService;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.Iterator;

import static com.refinitiv.ema.access.DirectoryValidators.*;
import static com.refinitiv.ema.rdm.EmaRdm.*;

/**
 * Default implementation of {@link DirectoryConsumerStatusService}.
 * <p>
 * This type represents the element-list payload for a single consumer-status
 * service entry within a directory consumer-status message. The payload may
 * store {@link #sourceMirroringMode()}, {@link #warmStandbyMode()}, or both.
 * The enclosing map-entry metadata represented by {@link #serviceId()} and
 * {@link #action()} is maintained on this object for convenience but is not part
 * of the {@link ElementList} returned by {@link #encode()} or consumed by
 * {@link #decode(ElementList)}.
 *
 * @see DirectoryConsumerStatusService
 * @see WarmStandbyDirectoryServiceTypes
 * @see SourceMirroringMode
 */
final class DirectoryConsumerStatusServiceImpl implements DirectoryConsumerStatusService
{
    private int serviceId;
    private int action;
    private int sourceMirroringMode;
    private int wsbMode;
    private boolean hasWarmStandbyMode;
    private boolean hasSourceMirroringMode;
    private final StringBuilder stringBuilder = new StringBuilder();

    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    DirectoryConsumerStatusServiceImpl()
    {
        clear();
    }

    /**
     * Clears this service entry and restores its default state.
     * <p>
     * This resets {@link #serviceId()} to {@code 0}, {@link #action()} to
     * {@link MapEntry.MapAction#ADD}, and marks both payload fields as absent.
     * Internal defaults are restored to
     * {@link SourceMirroringMode#ACTIVE_NO_STANDBY} and
     * {@link WarmStandbyDirectoryServiceTypes#ACTIVE}, but neither field is
     * considered present until its setter is called or a payload is decoded.
     *
     * @return this directory consumer-status service instance
     */
    @Override
    public DirectoryConsumerStatusService clear()
    {
        action = MapEntry.MapAction.ADD;
        serviceId = 0;
        hasWarmStandbyMode = false;
        hasSourceMirroringMode = false;
        sourceMirroringMode = SourceMirroringMode.ACTIVE_NO_STANDBY;
        wsbMode = WarmStandbyDirectoryServiceTypes.ACTIVE;
        return this;
    }

    /**
     * Performs a deep copy of another service entry into this object.
     * <p>
     * The current contents are cleared before copying. If
     * {@code sourceConsumerStatusService} is this object, the call is a no-op.
     *
     * @param sourceConsumerStatusService source service entry to copy from; must not be {@code null}
     * @return this directory consumer-status service instance
     *
     * @throws OmmInvalidUsageException if {@code sourceConsumerStatusService} is {@code null}
     */
    @Override
    public DirectoryConsumerStatusService copy(DirectoryConsumerStatusService sourceConsumerStatusService)
    {
        if (sourceConsumerStatusService == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceConsumerStatusService can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this == sourceConsumerStatusService)
        {
            return this;
        }

        clear();

        serviceId(sourceConsumerStatusService.serviceId());
        action(sourceConsumerStatusService.action());
        if(sourceConsumerStatusService.checkHasSourceMirroringMode())
        {
            sourceMirroringMode(sourceConsumerStatusService.sourceMirroringMode());
        }
        if(sourceConsumerStatusService.checkHasWarmStandbyMode())
        {
            warmStandbyMode(sourceConsumerStatusService.warmStandbyMode());
        }
        return this;
    }

    /**
     * Decodes this service entry from an EMA {@link ElementList} payload.
     * <p>
     * This method clears the current object before decoding. The supplied
     * {@link ElementList} represents only the service payload; the enclosing
     * map-entry metadata represented by {@link #serviceId()} and
     * {@link #action()} is not read from this method and must be handled by the
     * caller.
     * <p>
     * Recognized payload fields are
     * {@link EmaRdm#ENAME_CONS_SOURCE_MIROR_MODE} and
     * {@link EmaRdm#ENAME_WARMSTANDBY_MODE}. Either field may appear on its own,
     * or both may appear together, but both may not be absent. Unrecognized
     * elements are ignored.
     *
     * @param elementList element-list payload representing a consumer-status service entry
     * @return this directory consumer-status service instance
     *
     * @throws OmmInvalidUsageException if {@code elementList} is {@code null}, if
     *                                  an element contains an invalid value, or if
     *                                  both recognized payload fields are absent
     */
    @Override
    public DirectoryConsumerStatusService decode(ElementList elementList)
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

        boolean foundSourceMirroringMode = false;
        boolean foundWarmStandbyMode = false;
        while (iterator.hasNext())
        {
            elementEntry = iterator.next();
            elementName = elementEntry.name();
            switch (elementName)
            {
                case EmaRdm.ENAME_CONS_SOURCE_MIROR_MODE:
                    long sourceMirroringMode = elementEntry.uintValue();
                    if (isValidSourceMirroringMode(sourceMirroringMode))
                    {
                        sourceMirroringMode((int) sourceMirroringMode);
                        foundSourceMirroringMode = true;
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    break;
                case EmaRdm.ENAME_WARMSTANDBY_MODE:
                    long wsbMode = elementEntry.uintValue();
                    if (isValidWarmStandbyMode(wsbMode))
                    {
                        warmStandbyMode((int) wsbMode);
                        foundWarmStandbyMode = true;
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    break;
                default:
                    break;
            }
        }

        if(!foundSourceMirroringMode && !foundWarmStandbyMode)
        {
            throw new OmmInvalidUsageExceptionImpl().message("Both " + EmaRdm.ENAME_CONS_SOURCE_MIROR_MODE +
                            " and " + EmaRdm.ENAME_WARMSTANDBY_MODE + " elements are absent",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
        return this;
    }

    /**
     * Encodes this service entry's payload as an EMA {@link ElementList}.
     * <p>
     * The returned payload includes whichever of
     * {@link EmaRdm#ENAME_CONS_SOURCE_MIROR_MODE} and
     * {@link EmaRdm#ENAME_WARMSTANDBY_MODE} are currently present. Source-only,
     * warm-only, and combined payloads are valid, but encoding is rejected when
     * both payload fields are absent. The map-entry metadata represented by
     * {@link #serviceId()} and {@link #action()} is not included in the returned
     * payload.
     *
     * @return an {@link ElementList} representing this service entry's payload
     * @throws OmmInvalidUsageException if both payload fields are absent or if
     *                                  payload state is otherwise invalid
     */
    @Override
    public ElementList encode()
    {
        if(!checkHasSourceMirroringMode() && !checkHasWarmStandbyMode())
        {
            throw new OmmInvalidUsageExceptionImpl().message("Both " + EmaRdm.ENAME_CONS_SOURCE_MIROR_MODE +
                            " and " + EmaRdm.ENAME_WARMSTANDBY_MODE + " elements are absent",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        ElementList elementList = EmaFactory.createElementList();

        if (checkHasSourceMirroringMode())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_CONS_SOURCE_MIROR_MODE,
                    sourceMirroringMode()));
        }

        if (checkHasWarmStandbyMode())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_WARMSTANDBY_MODE,
                    warmStandbyMode()));
        }

        return elementList;
    }

    /**
     * Returns the identifier of the service this status concerns.
     * <p>
     * This value identifies the enclosing directory map entry and is not part of
     * the encoded or decoded {@link ElementList} payload.
     *
     * @return the service identifier
     */
    @Override
    public int serviceId()
    {
        return serviceId;
    }

    /**
     * Sets the identifier of the service this status concerns.
     * <p>
     * This value identifies the enclosing directory map entry and is not part of
     * the encoded or decoded {@link ElementList} payload.
     *
     * @param serviceId the service identifier
     * @return this directory consumer-status service instance
     * @throws OmmInvalidUsageException if {@code serviceId} is outside the valid range
     */
    @Override
    public DirectoryConsumerStatusService serviceId(int serviceId)
    {
        if (!isValidServiceId(serviceId))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid serviceId value of " + serviceId,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.serviceId = serviceId;
        return this;
    }

    /**
     * Returns the action associated with the enclosing directory map entry.
     *
     * @return the map-entry action
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Sets the action associated with the enclosing directory map entry.
     *
     * @param action action populated from {@link MapEntry.MapAction}
     * @return this directory consumer-status service instance
     * @throws OmmInvalidUsageException if {@code action} is not one of
     *                                  {@link MapEntry.MapAction#ADD},
     *                                  {@link MapEntry.MapAction#UPDATE}, or
     *                                  {@link MapEntry.MapAction#DELETE}
     */
    @Override
    public DirectoryConsumerStatusService action(int action)
    {
        if (!isValidMapEntryAction(action))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + action,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.action = action;
        return this;
    }

    /**
     * Returns the source-mirroring mode for this service entry.
     * <p>
     * Valid values are defined by {@link SourceMirroringMode}.
     *
     * @return the source-mirroring mode
     * @throws OmmInvalidUsageException if {@link #checkHasSourceMirroringMode()} returns {@code false}
     */
    @Override
    public int sourceMirroringMode()
    {
        if (!checkHasSourceMirroringMode())
        {
            throw new OmmInvalidUsageExceptionImpl().message(ENAME_CONS_SOURCE_MIROR_MODE +
                    " element is not set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return sourceMirroringMode;
    }

    /**
     * Sets the source-mirroring mode for this service entry.
     * <p>
     * Valid values are defined by {@link SourceMirroringMode}.
     *
     * @param sourceMirroringMode the source-mirroring mode
     * @return this directory consumer-status service instance
     * @throws OmmInvalidUsageException if {@code sourceMirroringMode} is not one
     *                                  of the values defined in
     *                                  {@link SourceMirroringMode}
     */
    @Override
    public DirectoryConsumerStatusService sourceMirroringMode(int sourceMirroringMode)
    {
        if (isValidSourceMirroringMode(sourceMirroringMode))
        {
            this.sourceMirroringMode = sourceMirroringMode;
            applyHasSourceMirroringMode();
            return this;
        }
        else
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid sourceMirroringMode value: " + sourceMirroringMode,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    /**
     * Returns the warm-standby mode for this service entry.
     * <p>
     * This field is optional. Call {@link #checkHasWarmStandbyMode()} before
     * invoking this method.
     *
     * @return the warm-standby mode
     * @throws OmmInvalidUsageException if {@link #checkHasWarmStandbyMode()}
     *                                  returns {@code false}
     */
    @Override
    public int warmStandbyMode()
    {
        if (!checkHasWarmStandbyMode())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_WARMSTANDBY_MODE +
                    " element is not set", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return wsbMode;
    }

    /**
     * Sets the warm-standby mode for this service entry.
     * <p>
     * Calling this method marks the optional warm-standby field as present.
     * Valid values are defined by {@link WarmStandbyDirectoryServiceTypes}.
     *
     * @param warmStandbyMode the warm-standby mode
     * @return this directory consumer-status service instance
     * @throws OmmInvalidUsageException if {@code warmStandbyMode} is not one of
     *                                  the values defined in
     *                                  {@link WarmStandbyDirectoryServiceTypes}
     */
    @Override
    public DirectoryConsumerStatusService warmStandbyMode(int warmStandbyMode)
    {
        if (isValidWarmStandbyMode(warmStandbyMode))
        {
            this.wsbMode = warmStandbyMode;
            applyHasWarmStandbyMode();
            return this;
        }
        else
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid warmStandbyMode value: " + warmStandbyMode,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
    }

    /**
     * Checks whether the optional source-mirroring field is present.
     *
     * @return {@code true} if the source-mirroring mode is available; otherwise {@code false}
     */
    @Override
    public boolean checkHasSourceMirroringMode()
    {
        return hasSourceMirroringMode;
    }

    private void applyHasSourceMirroringMode()
    {
        hasSourceMirroringMode = true;
    }

    /**
     * Checks whether the optional warm-standby field is present.
     *
     * @return {@code true} if the warm-standby mode is available; otherwise {@code false}
     */
    @Override
    public boolean checkHasWarmStandbyMode()
    {
        return hasWarmStandbyMode;
    }

    private void applyHasWarmStandbyMode()
    {
        hasWarmStandbyMode = true;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);
        stringBuilder.append(TAB)
                .append(TAB)
                .append("ConsumerStatusService:")
                .append(EOL)
                .append(TAB)
                .append(TAB)
                .append(TAB)
                .append("action: ")
                .append(action())
                .append(EOL)
                .append(TAB)
                .append(TAB)
                .append(TAB)
                .append("serviceId: ")
                .append(serviceId())
                .append(EOL);

        if(checkHasSourceMirroringMode())
        {
            stringBuilder.append(TAB)
                    .append(TAB)
                    .append(TAB)
                    .append("sourceMirroringMode: ")
                    .append(SourceMirroringMode.asString(sourceMirroringMode()))
                    .append(EOL);
        }

        if(checkHasWarmStandbyMode())
        {
            stringBuilder.append(TAB)
                    .append(TAB)
                    .append(TAB)
                    .append("warmStandbyMode: ")
                    .append(WarmStandbyDirectoryServiceTypes.asString(warmStandbyMode()))
                    .append(EOL);
        }

        return stringBuilder.toString();
    }
}
