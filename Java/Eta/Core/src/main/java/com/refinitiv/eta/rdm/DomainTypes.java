/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.rdm;

/**
 * Domain Type enumeration values, see RDM Usage Guide for domain model
 * definitions (DMT = DomainType). This will be extended as new message types
 * are defined and implemented.
 */
public class DomainTypes
{
    // DomainTypes class cannot be instantiated
    private DomainTypes()
    {
        throw new AssertionError();
    }

    /** Login Message */
    public static final int LOGIN = 1;
    /** Source Message */
    public static final int SOURCE = 4;
    /** Dictionary Message */
    public static final int DICTIONARY = 5;
    /** Market Price Message */
    public static final int MARKET_PRICE = 6;
    /** Market by Order/Order Book Message */
    public static final int MARKET_BY_ORDER = 7;
    /** Market by Price/Market Depth Message */
    public static final int MARKET_BY_PRICE = 8;
    /** Market Maker Message */
    public static final int MARKET_MAKER = 9;
    /** Symbol List Messages */
    public static final int SYMBOL_LIST = 10;
    /** Service Provider Status */
    public static final int SERVICE_PROVIDER_STATUS = 11;
    /** History Message */
    public static final int HISTORY = 12;
    /** Headline Message */
    public static final int HEADLINE = 13;
    /** Story Message */
    public static final int STORY = 14;
    /** Replay Headline Message */
    public static final int REPLAYHEADLINE = 15;
    /** Replay Story Message */
    public static final int REPLAYSTORY = 16;
    /** Transaction Message */
    public static final int TRANSACTION = 17;
    /** Yield Curve */
    public static final int YIELD_CURVE = 22;
    /** Contribution */
    public static final int CONTRIBUTION = 27;
    /* 28 reserved for now */
    /** Provider Message */
    public static final int PROVIDER_ADMIN = 29;
	/** Analytics */
	public static final int ANALYTICS = 30;
	/** Reference */
	public static final int REFERENCE = 31;
	/** News Text Analytics */
	public static final int NEWS_TEXT_ANALYTICS = 33;
	/** Economic Indicator domain */
	public static final int ECONOMIC_INDICATOR = 34;
	/** Poll domain */
	public static final int POLL = 35;
	/** Forecast domain */
	public static final int FORECAST = 36;
	/** Market By Time domain */
	public static final int MARKET_BY_TIME = 37;
	/** System domain for use with domain neutral content (e.g. tunnel stream creation) */
	public static final int SYSTEM = 127;
    /* Maximum reserved message type value */
    static final int MAX_RESERVED = 127;
    /* Maximum value for a message type */
    static final int MAX_VALUE = 255;

    /**
     * String representation of a domain type name.
     * 
     * @param domainType domain type
     * 
     * @return the string representation of a domain type name
     */
    public static String toString(int domainType)
    {
        String ret = "";

        switch (domainType)
        {
            case LOGIN:
                ret = "LOGIN";
                break;
            case SOURCE:
                ret = "SOURCE";
                break;
            case DICTIONARY:
                ret = "DICTIONARY";
                break;
            case MARKET_PRICE:
                ret = "MARKET_PRICE";
                break;
            case MARKET_BY_ORDER:
                ret = "MARKET_BY_ORDER";
                break;
            case MARKET_BY_PRICE:
                ret = "MARKET_BY_PRICE";
                break;
            case MARKET_MAKER:
                ret = "MARKET_MAKER";
                break;
            case SYMBOL_LIST:
                ret = "SYMBOL_LIST";
                break;
            case SERVICE_PROVIDER_STATUS:
            	ret = "SERVICE_PROVIDER_STATUS";
            	break;
            case HISTORY:
                ret = "HISTORY";
                break;
            case HEADLINE:
                ret = "HEADLINE";
                break;
            case STORY:
                ret = "STORY";
                break;
            case REPLAYHEADLINE:
                ret = "REPLAYHEADLINE";
                break;
            case REPLAYSTORY:
                ret = "REPLAYSTORY";
                break;
            case TRANSACTION:
                ret = "TRANSACTION";
                break;
            case YIELD_CURVE:
                ret = "YIELD_CURVE";
                break;
            case CONTRIBUTION:
                ret = "CONTRIBUTION";
                break;
            case PROVIDER_ADMIN:
                ret = "PROVIDER_ADMIN";
                break;
			case ANALYTICS:
				ret = "ANALYTICS";
				break;
			case REFERENCE:
				ret = "REFERENCE";
				break;
			case NEWS_TEXT_ANALYTICS:
				ret = "NEWS_TEXT_ANALYTICS";
				break;
			case ECONOMIC_INDICATOR:
				ret = "ECONOMIC_INDICATOR";
				break;
			case POLL:
				ret = "POLL";
				break;
			case FORECAST:
				ret = "FORECAST";
				break;
			case MARKET_BY_TIME:
				ret = "MARKET_BY_TIME";
				break;
            case SYSTEM:
                ret = "SYSTEM";
                break;
            default:
                ret = Integer.toString(domainType);
                break;
        }

        return ret;
    }

