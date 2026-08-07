/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceLink;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.BitSet;
import java.util.Iterator;

import static com.refinitiv.ema.access.DirectoryValidators.*;

/**
 * Default {@link DirectoryServiceLink} implementation.
 * <p>
 * Stores the data for a single Source Directory service-link entry describing an
 * upstream source associated with a service. The encoded payload is an
 * {@link ElementList} containing the link {@code type} when present, the required
 * {@code linkState}, and the optional {@code linkCode} and {@code text} fields.
 * The {@linkplain #name() name} is maintained separately and is not part of the
 * encoded {@link ElementList}; it is used as the map-entry key by the enclosing
 * link information filter.
 *
 * @see DirectoryServiceLink
 */
final class DirectoryServiceLinkImpl implements DirectoryServiceLink
{
    private String name;
    private int type;
    private int linkCode;
    private String text;
    private final BitSet flags = new BitSet();
    private int action; // map entry action
    private final StringBuilder stringBuilder = new StringBuilder();

    private static final int LINK_STATE_UP_FLAG = 0;
    private static final int HAS_TEXT_FLAG = 1;
    private static final int HAS_LINK_CODE_FLAG = 2;
    private static final int HAS_TYPE_FLAG = 3;
    private final static String EOL = System.lineSeparator();
    private final static String TAB = "\t";

    /**
     * Creates a service-link instance initialized to its default state.
     */
    DirectoryServiceLinkImpl()
    {
        clear();
    }

    /**
     * Resets this link to its default state.
     * <p>
     * After calling this method, {@linkplain #name() name} is an empty string,
     * {@linkplain #linkState() link state} is {@link EmaRdm.LinkStates#DOWN},
     * {@linkplain #action() action} is {@link MapEntry.MapAction#ADD}, and the
     * optional {@code type}, {@code linkCode}, and {@code text} fields are marked
     * as absent.
     *
     * @return this directory service link filter instance
     */
    @Override
    public DirectoryServiceLink clear()
    {
        flags.clear();
        name = "";
        type = EmaRdm.SERVICE_LINK_INTERACTIVE;
        linkCode = EmaRdm.SERVICE_LINK_CODE_NONE;
        text = "";
        action = MapEntry.MapAction.ADD;
        return this;
    }

    /**
     * Replaces the contents of this object with a deep copy of another service link.
     * <p>
     * This implementation clears the current content before copying values from
     * {@code sourceServiceLink}. Passing this instance returns without modifying
     * the object.
     *
     * @param sourceServiceLink source service link to copy from
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code sourceServiceLink} is {@code null}
     */
    @Override
    public DirectoryServiceLink copy(DirectoryServiceLink sourceServiceLink)
    {
        if (sourceServiceLink == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceServiceLink can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceServiceLink == this)
        {
            return this;
        }

        clear();

        if (sourceServiceLink.checkHasLinkCode())
        {
            linkCode(sourceServiceLink.linkCode());
        }

        if (sourceServiceLink.checkHasText())
        {
            text(sourceServiceLink.text());
        }

        if (sourceServiceLink.checkHasType())
        {
            type(sourceServiceLink.type());
        }

        name(sourceServiceLink.name());
        action(sourceServiceLink.action());
        linkState(sourceServiceLink.linkState());
        return this;
    }

