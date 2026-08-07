/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.rdm.EmaRdm;

/**
 * Represents a single RDM Source Directory service link entry describing an
 * upstream source associated with a service.
 * <p>
 * A service link is encoded as an {@link ElementList} when used as the payload
 * of a link entry. The link {@linkplain #name() name} itself is not part of that
 * {@link ElementList}; it is used by {@link DirectoryServiceLinkInfo} as the map key
 * for the enclosing link entry.
 * <p>
 * The {@linkplain #action() action} inherited from {@link DirectoryServiceFilter}
 * uses {@link com.refinitiv.ema.access.MapEntry.MapAction} values.
 * Required state includes {@linkplain #linkState() link state}. Optional fields are
 * {@linkplain #type() type}, {@linkplain #linkCode() link code}, and {@linkplain #text() text},
 * each guarded by a corresponding {@code checkHas*()} method.
 *
 * @see DirectoryServiceFilter
 */
public interface DirectoryServiceLink extends DirectoryServiceFilter<ElementList>
{
    /**
     * Clears this service-link filter and resets it to its default state.
     * <p>
     * This removes all optional link fields, restores the default link state,
     * resets the link name, and restores the default map-entry action.
     *
     * @return this directory service link filter instance
     */
    @Override
    DirectoryServiceLink clear();

    /**
     * Sets the map-entry action associated with this service-link filter.
     * <p>
     * Valid values typically come from
     * {@link com.refinitiv.ema.access.MapEntry.MapAction}.
     *
     * @param action the map-entry action
     * @return this directory service link filter instance
     */
    @Override
    DirectoryServiceLink action(int action);

    /**
     * Decodes an {@link ElementList} payload into this service-link filter.
     * <p>
     * Implementations replace the current content with the decoded link-filter
     * data.
     *
     * @param struct encoded {@link ElementList} representing the service-link filter
     * @return this directory service link filter instance
     */
    @Override
    DirectoryServiceLink decode(ElementList struct);

    /**
     * Replaces the contents of this object with a deep copy of another service
     * link.
     * <p>
     * Implementations typically clear the current content before copying values
     * from {@code sourceServiceLink}.
     *
     * @param sourceServiceLink source service link to copy from
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceLink} is {@code null}
     */
    DirectoryServiceLink copy(DirectoryServiceLink sourceServiceLink);

    /**
     * Returns the name identifying this upstream source.
     * <p>
     * This value is not part of the {@link ElementList} returned by
     * {@link #encode()}; it is used as the map-entry key when encoded by
     * {@link DirectoryServiceLinkInfo}.
     *
     * @return upstream source name
     */
    String name();

    /**
     * Sets the name identifying this upstream source.
     * <p>
     * This value is not part of the {@link ElementList} returned by
     * {@link #encode()}; it is used as the map-entry key when encoded by
     * {@link DirectoryServiceLinkInfo}.
     *
     * @param name the name
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code name} is {@code null}
     */
    DirectoryServiceLink name(String name);

    /**
     * Returns the type of this service link.
     * <p>
     * This is an optional field. Call {@link #checkHasType()} before calling this
     * method.
     *
     * @return one of the {@link EmaRdm} {@code SERVICE_LINK_*} constants
     * @throws OmmInvalidUsageException if {@link #checkHasType()} returns {@code false}
     */
    int type();

    /**
     * Sets the type of this service link.
     * <p>
     * Calling this method marks the optional {@code type} field as present.
     *
     * @param type the type
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if type is not one of {@link EmaRdm#SERVICE_LINK_INTERACTIVE}
     * or {@link EmaRdm#SERVICE_LINK_BROADCAST}
     */
    DirectoryServiceLink type(int type);

    /**
     * Returns the state indicating whether the source is up or down.
     * <p>
     * This field is always available. After {@link #clear()}, the default value is
     * {@link EmaRdm.LinkStates#DOWN}.
     *
     * @return one of {@link EmaRdm.LinkStates#UP} or {@link EmaRdm.LinkStates#DOWN}
     */
    int linkState();

    /**
     * Sets the state indicating whether the source is up or down.
     *
     * @param linkState the link state
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if linkState is not one of
     * {@link EmaRdm.LinkStates#UP} or {@link EmaRdm.LinkStates#DOWN}
     */
    DirectoryServiceLink linkState(int linkState);

    /**
     * Returns the code indicating additional information about the status of the
     * source.
     * <p>
     * This is an optional field. Call {@link #checkHasLinkCode()} before calling
     * this method.
     *
     * @return linkCode
     * @throws OmmInvalidUsageException if {@link #checkHasLinkCode()} returns {@code false}
     */
    int linkCode();

    /**
     * Sets the code indicating additional information about the status of the
     * source.
     * <p>
     * Calling this method marks the optional {@code linkCode} field as present.
     *
     * @param linkCode the link code
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if linkCode is not one of the supported
     * {@link EmaRdm} {@code SERVICE_LINK_CODE_*} values
     */
    DirectoryServiceLink linkCode(int linkCode);

    /**
     * Returns text further describing the state provided by the {@code linkState}
     * and {@code linkCode} fields.
     * <p>
     * This is an optional field. Call {@link #checkHasText()} before calling this
     * method.
     *
     * @return text
     * @throws OmmInvalidUsageException if {@link #checkHasText()} returns {@code false}
     */
    String text();

    /**
     * Sets the text further describing the current link state.
     * <p>
     * Calling this method marks the optional {@code text} field as present.
     *
     * @param text the text
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code text} is {@code null}
     */
    DirectoryServiceLink text(String text);

    /**
     * Indicates presence of the link text field.
     *
     * @return true - if text field is present, false - if not.
     */
    boolean checkHasText();

    /**
     * Indicates presence of the link code field.
     *
     * @return true - if link code field is present, false - if not.
     */
    boolean checkHasLinkCode();

    /**
     * Indicates presence of the link type field.
     *
     * @return true - if link type field is present, false - if not.
     */
    boolean checkHasType();
}