    /**
     * Returns domainType value from domain type string.
     * 
     * @param domainTypeString domain type string representation
     * 
     * @return the domainType value
     */
    public static int domainTypeFromString(String domainTypeString)
    {
        int ret = 0;

        if (domainTypeString.equals("LOGIN"))
        {
            ret = LOGIN;
        }
        else if (domainTypeString.equals("SOURCE"))
        {
            ret = SOURCE;
        }
        else if (domainTypeString.equals("DICTIONARY"))
        {
            ret = DICTIONARY;
        }
        else if (domainTypeString.equals("MARKET_PRICE"))
        {
            ret = MARKET_PRICE;
        }
        else if (domainTypeString.equals("MARKET_BY_ORDER"))
        {
            ret = MARKET_BY_ORDER;
        }
        else if (domainTypeString.equals("MARKET_BY_PRICE"))
        {
            ret = MARKET_BY_PRICE;
        }
        else if (domainTypeString.equals("MARKET_MAKER"))
        {
            ret = MARKET_MAKER;
        }
        else if (domainTypeString.equals("SYMBOL_LIST"))
        {
            ret = SYMBOL_LIST;
        }
        else if (domainTypeString.equals("SERVICE_PROVIDER_STATUS"))
        {
        	ret = SERVICE_PROVIDER_STATUS;
        }
        else if (domainTypeString.equals("HISTORY"))
        {
            ret = HISTORY;
        }
        else if (domainTypeString.equals("HEADLINE"))
        {
            ret = HEADLINE;
        }
        else if (domainTypeString.equals("STORY"))
        {
            ret = STORY;
        }
        else if (domainTypeString.equals("REPLAYHEADLINE"))
        {
            ret = REPLAYHEADLINE;
        }
        else if (domainTypeString.equals("REPLAYSTORY"))
        {
            ret = REPLAYSTORY;
        }
        else if (domainTypeString.equals("TRANSACTION"))
        {
            ret = TRANSACTION;
        }
        else if (domainTypeString.equals("YIELD_CURVE"))
        {
            ret = YIELD_CURVE;
        }
        else if (domainTypeString.equals("CONTRIBUTION"))
        {
            ret = CONTRIBUTION;
        }
        else if (domainTypeString.equals("PROVIDER_ADMIN"))
        {
            ret = PROVIDER_ADMIN;
        }
		else if (domainTypeString.equals("ANALYTICS"))
		{
			ret = ANALYTICS;
		}
		else if (domainTypeString.equals("REFERENCE"))
		{
			ret = REFERENCE;
		}
		else if (domainTypeString.equals("NEWS_TEXT_ANALYTICS"))
		{
			ret = NEWS_TEXT_ANALYTICS;
		}
		else if (domainTypeString.equals("ECONOMIC_INDICATOR"))
		{
			ret = ECONOMIC_INDICATOR;
		}
		else if (domainTypeString.equals("POLL"))
		{
			ret = POLL;
		}
		else if (domainTypeString.equals("FORECAST"))
		{
			ret = FORECAST;
		}
		else if (domainTypeString.equals("MARKET_BY_TIME"))
		{
			ret = MARKET_BY_TIME;
		}
		else if (domainTypeString.equals("SYSTEM"))
		{
			ret = SYSTEM;
		}

        return ret;
    }
}
