/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.AnsiPage;

/// <summary>
/// The GraphicSet enumeration is used to define the graphic set that will be associated
/// with each PageCell object.
/// </summary>
public sealed class GraphicSet
{
    private readonly byte m_Value;
    private readonly string m_Name;

    /// <summary>
    /// Returns a String object representing this GraphicSet.
    /// </summary>
    ///
    /// <returns>a string representation of the GraphicSet.</returns>
    public override string ToString()
    {
        return m_Name;
    }

    /// <summary>
    /// Returns a byte representing this GraphicSet.
    /// </summary>
    ///
    /// <returns>a byte representation of the GraphicSet.</returns>
    public byte ToByte()
    {
        return m_Value;
    }

    /// <summary>
    /// Unknown GraphicSet (default).
    /// </summary>
    public readonly static GraphicSet Unknown = new GraphicSet("Unknown", (byte)0);
    /// <summary>
    /// USAscii is the United States ASCII (default).
    /// </summary>
    public readonly static GraphicSet USAscii = new GraphicSet("USAscii", (byte)'B');
    /// <summary>
    /// UKAscii is the United Kingdom ASCII.
    /// </summary>
    public readonly static GraphicSet UKAscii = new GraphicSet("UKAscii", (byte)'A');
    /// <summary>
    /// VT100Graphics is the VT100 Graphics.
    /// </summary>
    public readonly static GraphicSet VT100Graphics = new GraphicSet("VT100Graphics", (byte)'0');
    /// <summary>
    /// chapdelaineAscii is the Chapdelaine Special ASCII.
    /// </summary>
    public readonly static GraphicSet ChapdelaineAscii = new GraphicSet("chapdelaineAscii", (byte)':');
    /// <summary>
    /// RMJAscii is the RMJ Special ASCII.
    /// </summary>
    public readonly static GraphicSet RMJAscii = new GraphicSet("RMJAscii", (byte)';');
    /// <summary>
    /// garbanAscii is the Garban Special ASCII.
    /// </summary>
    public readonly static GraphicSet GarbanAscii = new GraphicSet("garbanAscii", (byte)'<');
    /// <summary>
    /// garbanGraphics is the Garban Graphics.
    /// </summary>
    public readonly static GraphicSet GarbanGraphics = new GraphicSet("garbanGraphics", (byte)'b');
    /// <summary>
    /// mabonAscii is the Mabon Special ASCII.
    /// </summary>
    public readonly static GraphicSet MabonAscii = new GraphicSet("mabonAscii", (byte)'=');
    /// <summary>
    /// mosaicGraphics is the Mosaic Graphic Set.
    /// </summary>
    public readonly static GraphicSet MosaicGraphics = new GraphicSet("mosaicGraphics", (byte)'>');
    /// <summary>
    /// FBIAscii is the FBI Special ASCII.
    /// </summary>
    public readonly static GraphicSet FBIAscii = new GraphicSet("FBIAscii", (byte)'?');
    /// <summary>
    /// FBIGraphics is the FBI Special Graphics.
    /// </summary>
    public readonly static GraphicSet FBIGraphics = new GraphicSet("FBIGraphics", (byte)'f');
    /// <summary>
    /// telerateAscii is the Telerate Special ASCII.
    /// </summary>
    public readonly static GraphicSet TelerateAscii = new GraphicSet("telerateAscii", (byte)'s');
    /// <summary>
    /// topic is the Topic Character Set.
    /// </summary>
    public readonly static GraphicSet Topic = new GraphicSet("topic", (byte)'t');
    /// <summary>
    /// generalGraphics is the General Graphics.
    /// </summary>
    public readonly static GraphicSet GeneralGraphics = new GraphicSet("generalGraphics", (byte)'g');
    /// <summary>
    /// viewdataMosaic is the Viewdata Mosaic.
    /// </summary>
    public readonly static GraphicSet ViewdataMosaic = new GraphicSet("viewdataMosaic", (byte)'v');
    /// <summary>
    /// viewdataSeparatedMosaic is the Viewdata Separated Mosaic.
    /// </summary>
    public readonly static GraphicSet ViewdataSeparatedMosaic = new GraphicSet("viewdataSeparatedMosaic", (byte)'w');

    /// <summary>
    /// Create a named GraphicSet with a given integer value.
    /// </summary>
    ///
    /// <param name="name">name of the GraphicSet</param>
    /// <param name="val"> value of the GraphicSet</param>
    internal GraphicSet(string name, byte val)
    {
        m_Value = val;
        m_Name = string.Empty;
        m_Name.Concat(name);
    }

    static readonly GraphicSet[] GraphicSets = new GraphicSet[255];

    internal static GraphicSet GetGraphicSet(int value)
    {
        if (value >= GraphicSets.Length)
            return Unknown;
        GraphicSet gs = GraphicSets[value];
        if (gs == null)
            return Unknown;
        return gs;
    }

    /// <summary>
    /// Constructs a GraphicSet using a byte value
    /// </summary>
    ///
    /// <param name="val">the value used to set the GraphicSet.</param>
    private GraphicSet(int val)
    {
        m_Name = val switch
        {
            '\0' => "Unknown",
            'B' => "USAscii", // United States ASCII (default)
            'A' => "UKAscii",
            '0' => "VT100Graphics",
            ':' => "chapdelaineAscii",
            ';' => "RMJAscii",
            '<' => "garbanAscii",
            'b' => "garbanGraphics",
            '=' => "mabonAscii",
            '>' => "mosaicGraphics",
            '?' => "FBIAscii",
            'f' => "FBIGraphics",
            's' => "telerateAscii",
            't' => "topic",
            'g' => "generalGraphics",
            'v' => "viewdataMosaic",
            'w' => "viewdataSeparatedMosaic",
            _ => string.Empty
        };

        m_Value = (byte)val;
        GraphicSets[val] = this;
    }
}
