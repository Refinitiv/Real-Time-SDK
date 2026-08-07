/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmInvalidUsageException;

import static com.refinitiv.ema.rdm.EmaRdm.*;

/**
 * Represents consumer-status information for a single service entry within a
 * directory consumer-status message.
 * <p>
 * A service entry is split across two layers:
 * <ul>
 *     <li>the enclosing directory {@link MapEntry}, identified by
 *     {@link #serviceId()} and {@link #action()}, and</li>
 *     <li>the entry payload, represented as an {@link ElementList} and decoded
 *     by {@link #decode(ElementList)} or produced by {@link #encode()}.</li>
 * </ul>
 * The payload may contain {@link #sourceMirroringMode()},
 * {@link #warmStandbyMode()}, or both. Each field is optional individually,
 * and presence is reported through {@link #checkHasSourceMirroringMode()} and
 * {@link #checkHasWarmStandbyMode()}. However, at least one of these payload
 * fields must be present when encoding or decoding a service payload.
 * <p>
 * When this object is populated as part of decoding a full directory
 * consumer-status message, a {@link MapEntry.MapAction#DELETE DELETE} entry may
 * legitimately contribute only {@link #serviceId()} and {@link #action()}, with
 * no payload fields present.
 *
 * @see WarmStandbyDirectoryServiceTypes
 * @see SourceMirroringMode
 */
public interface DirectoryConsumerStatusService
{
    /**
     * Clears this consumer-status service and prepares it for re-use.
     * <p>
     * This resets {@link #serviceId()} to {@code 0} and {@link #action()} to
     * {@link MapEntry.MapAction#ADD}. It also marks both payload fields as
     * absent, so {@link #checkHasSourceMirroringMode()} and
     * {@link #checkHasWarmStandbyMode()} return {@code false} until the
     * corresponding setters are called or a payload is decoded.
     * <p>
     * Implementations may still reset internal default values for later use,
     * but those defaults are not considered present after {@code clear()}.
     *
     * @return this directory consumer-status service instance
     */
    DirectoryConsumerStatusService clear();

    /**
     * Performs a deep copy of another service entry into this object.
     * <p>
     * If {@code sourceConsumerStatusService} is this object, the call is a no-op.
     *
     * @param sourceConsumerStatusService the service entry to copy from; must not be {@code null}
     * @return this directory consumer-status service instance
     *
     * @throws OmmInvalidUsageException if {@code sourceConsumerStatusService} is {@code null}
     */
    DirectoryConsumerStatusService copy(DirectoryConsumerStatusService sourceConsumerStatusService);

    /**
     * Decodes a consumer-status service payload from an EMA {@link ElementList}.
     * <p>
     * This method clears the current object and decodes only the element-list
     * payload. The enclosing directory map-entry metadata represented by
     * {@link #serviceId()} and {@link #action()} is not part of the supplied
     * {@link ElementList} and therefore remains the caller's responsibility.
     * <p>
     * Recognized payload fields are {@link #sourceMirroringMode()} and
     * {@link #warmStandbyMode()}. Either field may be supplied on its own, or
     * both may be supplied together, but both may not be absent. After
     * decoding, callers can use {@link #checkHasSourceMirroringMode()} and
     * {@link #checkHasWarmStandbyMode()} to determine which values were
     * supplied. Unrecognized elements are ignored.
     *
     * @param elementList the element payload representing this consumer-status service
     * @return this directory consumer-status service instance
     *
     * @throws OmmInvalidUsageException if {@code elementList} is {@code null},
     *                                  if decoding fails, if a recognized field
     *                                  contains an invalid value, or if both
     *                                  source-mirroring and warm-standby fields
     *                                  are absent
     */
    DirectoryConsumerStatusService decode(ElementList elementList);

    /**
     * Encodes this service entry's payload as an EMA {@link ElementList}.
     * <p>
     * Only payload fields are encoded by this method. The enclosing directory
     * map-entry metadata represented by {@link #serviceId()} and {@link #action()}
     * is not included.
     * <p>
     * Implementations encode whichever payload fields are currently present.
     * Either {@link #sourceMirroringMode()} or {@link #warmStandbyMode()} may be
     * encoded on its own, and both may be encoded together, but both may not be
     * absent.
     *
     * @return an {@link ElementList} representing this consumer-status service payload
     * @throws OmmInvalidUsageException if both payload fields are absent or if
     *                                  payload state is otherwise invalid
     */
    ElementList encode();

    /**
     * Returns the identifier of the service this status concerns.
     * <p>
     * This value identifies the enclosing directory {@link MapEntry} and is not
     * part of the {@link ElementList} produced by {@link #encode()} or consumed by
     * {@link #decode(ElementList)}.
     *
     * @return the service identifier
     */
    int serviceId();

