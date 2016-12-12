package com.thomsonreuters.upa.ansipage;

/**
 * The CellStyle enumeration is used to define the style of each PageCell object.
 */
public final class CellStyle implements Cloneable{

  private String _name;
  byte _value;
  boolean _underlineEnable = false;
  boolean _immutable = false;

  private CellStyle(String name, byte value)
  {
    _name = name;
    _value = value;
    _immutable = true;
  }

  /**
   * Makes a copy of this CellStyle. Changes to the copy will not affect the original and vice versa.
   * 
   * @return copy of this CellStyle
   */
  public Object clone()
  {
    return new CellStyle(_value);
  }

  /**
   * Returns a String object representing this CellStyle's color.
   * 
   * @return a string representation of the style of this CellStyle.
   */
  public String toString()
  {
    return _name;
  }

  /**
   * Constructs a CellStyle using a byte value
   * 
   * @param value - the value used to set the style of the CellStyle
   */
  public CellStyle(byte value)
  {
        switch (value)
        {
            case 0x00:
                _name = "plain";
                break;
            case 0x01:
                _name = "blink";
                break;
            case 0x02:
                _name = "reverse";
                break;
            case 0x04:
                _name = "dim";
                break;
            case 0x08:
                _name = "underline";
                break;
            case 0x10:
                _name = "bright";
                break;
            default:
                break;
        }
    _value = value;
  }

  /**
   * Plain
   */
  public final static CellStyle plain = new CellStyle("plain", (byte)0x00);
  /**
   * Blink
   */
  public final static CellStyle blink = new CellStyle("blink", (byte)0x01);
  /**
   * Reverse
   */
  public final static CellStyle reverse = new CellStyle("reverse", (byte)0x02);
  /**
   * Dim
   */
  public final static CellStyle dim = new CellStyle("dim", (byte)0x04);
  /**
   * Underline
   */
  public final static CellStyle underline = new CellStyle("underline", (byte)0x08);

  /**
   * Bright
   */
  public final static CellStyle bright = new CellStyle("bright", (byte)0x10);

  /**
   * Used to enable the underline
   * 
   * @param bUnderlinEnable Enable/Disable the Underline style of the cell. Type: boolean
   * 
   * @deprecated - replaced by {@link #setStyle(CellStyle) setStyle(CellStyle.underline)}
   */
  public void setUnderline(boolean bUnderlinEnable)
  {
    _underlineEnable = bUnderlinEnable;
  }

  /**
   * Used to determine whether the Underline is enabled
   * 
   * @return true if the Underline is enabled.
   * 
   * @deprecated - replaced by {@link #hasStyle(CellStyle)}
   */
  public boolean getUnderline()
  {
    return _underlineEnable;
  }

  /**
   * Returns a byte representing this CellStyle.
   * 
   * @return a byte representation of the style of this CellStyle.
    */
  byte toByte()
  {
    return _value;

  }

  /**
   * Default constructor that assigns plain and not underlined as the style of the CellStyle.
   */
  public CellStyle()
  {
    _name = "plain";
    _value = 0;
    _underlineEnable = false;
  }

  /**
   * @return true if this object contains the cell style any of the styles specified.
   */
    public boolean hasStyle(CellStyle c)
    {
        return (_value & c._value) != 0;
    }

    /**
     * Adds the style or styles to the existing cell styles stored in this object.
     * 
     * @throws UnsupportedOperationException if the object is one of the CellStyle constants (e.g. {@link #underline}).
     */
    public void setStyle(CellStyle c)
    {
        if (_immutable)
            throw new UnsupportedOperationException("Cannot modify a CellStyle constant");
        _value |= c._value;
    }

    /**
     * Clears all styles, leaving the style equivalent to {@link #plain}.
     */
    public void reset()
    {
        _value = 0;
    }

}


