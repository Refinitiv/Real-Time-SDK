package com.thomsonreuters.upa.examples.consumerperf;

import java.util.List;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;


/**
 * Parses an item file
 */
class ItemFileParserImpl extends DefaultHandler implements ItemFileParser
{
    // XML tag names
    private static final String LATENCY_ITEM_TAG = "latencyItem";
    private static final String ITEM_LIST_TAG = "itemList";
    private static final String ITEM_TAG = "item";
    
    // XML attribute names
    private static final String DOMAIN_ATTR = "domain";
    private static final String NAME_ATTR = "name";
    private static final String MARKET_PRICE = "MarketPrice";
    
    private boolean _processingItemList;
    private boolean _latencyItemFound;
    private List<Item> _items;
    private List<Item> _latencyItems;

    // the types of items in the file
    private enum ItemTypes
    {
        latencyItem,
        item
    }
    
    /**
     * Initializes an ItemFileParserImpl
     */
    public ItemFileParserImpl()
    {
    }
    
    /* (non-Javadoc)
     * @see com.thomsonreuters..examples.common.ItemFileParser#parse(java.lang.String, java.util.List, java.util.List)
     */
    @Override
    public boolean parse(String filename, List<Item> items, List<Item> latencyItems)
    {
        boolean success = false;
        
        if (filename == null || filename.isEmpty() || items == null || latencyItems == null)
        {
            return success;
        }
        
        _processingItemList = false;
        _latencyItemFound = false;
        _items = items;
        _latencyItems = latencyItems;

        //get a factory
        SAXParserFactory spf = SAXParserFactory.newInstance();
        try
        {
            //get a new instance of parser
            SAXParser sp = spf.newSAXParser();

            //parse the file and also register this class for call backs
            sp.parse(filename, this);
            success = true;

        }
        catch (Exception e)
        {
            System.out.println(String.format("Error parsing file \"%s\": %s", filename, e.toString()));
            success = false;
        }
        finally
        {
            _items = null;
            _latencyItems = null;
        }
        
        return success;
    }
    
    
    /**
     * Receive notification of the start of an element.
     *
     * @param uri The Namespace URI, or the empty string if the
     *        element has no Namespace URI or if Namespace
     *        processing is not being performed.
     * @param localName The local name (without prefix), or the
     *        empty string if Namespace processing is not being
     *        performed.
     * @param qName The qualified name (with prefix), or the
     *        empty string if qualified names are not available.
     * @param attributes The attributes attached to the element.  If
     *        there are no attributes, it shall be an empty
     *        Attributes object.
     * @exception org.xml.sax.SAXException Any SAX exception, possibly
     *            wrapping another exception.
     * @see org.xml.sax.ContentHandler#startElement
     */
    @Override
    public void startElement(String namespaceURI,
                             String sName, // simple name
                             String qName, // qualified name
                             Attributes attrs) throws SAXException
    {
        switch (qName)
        {
            case ITEM_TAG :
                if (_processingItemList)
                {
                	if (_latencyItemFound)
                	{
                		processItem(ItemTypes.item, attrs);
                	}
                	else
                	{
                		processItem(ItemTypes.latencyItem, attrs);
                		_latencyItemFound = true;
                	}
                }
                break;
            case ITEM_LIST_TAG :
                _processingItemList = true;
                break;
            default:
                break;
        }
    }
    
    /**
     * Processes an item tag of the specified type
     * 
     * @param tag The type of tag being processed
     * @param attrs The attribute associated with the tag
     * 
     * @throws SAXException Thrown if the tag contains invalid content
     */
    private void processItem(ItemTypes type, Attributes attrs) throws SAXException
    {
        String domain = null;
        String name = null;
        
        if (attrs != null)
        {
            for (int i = 0; i < attrs.getLength(); i++)
            {
                if (DOMAIN_ATTR.equals(attrs.getLocalName(i)))
                {
                    domain = attrs.getValue(i);
                }
                else if (NAME_ATTR.equals(attrs.getLocalName(i)))
                {
                    name = attrs.getValue(i);
                }
            }
            
            validateParsedItem(domain, name);

            Domain d;
            switch (domain)
            {
                case MARKET_PRICE:
                    d = Domain.MarketPrice;
                    break;
                default:
                    throw new SAXException(String.format("ERROR: \"%s\" is an invalid value for the %s tag's %s attribute", domain, LATENCY_ITEM_TAG, DOMAIN_ATTR));
            }
            
            Item item = new ItemImpl(d, name);
            addItem(type, item);
        }
    }
    
    private void validateParsedItem(String domain, String name) throws SAXException
    {
        if (domain == null || domain.isEmpty())
        {
            throw new SAXException(String.format("ERROR: the %s attribute is required for the %s tag", DOMAIN_ATTR, LATENCY_ITEM_TAG));
        }
        
        if (name == null || name.isEmpty())
        {
            throw new SAXException(String.format("ERROR: the %s attribute is required for the %s tag", NAME_ATTR, LATENCY_ITEM_TAG));
        }
    }
    
    private void addItem(ItemTypes type, Item item)
    {
        switch (type)
        {
            case item :
                _items.add(item);        
                break;
            case latencyItem :
                _latencyItems.add(item);
                break;
            default:
                assert (false);
                break;
        }        
    }
    
}