    /**
     * Sets the identifier of the service this status concerns.
     * <p>
     * This value identifies the enclosing directory {@link MapEntry} and is not
     * part of the {@link ElementList} produced by {@link #encode()} or consumed by
     * {@link #decode(ElementList)}.
     *
     * @param serviceId the service identifier
     * @return this directory consumer-status service instance
     * @throws OmmInvalidUsageException if {@code serviceId} is outside the valid range
     */
    DirectoryConsumerStatusService serviceId(int serviceId);

    /**
     * Returns the action associated with this service's enclosing directory map entry.
     * <p>
     * This value describes how the enclosing directory {@link MapEntry} should be
     * interpreted and is not part of the {@link ElementList} payload handled by
     * {@link #encode()} and {@link #decode(ElementList)}.
     *
     * @return the map-entry action
     */
    int action();

    /**
     * Sets the action associated with this service's enclosing directory map entry.
     * <p>
     * This value affects the enclosing directory {@link MapEntry} rather than the
     * {@link ElementList} payload encoded or decoded by this interface.
     *
     * @param action the map-entry action, typically one of {@link MapEntry.MapAction}
     * @return this directory consumer-status service instance
     * @throws OmmInvalidUsageException if {@code action} is not a supported
     *                                  {@link MapEntry.MapAction} value
     */
    DirectoryConsumerStatusService action(int action);

    /**
     * Returns the source-mirroring mode for this service entry.
     * <p>
     * This field is optional. Call {@link #checkHasSourceMirroringMode()} before
     * calling this method. When present, it can be the only payload field, or it
     * can appear alongside {@link #warmStandbyMode()}.
     *
     * @return the source-mirroring mode
     * @throws OmmInvalidUsageException if {@link #checkHasSourceMirroringMode()} returns {@code false}
     */
    int sourceMirroringMode();

    /**
     * Sets the source-mirroring mode for this service entry.
     * <p>
     * Calling this method marks the source-mirroring field as present. This
     * field may be set with or without {@link #warmStandbyMode()}, provided at
     * least one of the two payload fields is present before {@link #encode()} is
     * called. Valid values are defined by {@link SourceMirroringMode}.
     *
     * @param sourceMirroringMode the source mirroring mode
     * @return this directory consumer-status service instance
     * @throws OmmInvalidUsageException if {@code sourceMirroringMode} is not one
     *                                  of the values defined in
     *                                  {@link SourceMirroringMode}
     */
    DirectoryConsumerStatusService sourceMirroringMode(int sourceMirroringMode);

    /**
     * Returns the warm-standby mode for this service entry.
     * <p>
     * This field is optional. Call {@link #checkHasWarmStandbyMode()} before
     * calling this method. When present, it can be the only payload field, or it
     * can appear alongside {@link #sourceMirroringMode()}.
     *
     * @return the warm-standby mode
     * @throws OmmInvalidUsageException if {@link #checkHasWarmStandbyMode()} returns {@code false}
     */
    int warmStandbyMode();

    /**
     * Sets the warm-standby mode for this service entry.
     * <p>
     * Calling this method marks the optional warm-standby field as present. This
     * field may be set with or without {@link #sourceMirroringMode()}, provided
     * at least one of the two payload fields is present before
     * {@link #encode()} is called. Valid values are defined by
     * {@link WarmStandbyDirectoryServiceTypes}.
     *
     * @param warmStandbyMode the warm-standby mode
     * @return this directory consumer-status service instance
     * @throws OmmInvalidUsageException if {@code warmStandbyMode} is not one of
     *                                  the values defined in
     *                                  {@link WarmStandbyDirectoryServiceTypes}
     */
    DirectoryConsumerStatusService warmStandbyMode(int warmStandbyMode);

    /**
     * Checks whether the optional warm-standby field is present.
     * <p>
     * A return value of {@code false} simply means this field is absent; the
     * payload may still be valid if {@link #checkHasSourceMirroringMode()}
     * returns {@code true}.
     *
     * @return {@code true} if {@link #warmStandbyMode()} is available; otherwise {@code false}
     */
    boolean checkHasWarmStandbyMode();

    /**
     * Checks whether the optional source-mirroring field is currently present.
     * <p>
     * A return value of {@code false} simply means this field is absent; the
     * payload may still be valid if {@link #checkHasWarmStandbyMode()} returns
     * {@code true}.
     *
     * @return {@code true} if {@link #sourceMirroringMode()} is available; otherwise {@code false}
     */
    boolean checkHasSourceMirroringMode();
}
