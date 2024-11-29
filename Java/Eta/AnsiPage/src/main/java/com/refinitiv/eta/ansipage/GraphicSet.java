package com.refinitiv.eta.ansipage;

/**
 * The GraphicSet enumeration is used to define
 * the graphic set that will be associated with each PageCell object.
 */
public final class GraphicSet implements Cloneable{

  private byte _value;
  private String _name;

  /**
   * Makes a copy of this GraphicSet.
   * Changes to the copy will not affect the original and vice versa.
   * 
   * @return copy of this GraphicSet.
   * 
   * @deprecated - GraphicSet is immutable, so clone is not needed
   */
  @Deprecated
  public Object clone()
  {
    return this;
  }

  /**
   * Returns a String object representing this GraphicSet.
   * 
   * @return a string representation of the GraphicSet.
   */
  public String toString()
  {
    return _name;
  }

  /**
   * Returns a byte representing this GraphicSet.
   * 
   * @return a byte representation of the GraphicSet.
   */
  public byte toByte()
  {
    return _value;
  }

  /**
   * Field value indicating the GraphicSet. This is a GraphicSet value.
   * 
   * @see #USAscii
   * @see #UKAscii
   * @see #VT100Graphics
   * @see #chapdelaineAscii
   * @see #RMJAscii
   * @see #garbanAscii
   * @see #garbanGraphics
   * @see #mabonAscii
   * @see #mosaicGraphics
   * @see #FBIAscii
   * @see #FBIGraphics
   * @see #telerateAscii
   * @see #generalGraphics
   * @see #viewdataMosaic
   * @see #viewdataSeparatedMosaic
   */
  public final static GraphicSet

  /**
   * Unknown GraphicSet (default).
   */
  Unknown = new GraphicSet("Unknown", (byte)0),

  /**
   * USAscii is the United States ASCII (default).
   */
  USAscii = new GraphicSet("USAscii", (byte)'B'),
  /**
   * UKAscii is the United Kingdom ASCII.
   */
  UKAscii = new GraphicSet("UKAscii", (byte)'A'),

  /**
   * VT100Graphics is the VT100 Graphics.
   */
  VT100Graphics = new GraphicSet("VT100Graphics", (byte)'0'),

  /**
   * chapdelaineAscii is the Chapdelaine Special ASCII.
   */
  chapdelaineAscii = new GraphicSet("chapdelaineAscii", (byte)':'),

  /**
   * RMJAscii is the RMJ Special ASCII.
   */
  RMJAscii = new GraphicSet("RMJAscii", (byte)';'),

  /**
   * garbanAscii is the Garban Special ASCII.
   */
  garbanAscii = new GraphicSet("garbanAscii", (byte)'<'),

  /**
   * garbanGraphics is the Garban Graphics.
   */
  garbanGraphics = new GraphicSet("garbanGraphics", (byte)'b'),

  /**
   * mabonAscii is the Mabon Special ASCII.
   */
  mabonAscii = new GraphicSet("mabonAscii", (byte)'='),

  /**
   * mosaicGraphics is the Mosaic Graphic Set.
   */
  mosaicGraphics = new GraphicSet("mosaicGraphics", (byte)'>'),

  /**
   * FBIAscii is the FBI Special ASCII.
   */
  FBIAscii = new GraphicSet("FBIAscii", (byte)'?'),

  /**
   * FBIGraphics is the FBI Special Graphics.
   */
  FBIGraphics = new GraphicSet("FBIGraphics", (byte)'f'),

  /**
   * telerateAscii is the Telerate Special ASCII.
   */
  telerateAscii = new GraphicSet("telerateAscii", (byte)'s'),

  /**
   * topic is the Topic Character Set.
   */
  topic = new GraphicSet("topic", (byte)'t'),

  /**
   * generalGraphics is the General Graphics.
   */
  generalGraphics = new GraphicSet("generalGraphics", (byte)'g'),

  /**
   * viewdataMosaic is the Viewdata Mosaic.
   */
  viewdataMosaic = new GraphicSet("viewdataMosaic", (byte)'v'),

  /**
   * viewdataSeparatedMosaic is the Viewdata Separated Mosaic.
   */
  viewdataSeparatedMosaic = new GraphicSet("viewdataSeparatedMosaic", (byte)'w');


  /**
   * Create a named GraphicSet with a given integer value.
   * 
   * @param name name of the GraphicSet
   * @param value value of the GraphicSet
   */
  protected GraphicSet(String name, byte value)
  {
    _value = value;
    _name = new String();
    _name.concat(name);
  }

  /**
   * Constructs a GraphicSet using a byte value
   * 
   * @param value - the value used to set the GraphicSet.
   * 
   * @deprecated - static GraphicSet constants should be used
   */
  @Deprecated
  public GraphicSet(byte value)
  {
      this((int) value);
  }

  static GraphicSet [] GraphicSets = new GraphicSet[255];

  static GraphicSet getGraphicSet(int value)
  {
      if (value >= GraphicSets.length)
          return Unknown;
      GraphicSet gs = GraphicSets[value];
      if (gs == null)
          return Unknown;
      return gs;
  }

  /**
   * Constructs a GraphicSet using a byte value
   * 
   * @param value - the value used to set the GraphicSet.
   */
  private GraphicSet(int value)
  {
        switch (value)
        {
            case 0:
                _name = "Unknown";
                break;
            case 'B':
                _name = "USAscii";// /< United States ASCII (default)
                break;
            case 'A':
                _name = "UKAscii";
                break;
            case '0':
                _name = "VT100Graphics";
                break;
            case ':':
                _name = "chapdelaineAscii";
                break;
            case ';':
                _name = "RMJAscii";
                break;
            case '<':
                _name = "garbanAscii";
                break;
            case 'b':
                _name = "garbanGraphics";
                break;
            case '=':
                _name = "mabonAscii";
                break;
            case '>':
                _name = "mosaicGraphics";
                break;
            case '?':
                _name = "FBIAscii";
                break;
            case 'f':
                _name = "FBIGraphics";
                break;
            case 's':
                _name = "telerateAscii";
                break;
            case 't':
                _name = "topic";
                break;
            case 'g':
                _name = "generalGraphics";
                break;
            case 'v':
                _name = "viewdataMosaic";
                break;
            case 'w':
                _name = "viewdataSeparatedMosaic";
                break;
            default:
                _name = "";
        }
        
    _value = (byte)value;
    GraphicSets[value] = this;
  }
}

