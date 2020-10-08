package com.refinitiv.eta.ansipage;


/**
 * The PageUpdate interface provide the ability to track updates that are made to the PageCell of an ANSI page.
 * This, among other things, facilitates the capability for a source application
 * to publish only the portions of the page image that have been modified. Each PageUpdate consists of three numeric values.
 * The first designates the row on which the update has occurred, and the remaining two identify the beginning and ending columns
 * of the update.  More specifically, the column of the first character in the update marks the beginning column, while the ending
 * column is marked as one column beyond the last character in the update.
 * <p>
 * For example, an update beginning on column 9 and continuing through column 13 would have a value of 9 for the beginning column and a
 * value of 14 for the ending column.  Subtracting the ending column value from the beginning column value yields the total number of characters
 * in the update; therefore, this update would be 5 characters long.
 *
 */

public final class PageUpdate implements Cloneable
{
  private short _nRow;
  private short _nBeginningColumn;
  private short _nEndingColumn;

  /**
   * Makes a copy of this PageUpdate. Changes to the copy will not affect the original and vice versa.
   * 
   * @return copy of this PageUpdate.
   */
  public Object clone()
  {
    Object pageUpdate = null;
    try
    {
      pageUpdate = super.clone();
    }
    catch (CloneNotSupportedException e)
    {
      e.printStackTrace(System.err);
    }
    return pageUpdate;
  }

  /**
   * PageUpdate() constructor is used to create a PageUpdate object
   * 
   * @param nRow Row that the PageUpdate object will be initialized to.  Type: short   Default: 0
   * @param nBeginningColumn Beginning column to initialize the PageUpdate object.  Type: short   Default: 0
   * @param nEndingColumn Ending column to initialize the PageUpdate object.  Type: short   Default: 0
   */
  public PageUpdate(short nRow, short nBeginningColumn, short nEndingColumn)
  {
    _nRow = nRow;
    _nBeginningColumn = nBeginningColumn;
    _nEndingColumn = nEndingColumn;
  }

  /**
   * Default constructor
   */
  public PageUpdate()
  {
    _nRow = 0;
    _nBeginningColumn = 0;
    _nEndingColumn = 0;
  }

  /**
   * Used to determine the column where the update begins.
   * 
   * @return the column where the update begins.
   */
  public short getBeginningColumn()
  {
    return _nBeginningColumn;
  }

  /**
   * Used to determine the column where the update ends.
   * 
   * @return the column which is one beyond where the update ends.
   */
  public short getEndingColumn()
  {
    return _nEndingColumn;
  }

  /**
   * Used to determine the row.
   * 
   * @return the row of the PageUpdate object.
   */
  public short getRow ()
  {
    return _nRow;
  }

  /**
   * Used to modify the column on which the update begins.
   * 
   * @param nBeginningColumn Column the update begins on.  Type: short
   */
  public void setBeginningColumn(short nBeginningColumn)
  {
    _nBeginningColumn = nBeginningColumn;
  }

  /**
   * Used to modify the column on which the update ends.
   * 
   * @param nEndingColumn One column beyond where the update ends on.  Type: short
   */
  public void setEndingColumn(short nEndingColumn)
  {
    _nEndingColumn = nEndingColumn;
  }

  /**
   * Used to modify the row on which the update appears.
   * 
   * @param nRow Row the update appears on.  Type: short
   */
  public void setRow (short nRow)
  {
    _nRow = nRow;
  }
}

