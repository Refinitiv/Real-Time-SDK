/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceLink;
import com.refinitiv.ema.domain.directory.DirectoryServiceLinkInfo;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.ArrayList;
import java.util.List;

import static com.refinitiv.ema.access.DirectoryValidators.isValidMapEntryAction;

/**
 * Default {@link DirectoryServiceLinkInfo} implementation.
 * <p>
 * Stores the RDM Source Directory service-link filter for a service. The
 * filter contains the set of upstream sources associated with the service and
 * is encoded as a {@link Map} whose keys are link names and whose entry
 * payloads are the {@link ElementList} values produced by
 * {@link DirectoryServiceLink#encode()}.
 *
 * @see DirectoryServiceLinkInfo
 */
final class DirectoryServiceLinkInfoImpl implements DirectoryServiceLinkInfo
{
    private final List<DirectoryServiceLink> linkList = new ArrayList<>();
    private int action = MapEntry.MapAction.ADD; //filter entry action
    private final StringBuilder stringBuilder = new StringBuilder();

    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    /**
     * Creates a service-link filter instance initialized to its default state.
     */
    DirectoryServiceLinkInfoImpl()
    {
    }

    /**
     * Returns the service-link entries describing the service's upstream
     * sources.
     *
     * @return list of service-link entries
     */
    @Override
    public List<DirectoryServiceLink> linkList()
    {
        return linkList;
    }

    /**
     * Replaces this filter's service-link entries with the supplied list.
     * <p>
     * The current list is cleared before the supplied entries are added. The
     * list reference itself is not retained.
     *
     * @param linkList list of service-link entries
     * @return this directory service link-info filter instance
     * @throws OmmInvalidUsageException if {@code linkList} is {@code null}
     */
    @Override
    public DirectoryServiceLinkInfo linkList(List<DirectoryServiceLink> linkList)
    {
        if (linkList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("linkList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (this.linkList == linkList)
        {
            return this;
        }

        this.linkList.clear();
        this.linkList.addAll(linkList);
        return this;
    }

    /**
     * Sets the action associated with this service-link filter.
     *
     * @param action action populated from {@link MapEntry.MapAction}
     * @return this directory service link-info filter instance
     * @throws OmmInvalidUsageException if {@code action} is not a supported
     *                                  {@link MapEntry.MapAction} value
     */
    @Override
    public DirectoryServiceLinkInfo action(int action)
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
     * Returns the action associated with this service-link filter.
     * <p>
     * Service-link filters use {@link MapEntry.MapAction} values.
     *
     * @return action associated with this service-link filter
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Returns the fixed RDM filter identifier for service-link data.
     *
     * @return {@link EmaRdm#SERVICE_LINK_ID}
     */
    @Override
    public int filterId()
    {
        return EmaRdm.SERVICE_LINK_ID;
    }

    /**
     * Encodes this service-link filter into a {@link Map} payload.
     * <p>
     * Each map entry uses the link {@linkplain DirectoryServiceLink#name() name}
     * as the ASCII key, the link {@linkplain DirectoryServiceLink#action() action}
     * as the map-entry action, and the encoded
     * {@link DirectoryServiceLink#encode() ElementList} as the entry payload.
     *
     * @return encoded {@link Map} representing this service-link filter
     */
    @Override
    public Map encode()
    {
        Map map = EmaFactory.createMap();
        for (DirectoryServiceLink serviceLink : linkList)
        {
            MapEntry mapEntry = EmaFactory.createMapEntry();
            mapEntry.keyAscii(serviceLink.name(), serviceLink.action(), serviceLink.encode());
            map.add(mapEntry);
        }

        return map;
    }

    /**
     * Decodes a {@link Map} payload into this service-link filter.
     * <p>
     * Existing content is cleared before decoding. Each map entry must use an
     * ASCII key that becomes the link {@linkplain DirectoryServiceLink#name() name}.
     * For {@code DELETE} actions, a {@code NO_DATA} payload is accepted. All other
     * entries must carry an {@link ElementList} payload that is decoded into the
     * corresponding {@link DirectoryServiceLink}. If decoding fails, this object
     * remains in inconsistent state and should be cleared for further usage.
     *
     * @param map encoded map representing a service-link filter
     * @return this directory service link-info filter instance
     * @throws OmmInvalidUsageException if {@code map} is {@code null}, if a map
     *                                  entry uses an unexpected key or payload
     *                                  type, or if a link entry cannot be decoded
     */
    @Override
    public DirectoryServiceLinkInfo decode(Map map)
    {
        if (map == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("map can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        for (MapEntry mapEntry : map)
        {
            if (mapEntry.key().data().dataType() != DataType.DataTypes.ASCII)
            {
                throw new OmmInvalidUsageExceptionImpl().message("Unexpected map entry key type: " +
                                DataType.asString(mapEntry.key().data().dataType()),
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
            }

            if (mapEntry.loadType() == DataType.DataTypes.ELEMENT_LIST ||
                    (mapEntry.action() == MapEntry.MapAction.DELETE && mapEntry.loadType() == DataType.DataTypes.NO_DATA))
            {
                DirectoryServiceLink serviceLink = new DirectoryServiceLinkImpl();
                if (mapEntry.loadType() == DataType.DataTypes.ELEMENT_LIST)
                {
                    serviceLink.decode(mapEntry.elementList());
                }
                serviceLink.name(mapEntry.key().ascii().ascii());
                serviceLink.action(mapEntry.action());
                linkList.add(serviceLink);
            }
            else
            {
                throw new OmmInvalidUsageExceptionImpl().message("Unexpected map entry payload: " +
                                DataType.asString(mapEntry.loadType()),
                        OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
            }
        }
        return this;
    }

    /**
     * Resets this service-link filter to its default state.
     * <p>
     * After calling this method, the link list is empty and
     * {@linkplain #action() action} is {@link MapEntry.MapAction#ADD}.
     *
     * @return this directory service link-info filter instance
     */
    @Override
    public DirectoryServiceLinkInfo clear()
    {
        linkList.clear();
        action = MapEntry.MapAction.ADD;
        return this;
    }

    /**
     * Replaces the contents of this object with a deep copy of another
     * service-link filter.
     * <p>
     * This implementation clears the current content before copying the source
     * filter's {@linkplain #action() action} and deep copies of its
     * {@link DirectoryServiceLink} entries. Passing this instance returns
     * without modifying the object.
     *
     * @param sourceServiceLinkInfo source service-link filter to copy from
     * @return this directory service link-info filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceLinkInfo} is {@code null}
     */
    @Override
    public DirectoryServiceLinkInfo copy(DirectoryServiceLinkInfo sourceServiceLinkInfo)
    {
        if (sourceServiceLinkInfo == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceServiceLinkInfo can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceServiceLinkInfo == this)
        {
            return this;
        }

        clear();
        action(sourceServiceLinkInfo.action());
        for (DirectoryServiceLink serviceLink : sourceServiceLinkInfo.linkList())
        {
            DirectoryServiceLink newServiceLink = new DirectoryServiceLinkImpl();
            newServiceLink.copy(serviceLink);
            linkList.add(newServiceLink);
        }
        return this;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);

        for (DirectoryServiceLink link : linkList)
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("LinkFilter: ");
            stringBuilder.append(EOL);
            stringBuilder.append(link);
        }

        return stringBuilder.toString();
    }
}
