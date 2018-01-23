package com.thomsonreuters.upa.valueadd.examples.common;

/** Item argument class for the Value Add consumer and
 * non-interactive provider applications. */
public class ItemArg
{
    int domain; /* Domain of an item */
    String itemName; /* Name of an item */
    boolean enablePrivateStream; /* enable private stream for this item */
    // APIQA: adding flags and viewId
    boolean enableView = false; /* enable private stream for this item */
    boolean enableSnapshot = false; /* enable private stream for this item */
    boolean enableMsgKeyInUpdates = false; /* enable msgKeyInUpdates for this item */
    int viewId = 0; /* enable private stream for this item */
    // END APIQA:
    boolean symbolListData; /* enable symbollist datastream */

    public ItemArg(int domain, String itemName, boolean enablePrivateStream)
    {
        this.domain = domain;
        this.itemName = itemName;
        this.enablePrivateStream = enablePrivateStream;
        this.symbolListData = false;
    }

    // APIQA
    public ItemArg(int domain, String itemName, boolean enablePrivateStream, boolean enableView, boolean enableSnapshot, int viewId, boolean enableMsgKeyInUpdates)
    {
        this.domain = domain;
        this.itemName = itemName;
        this.enablePrivateStream = enablePrivateStream;
        this.symbolListData = false;
        this.enableView = enableView;
        this.enableSnapshot = enableSnapshot;
        this.enableMsgKeyInUpdates = enableMsgKeyInUpdates;
        this.viewId = viewId;
    }

    // END APIQA

    public ItemArg()
    {
    }

    public int domain()
    {
        return domain;
    }

    public void domain(int domain)
    {
        this.domain = domain;
    }

    public String itemName()
    {
        return itemName;
    }

    public void itemName(String itemName)
    {
        this.itemName = itemName;
    }

    // APIQA:
    public boolean enableSnapshot()
    {
        return enableSnapshot;
    }

    public void enableSnapshot(boolean isSnapshot)
    {
        this.enableSnapshot = isSnapshot;
    }

    public boolean enableView()
    {
        return enableView;
    }

    public void enableView(boolean isView)
    {
        this.enableView = isView;
    }

    public int viewId()
    {
        return viewId;
    }

    public void viewId(int viewId)
    {
        this.viewId = viewId;
    }
    public boolean enableMsgKeyInUpdates()
    {
        return enableMsgKeyInUpdates;
    }

    public void enableMsgKeyInUpdates(boolean isMsgKeyInUpdates)
    {
        this.enableMsgKeyInUpdates = isMsgKeyInUpdates;
    }
    // END APIQA:

    public boolean enablePrivateStream()
    {
        return enablePrivateStream;
    }

    public void enablePrivateStream(boolean enablePrivateStream)
    {
        this.enablePrivateStream = enablePrivateStream;
    }

    public boolean symbolListData()
    {
        return symbolListData;
    }

    public void symbolListData(boolean symbolListData)
    {
        this.symbolListData = symbolListData;
    }
}
