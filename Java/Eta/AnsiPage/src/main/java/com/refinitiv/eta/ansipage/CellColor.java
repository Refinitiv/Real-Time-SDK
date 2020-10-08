package com.refinitiv.eta.ansipage;

import java.awt.Color;

/**
 * The CellColor enumeration is used to define the foreground
 * and background colors for each PageCell object.
 */
public final class CellColor implements Cloneable
{
   private String _name;
   private byte _value;
   Color _color;


   /**
    * Makes a copy of this CellColor. Changes to the copy will not affect the original and vice versa.
    * 
    * @return copy of this CellColor
    * 
    * @deprecated - CellColor is immutable, so clone is not needed
    */
   public Object clone()
   {
     return this;
   }

   /**
    * Returns a String object representing this CellColor's color.
    * 
    * @return a string representation of the color of this CellColor.
    */
   public String toString()
   {
     return _name;
   }

   /**
    * Returns a byte representing this CellColor.
    * 
    * @return a byte representation of the color of this CellColor
    */

   public byte toByte()
   {
       return _value;
   }

   /**
    * Default constructor that assigns MONO as the color of the CellColor
    * 
    * @deprecated - static CellColor constants should be used
    */
   public CellColor()
   {
     _name = "mono";
     _value = 0;
     _color = Color.BLACK;
   }

   /**
    * Constructs a CellColor using a byte value
    * 
    * @param value - the value used to set the color of the CellColor
    * 
    * @deprecated - static CellColor constants should be used
    */
   public CellColor(byte value)
   {
       this((int)value);
   }

   /**
    * Constructs a CellColor using a byte value
    * 
    * @param value - the value used to set the color of the CellColor
    */
   private CellColor(int value)
   {
        switch (value)
        {
            case 0x0f:
                _name = "mono";
                _color = Color.BLACK;
                break;
            case 0x00:
                _name = "black";
                _color = Color.BLACK;
                break;
            case 0x01:
                _name = "red";
                _color = Color.RED;
                break;
            case 0x02:
                _name = "green";
                _color = Color.GREEN;
                break;
            case 0x03:
                _name = "yellow";
                _color = Color.YELLOW;
                break;
            case 0x04:
                _name = "blue";
                _color = Color.BLUE;
                break;
            case 0x05:
                _name = "magenta";
                _color = Color.MAGENTA;
                break;
            case 0x06:
                _name = "cyan";
                _color = Color.CYAN;
                break;
            case 0x07:
                _name = "white";
                _color = Color.WHITE;
                break;
            default:
                break;
        }
     _value = (byte)value;
   }

   /**
    * Field value indicating the CellColor.
    * This is a CellColor value.
    * The first color of the CellColor is <code>black</code> which is 0, the last is mono which is 15.
    * 
    * @see #black
    * @see #red
    * @see #green
    * @see #yellow
    * @see #blue
    * @see #magenta
    * @see #cyan
    * @see #white
    * @see #mono
    */
  public final static CellColor
  /**
   * Value of the CellColor indicating Mono.
   */
   mono = new CellColor(0x0f),
   /**
   * Value of the CellColor indicating Black.
   */
   black = new CellColor(0x00),
   /**
    * Value of the CellColor indicating Red.
    */
   red = new CellColor(0x01),
   /**
    * Value of the CellColor indicating Green.
    */
   green = new CellColor(0x02),
   /**
    * Value of the CellColor indicating Yellow.
   */
   yellow = new CellColor(0x03),
   /**
   * Value of the CellColor indicating Blue.
   */
   blue = new CellColor(0x04),
   /**
    * Value of the CellColor indicating Magenta.
    */
   magenta = new CellColor(0x05),
   /**
    * Value of the CellColor indicating Cyan.
    */
   cyan = new CellColor(0x06),
   /**
    * Value of the CellColor indicating White.
    */
   white = new CellColor(0x07);

   /**
    * Array of CellColor
    */
   public final static CellColor[] cellColor =
   {
     mono,  ///< Mono (default)
     black, ///< Black
     red,   ///< Red
     green, ///< Green
     yellow,  ///< Yellow
     blue,  ///< Blue
     magenta, ///< Magenta
     cyan,  ///< Cyan
     white  ///< White
   };

    /**
     * Gets the color.
     *
     * @return the color
     */
    public Color getColor()
    {
        return _color;
    }

    static CellColor getCellColor(int v)
    {
        if (v <= 7)
            return cellColor[v + 1];
        return cellColor[0];
    }
}