    /**
     * Encodes this service link into an {@link ElementList} payload.
     * <p>
     * The returned payload always contains {@link EmaRdm#ENAME_LINK_STATE}. It also
     * contains {@link EmaRdm#ENAME_TYPE}, {@link EmaRdm#ENAME_LINK_CODE}, and
     * {@link EmaRdm#ENAME_TEXT} when the corresponding optional fields are present.
     * The {@linkplain #name() name} is not encoded.
     *
     * @return encoded {@link ElementList} representing this service link
     */
    @Override
    public ElementList encode()
    {
        ElementList elementList = EmaFactory.createElementList();

        if (checkHasType())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, type));
        }

        elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, linkState()));

        if (checkHasLinkCode())
        {
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_CODE, linkCode));
        }

        if (checkHasText())
        {
            elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_TEXT, text));
        }

        return elementList;
    }

    /**
     * Decodes an {@link ElementList} payload into this service link.
     * <p>
     * Existing content is cleared before decoding. The payload must contain
     * {@link EmaRdm#ENAME_LINK_STATE}; {@link EmaRdm#ENAME_TYPE},
     * {@link EmaRdm#ENAME_LINK_CODE}, and {@link EmaRdm#ENAME_TEXT} are optional.
     * Unknown elements are ignored. A blank {@code TEXT} element leaves the text
     * field absent. If decoding fails, this object remains in inconsistent state
     * and should be cleared for further usage.
     *
     * @param elementList encoded element list representing a service link
     * @return this directory service link filter instance
     *
     * @throws OmmInvalidUsageException if {@code elementList} is {@code null}, if a
     *                                  required element is missing, or if an element
     *                                  value cannot be decoded
     */
    @Override
    public DirectoryServiceLink decode(ElementList elementList)
    {
        if (elementList == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("elementList can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        clear();

        Iterator<ElementEntry> iterator = elementList.iteratorByRef();

        boolean foundLinkState = false;
        while (iterator.hasNext())
        {
            ElementEntry elementEntry = iterator.next();
            String elementName = elementEntry.name();

            switch (elementName)
            {
                case EmaRdm.ENAME_TYPE:
                    long type = elementEntry.uintValue();
                    if (type == EmaRdm.SERVICE_LINK_INTERACTIVE || type == EmaRdm.SERVICE_LINK_BROADCAST)
                    {
                        type((int) type);
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    break;
                case EmaRdm.ENAME_LINK_STATE:
                    long linkState = elementEntry.uintValue();
                    if (isValidLinkState(linkState))
                    {
                        linkState((int) linkState);
                    }
                    else
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }
                    foundLinkState = true;
                    break;
                case EmaRdm.ENAME_LINK_CODE:
                    long linkCodeValue = elementEntry.uintValue();
                    if (!isValidLinkCode(linkCodeValue))
                    {
                        throw new OmmInvalidUsageExceptionImpl().message("Invalid element value of " +
                                elementName, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
                    }

                    linkCode((int) linkCodeValue);
                    break;
                case EmaRdm.ENAME_TEXT:
                    if(elementEntry.code() != Data.DataCode.BLANK)
                    {
                        text(elementEntry.ascii().ascii());
                    }
                    break;
                default:
                    break;
            }
        }

        if(!foundLinkState)
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_LINK_STATE +
                    " element is absent", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }
        return this;
    }

    /**
     * Returns the name identifying this upstream source.
     * <p>
     * This value is not part of the encoded {@link ElementList}; it is carried as
     * the map-entry key by the enclosing link information filter.
     *
     * @return upstream source name
     */
    @Override
    public String name()
    {
        return name;
    }

    /**
     * Sets the name identifying this upstream source.
     * <p>
     * This value is not part of the encoded {@link ElementList}; it is carried as
     * the map-entry key by the enclosing link information filter.
     *
     * @param name upstream source name
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code name} is {@code null}
     */
    @Override
    public DirectoryServiceLink name(String name)
    {
        if (name == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("name can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.name = name;
        return this;
    }

    /**
     * Returns the type of this service link.
     * <p>
     * This is an optional field. Call {@link #checkHasType()} before calling this
     * method.
     *
     * @return one of {@link EmaRdm#SERVICE_LINK_INTERACTIVE} or
     *         {@link EmaRdm#SERVICE_LINK_BROADCAST}
     * @throws OmmInvalidUsageException if {@link #checkHasType()} returns
     *                                  {@code false}
     */
    @Override
    public int type()
    {
        if (!checkHasType())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_TYPE + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return type;
    }

    /**
     * Sets the type of this service link.
     * <p>
     * Calling this method marks the optional {@code type} field as present.
     *
     * @param type the link type
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code type} is not one of
     *                                  {@link EmaRdm#SERVICE_LINK_INTERACTIVE} or
     *                                  {@link EmaRdm#SERVICE_LINK_BROADCAST}
     */
    @Override
    public DirectoryServiceLink type(int type)
    {
        if (type != EmaRdm.SERVICE_LINK_INTERACTIVE && type != EmaRdm.SERVICE_LINK_BROADCAST)
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid type value " + type,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.type = type;
        applyHasType();
        return this;
    }

    /**
     * Returns the state indicating whether the source is up or down.
     * <p>
     * This field is always available. After {@link #clear()}, the default value is
     * {@link EmaRdm.LinkStates#DOWN}.
     *
     * @return one of {@link EmaRdm.LinkStates#UP} or {@link EmaRdm.LinkStates#DOWN}
     */
    @Override
    public int linkState()
    {
        return flags.get(LINK_STATE_UP_FLAG) ? EmaRdm.LinkStates.UP : EmaRdm.LinkStates.DOWN;
    }

    /**
     * Sets the state indicating whether the source is up or down.
     *
     * @param linkState the link state
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code linkState} is not one of
     *                                  {@link EmaRdm.LinkStates#UP} or
     *                                  {@link EmaRdm.LinkStates#DOWN}
     */
    @Override
    public DirectoryServiceLink linkState(int linkState)
    {
        if (!isValidLinkState(linkState))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid linkState value " + linkState,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (linkState == EmaRdm.LinkStates.UP)
        {
            flags.set(LINK_STATE_UP_FLAG);
        }
        else
        {
            flags.clear(LINK_STATE_UP_FLAG);
        }
        return this;
    }

    /**
     * Returns the code indicating additional information about the status of the
     * source.
     * <p>
     * This is an optional field. Call {@link #checkHasLinkCode()} before calling
     * this method.
     *
     * @return one of the supported {@link EmaRdm} {@code SERVICE_LINK_CODE_*}
     *         values
     * @throws OmmInvalidUsageException if {@link #checkHasLinkCode()} returns
     *                                  {@code false}
     */
    @Override
    public int linkCode()
    {
        if (!checkHasLinkCode())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_LINK_CODE + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return linkCode;
    }

    /**
     * Sets the code indicating additional information about the status of the
     * source.
     * <p>
     * Calling this method marks the optional {@code linkCode} field as present.
     *
     * @param linkCode the link code
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code linkCode} is not one of the
     *                                  supported {@link EmaRdm}
     *                                  {@code SERVICE_LINK_CODE_*} values
     */
    @Override
    public DirectoryServiceLink linkCode(int linkCode)
    {
        if (!isValidLinkCode(linkCode))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid linkCode value " + linkCode,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.linkCode = linkCode;
        applyHasLinkCode();
        return this;
    }

    /**
     * Returns text further describing the state provided by the {@code linkState}
     * and {@code linkCode} fields.
     * <p>
     * This is an optional field. Call {@link #checkHasText()} before calling this
     * method.
     *
     * @return descriptive link text
     * @throws OmmInvalidUsageException if {@link #checkHasText()} returns
     *                                  {@code false}
     */
    @Override
    public String text()
    {
        if (!checkHasText())
        {
            throw new OmmInvalidUsageExceptionImpl().message(EmaRdm.ENAME_TEXT + " element is not set",
                    OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
        }

        return text;
    }

    /**
     * Sets text further describing the current link state.
     * <p>
     * Calling this method marks the optional {@code text} field as present.
     *
     * @param text descriptive link text
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code text} is {@code null}
     */
    @Override
    public DirectoryServiceLink text(String text)
    {
        if (text == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("text can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.text  = text;
        applyHasText();
        return this;
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
     * Indicates whether the optional text field is present.
     *
     * @return {@code true} if the text field is present; {@code false} otherwise
     */
    @Override
    public boolean checkHasText()
    {
        return flags.get(HAS_TEXT_FLAG);
    }

    /**
     * Marks the optional text field as present.
     */
    private void applyHasText()
    {
        flags.set(HAS_TEXT_FLAG);
    }

    /**
     * Indicates whether the optional link-code field is present.
     *
     * @return {@code true} if the link-code field is present; {@code false}
     *         otherwise
     */
    @Override
    public boolean checkHasLinkCode()
    {
        return flags.get(HAS_LINK_CODE_FLAG);
    }

    /**
     * Marks the optional link-code field as present.
     */
    private void applyHasLinkCode()
    {
        flags.set(HAS_LINK_CODE_FLAG);
    }

    /**
     * Indicates whether the optional type field is present.
     *
     * @return {@code true} if the type field is present; {@code false} otherwise
     */
    @Override
    public boolean checkHasType()
    {
        return flags.get(HAS_TYPE_FLAG);
    }

    /**
     * Marks the optional type field as present.
     */
    private void applyHasType()
    {
        flags.set(HAS_TYPE_FLAG);
    }

    /**
     * Returns the action associated with this link filter.
     * <p>
     * Link filters use {@link MapEntry.MapAction} values.
     *
     * @return action associated with this link filter
     */
    @Override
    public int action()
    {
        return action;
    }

    /**
     * Sets the action associated with this link filter.
     *
     * @param action action populated from {@link MapEntry.MapAction}
     * @return this directory service link filter instance
     * @throws OmmInvalidUsageException if {@code action} is not a supported
     *                                  {@link MapEntry.MapAction} value
     */
    @Override
    public DirectoryServiceLink action(int action)
    {
        if (!isValidMapEntryAction(action))
        {
            throw new OmmInvalidUsageExceptionImpl().message("Invalid action value of " + action,
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        this.action = action;
        return this;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);

        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("name: ");
        stringBuilder.append(name());
        stringBuilder.append(EOL);

        if (checkHasType())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("linkType: ");
            stringBuilder.append(typeAsString(type()));
            stringBuilder.append(EOL);
        }

        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append(TAB);
        stringBuilder.append("linkState: ");
        stringBuilder.append(EmaRdm.LinkStates.asString(linkState()));
        stringBuilder.append(EOL);

        if (checkHasLinkCode())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("linkCode: ");
            stringBuilder.append(linkCodeAsString(linkCode()));
            stringBuilder.append(EOL);
        }

        if (checkHasText())
        {
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append(TAB);
            stringBuilder.append("linkText: ");
            stringBuilder.append(text());
            stringBuilder.append(EOL);
        }

        return stringBuilder.toString();
    }

    private String typeAsString(int type)
    {
        switch (type)
        {
            case EmaRdm.SERVICE_LINK_INTERACTIVE:
                return "INTERACTIVE";
            case EmaRdm.SERVICE_LINK_BROADCAST:
                return "BROADCAST";
            default:
                return Integer.toString(type);
        }
    }

    private String linkCodeAsString(int linkCode)
    {
        switch (linkCode)
        {
            case EmaRdm.SERVICE_LINK_CODE_NONE:
                return "NONE";
            case EmaRdm.SERVICE_LINK_CODE_OK:
                return "OK";
            case EmaRdm.SERVICE_LINK_CODE_RECOVERY_STARTED:
                return "RECOVERY_STARTED";
            case EmaRdm.SERVICE_LINK_CODE_RECOVERY_COMPLETED:
                return "RECOVERY_COMPLETED";
            default:
                return Integer.toString(linkCode);
        }
    }
}
