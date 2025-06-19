/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Rdm;

namespace LSEG.Eta.Example.VACommon;

/// <summary>
/// Item argument class for the Value Add consumer and non-interactive provider applications.
/// </summary>
public class ItemArg
{
    public DomainType Domain { get; set; }
    public string? ItemName { get; set; }
    public bool EnablePrivateStream { get; set; }
    public bool EnableView { get; set; }
    public bool EnableSnapshot { get; set; }
    public bool EnableMsgKeyInUpdates { get; set; }
    public int ViewId { get; set; }
    public bool SymbolListData { get; set; }

    public ItemArg(DomainType domain, string itemName, bool enablePrivateStream)
    {
        Domain = domain;
        ItemName = itemName;
        EnablePrivateStream = enablePrivateStream;
    }

    // APIQA
    public ItemArg(DomainType domain, string itemName, bool enablePrivateStream, bool enableView, bool enableSnapshot, int viewId, bool enableMsgKeyInUpdates)
    {
        Domain = domain;
        ItemName = itemName;
        EnablePrivateStream = enablePrivateStream;
        EnableView = enableView;
        EnableSnapshot = enableSnapshot;
        EnableMsgKeyInUpdates = enableMsgKeyInUpdates;
        ViewId = viewId;
    }

    // END APIQA

    public ItemArg()
    {
    }
}
