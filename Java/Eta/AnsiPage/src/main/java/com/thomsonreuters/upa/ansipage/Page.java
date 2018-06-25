package com.thomsonreuters.upa.ansipage;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.Vector;

import com.thomsonreuters.ansi.CharType;
import com.thomsonreuters.ansi.ListType;
import com.thomsonreuters.ansi.PageType;

/**
 * The Page interface is the heart of the AnsiPage API.  This interface encapsulates a list of cells defined by the PageCell
 * interface into one entity, forming the page image.  The Page interface also provides the ability to generate and interpret
 * ANSI encoded update streams, as well as the ability to reset the Page and a list of PageUpdate objects back to their default state.
 * The encoding process analyzes the list of updates and the corresponding page cells to produce an ANSI update stream.  This stream may
 * then, for example, be sent to a different application for decoding.
 * <p>
 * The decoding process parses the contents of an ANSI encoded update stream into the list of updates and the corresponding page cells.
 */
public class Page implements Cloneable
{
  private short _nNumberOfRows = 0;
  private short _nNumberOfColumns = 0;
  private ListType _updateList = null;
  PageType _page = null;

  /**
   * The Page() constructor is used to create a Page object.
   * 
   * @param nNumberOfRows Number of rows that the Page object will contain.  Type: short   Default: 25
   * @param nNumberOfColumns Number of columns that the Page object will contain.  Type: short   Default: 80
   */
  public Page(short nNumberOfRows, short nNumberOfColumns)
  {
    //If constructor is passed negative values (or zero), use default values
   _nNumberOfRows = ( nNumberOfRows <= 0 ) ? 25 : nNumberOfRows;
   _nNumberOfColumns = ( nNumberOfColumns <= 0 ) ? 80 : nNumberOfColumns;
   _updateList = new ListType();
   _page = new PageType(nNumberOfRows, nNumberOfColumns);
   _page.status.initStatus();
  }

  /**
   * Makes a copy of this Page. Changes to the copy will not affect the original and vice versa.
   *
   * @return the object
   */
  public Object clone()
  {
    Page page = new Page(_nNumberOfRows,_nNumberOfColumns);
    page._page = (PageType)_page.clone();
    page._updateList = (ListType)_updateList.clone();
    return page;
  }

  /**
   * <P>The decode() method analyzes and parses the ANSI encoded update stream
   * and updates the PageUpdate list as well as the corresponding PageCell objects
   * of the Page image.  
   * 
   * <p>Decoding an ANSI stream is accomplished by using a finite
   * state machine.  Each of the ANSI escape sequences is parsed according to the state
   * into which the previous characters have the machine.  As the sequences are recognized,
   * the page image is modified. This allows the update list to be
   * sized without knowledge of the maximum amount of update information that a source
   * can communicate.  The page image should not be altered between calls to the decode()
   * method.
   * 
   * <ul><li>As decode() is called, entries are added to the update list for the following
   *  reasons:
   * <li>For each repositioning of the cursor an entry with zero length is generated for that point.
   * Carriage Return plus Line Feed is treated as a single sequence.
   * <li>Each character written increases the length of the current entry by one.
   * If it wraps around at the end of a line, it gets treated as cursor movement and a new
   * entry is added as above.
   * <li>Scrolling generates an entry for each row scrolled, and one for the new cursor position
   * following the scroll.
   * </ul>
   * 
   * @param instream ByteArrayInputStream that contains the ANSI encoded update stream to be decoded.
   * @param pageUpdateList Vector of {@link PageUpdate} objects.  This list will be updated by the decode() method
   * based upon the contents of the ANSI encoded update string.
   * 
   * @return the actual number of characters that were successfully parsed.
   */
  public long decode (ByteArrayInputStream instream, Vector<PageUpdate> pageUpdateList)
  {
    if ( _updateList != null)
    {
      _updateList = null;
    }
    _updateList = new ListType();
    long nNumCharsParsed = _page.getDecoder().qa_decode(_page, instream, _updateList);
    pageUpdateList.clear();
    for ( int i = 0; i <_updateList.index + 1; i++)
    {
      PageUpdate pageUpdate = new PageUpdate();
      pageUpdate.setRow(_updateList.upd_list[i].row);
      pageUpdate.setBeginningColumn( _updateList.upd_list[i].upd_beg );
      pageUpdate.setEndingColumn( _updateList.upd_list[i].upd_end );
      pageUpdateList.add(pageUpdate);
    }
    return nNumCharsParsed;
  }

  /**
   * The encode() method analyzes the list of provided PageUpdate objects and the
   * corresponding PageCell objects in the Page image and produces an ANSI encoded update stream.
   * 
   * @param bFadeEnable Flag used to enable or disable the use of fading.  Type: boolean
   * @param pageUpdate Vector of {@link PageUpdate} objects.  This array will be analyzed by the
   * encode() method to produce the ANSI encoded update stream.
   * @param outstream ByteArrayOutputStream that will be filled in by the encode() method.
   * This ByteArrayOutputStream will contain the ANSI encoded update string when the
   * encoding process is complete.
   * 
   * @return true if all of the updates defined by the update list pointed to by
   * pageUpdate have been successfully encoded. Returns false, if there is any errors in encoding.
   */
  public boolean encode(boolean bFadeEnable, Vector<PageUpdate> pageUpdate, ByteArrayOutputStream outstream)
  {
    if (_updateList != null)
    {
      _updateList = null;
    }
    _updateList = new ListType();
    _updateList.index = (short)(pageUpdate.size()-1 );
    for (int i = 0; i<pageUpdate.size(); i++)
    {
      PageUpdate update = pageUpdate.elementAt(i) ;
      _updateList.upd_list[i].row = update.getRow();
      _updateList.upd_list[i].upd_beg = update.getBeginningColumn();
      _updateList.upd_list[i].upd_end = update.getEndingColumn();
    }

    long nRetVal = _page.getEncoder().qa_encode( _page, outstream,
                                 bFadeEnable, _updateList, _nNumberOfRows, _nNumberOfColumns );
    if (nRetVal == 0)
    {
      //Since the encoding of the current update has completed,
      //reset last_mod so that the next time encode() is called
      //for this page, it will know it is for a brand new update.
      _page.last_mod = 0;
      return true;
    }
    
    return false;
  }

  /**
   * The toString is used to output the content of each PageCell to a String object.
   *
   * @return String object that contains the character is each PageCell
   */
  public String toString()
  {
    StringBuilder buf = new StringBuilder(_nNumberOfRows*_nNumberOfColumns);
    for (short j= 1; j <= _nNumberOfRows; j++)
    {
      for (short k = 1; k <= _nNumberOfColumns; k++ )
      {
        buf.append(getChar( j, k ));
      }
      buf.append('\n');
    }
    return buf.toString();
  }

  /**
   * The toString is used to output the content of each PageCell to a String object.
   * If the drawBorder is set, the content will have a border.
   *
   * @param drawBorder the draw border
   * @return String object that contains the character is each PageCell
   */
  public String toString(boolean drawBorder)
  {
    StringBuilder buf = new StringBuilder(_nNumberOfRows*_nNumberOfColumns);
    if (drawBorder)
    {
      for (int i = 1; i <= _nNumberOfColumns; i++)
        buf.append('-');
    }
    buf.append('\n');
    for (short j= 1; j <= _nNumberOfRows; j++)
    {
      if (drawBorder)
      {
        buf.append('|');
      }
      for (short k = 1; k <= _nNumberOfColumns; k++ )
      {
          buf.append(getChar( j, k ));
      }
      if (drawBorder)
      {
        buf.append('|');
      }
      buf.append('\n');
    }
    if (drawBorder)
    {
      for (int i = 1; i <= _nNumberOfColumns; i++)
        buf.append('-');
    }
    buf.append('\n');
    return buf.toString();
  }

  /**
   * Used to determine the number of columns contained in the Page object.
   * 
   * @return returns the number of columns specified when the Page object was created.
   * If no default value was specified the default value is returned
   */
  public short getNumberOfColumns()
  {
    return _nNumberOfColumns;
  }

  /**
   * Used to determine the number of rows contained in the Page object.
   * 
   * @return the number of rows specified when the Page object was created.
   * If no default value was specified the default value is returned.
   */
  public short getNumberOfRows()
  {
    return _nNumberOfRows;
  }

  /**
   * Used to determine the contents of a particular cell within the Page object.
   * 
   * @param nRow Row of the desired cell to return.
   * @param nColumn Column of the desired cell to return.
   * 
   * @return a copy of the PageCell object at the specified row and column of the Page image.
   * If the row and column are out of range, an immutable "null" PageCell is returned.
   * 
   * @deprecated - returns a new object with each call.  The other "<code>get</code>" functions are more efficient.
   */
  public PageCell getPageCell(short nRow, short nColumn)
  {
    //Check to make sure that the requested cell is within the page bounds before returning the cell.
    if( outOfBounds(nRow, nColumn) )
      return NullCell.Singleton;
     
     //Return the element of the page image that is at the offset determined by the row and column requested
     return new PageCell(getCharType(nRow, nColumn)) ; 
  }

  /**
   * The contents of a particular cell.
   * 
   * @param nRow Row of the desired char
   * @param nColumn Column of the desired char
   * 
   * @return char of a particular cell within the page.
   * If the row and column are out of range, '\0' is returned.
   */
  public char getChar(short nRow, short nColumn)
  {
      if( outOfBounds(nRow, nColumn) )
          return '\0';

      return getCharType(nRow, nColumn).ch;
  }

  /**
   * The CellColor of a particular cell's background.
   * 
   * @param nRow Row of the desired background color
   * @param nColumn Column of the desired background color
   * 
   * @return background color of a particular cell within the page.
   * If the row and column are out of range, {@link CellColor#mono} is returned.
   */
  public CellColor getBackgroundColor(short nRow, short nColumn)
  {
      if( outOfBounds(nRow, nColumn) )
          return CellColor.mono;
      CharType cell = getCharType(nRow, nColumn);
      return CellColor.getCellColor(cell.c_attr >> 4);
  }

  /**
   * The fade CellColor of a particular cell's background.
   * 
   * @param nRow Row of the desired background fade color
   * @param nColumn Column of the desired background fade color
   * 
   * @return background fade color of a particular cell within the page.
   * If the row and column are out of range, {@link CellColor#mono} is returned.
   */
  public CellColor getBackgroundFadeColor(short nRow, short nColumn)
  {
      if( outOfBounds(nRow, nColumn) )
          return CellColor.mono;
      CharType cell = getCharType(nRow, nColumn);
      return CellColor.getCellColor(cell.c_fade_attr >> 4);
  }

  /**
   * Checks for style.
   *
   * @param nRow Row of the desired CellStyle
   * @param nColumn Column of the desired CellStyle
   * @param style style to look for in page
   * @return true if any of the styles in <code>style</code> is enabled for the given cell
   */
  public boolean hasStyle(short nRow, short nColumn, CellStyle style)
  {
      if( outOfBounds(nRow, nColumn) )
          return false;
      CharType cell = getCharType(nRow, nColumn);
      return (cell.attr & style._value) != 0;
  }


  /**
   * Checks for fade style.
   *
   * @param nRow Row of the desired fade style
   * @param nColumn Column of the desired style CellStyle
   * @param style style to look for in page
   * @return true if any of the styles in <code>style</code> is enabled for the given cell
   */
  public boolean hasFadeStyle(short nRow, short nColumn, CellStyle style)
  {
      if( outOfBounds(nRow, nColumn) )
          return false;
      CharType cell = getCharType(nRow, nColumn);
      return (cell.fade_attr & style._value) != 0;
  }

  /**
   * The CellColor of a particular cell's foreground.
   * 
   * @param nRow Row of the desired foreground color
   * @param nColumn Column of the desired foreground color
   * 
   * @return foreground color of a particular cell within the page.
   * If the row and column are out of range, {@link CellColor#mono} is returned.
   */
  public CellColor getForegroundColor(short nRow, short nColumn)
  {
      if( outOfBounds(nRow, nColumn) )
          return CellColor.mono;
      CharType cell = getCharType(nRow, nColumn);
      return CellColor.getCellColor(cell.c_attr& 0x0f );
  }

  /**
   * The fade CellColor of a particular cell's foreground.
   * 
   * @param nRow Row of the desired foreground fade color
   * @param nColumn Column of the desired foreground fade color
   * 
   * @return foreground fade color of a particular cell within the page.
   * If the row and column are out of range, {@link CellColor#mono} is returned.
   */
  public CellColor getForegroundFadeColor(short nRow, short nColumn)
  {
      if( outOfBounds(nRow, nColumn) )
          return CellColor.mono;
      CharType cell = getCharType(nRow, nColumn);
      return CellColor.getCellColor(cell.c_fade_attr & 0x0f);
  }

  /**
   * The GraphicSet of a particular cell within the Page object.
   * 
   * @param nRow Row of the desired GraphicSet
   * @param nColumn Column of the desired GraphicSet
   * 
   * @return GraphicSet of a particular cell within the page.
   * If the row and column are out of range, {@link GraphicSet#Unknown} is returned.
   */
  public GraphicSet getGraphicSet(short nRow, short nColumn)
  {
      if( outOfBounds(nRow, nColumn) )
          return GraphicSet.Unknown;
      CharType cell = getCharType(nRow, nColumn);
      return GraphicSet.getGraphicSet(cell.gs);
  }

  /**
   * Used to reset the Page object and the pageUpdateList back to their default state.
   * 
   * @param pageUpdateList the list of desired cell to modify.  Type: Vector of PageUpdate
   */
  public void reset(Vector<PageUpdate> pageUpdateList)
  {
    if (_updateList != null)
      _updateList = null;
    _updateList = new ListType();
    _updateList.index = 0;
    _page.getDecoder().qa_reset(_page, _updateList);
    for (int i = 0; i < pageUpdateList.size(); i++)
    {
      PageUpdate update = pageUpdateList.elementAt(i);
      update.setRow((short)0);
      update.setBeginningColumn((short)0);
      update.setEndingColumn((short)0);
    }
  }

  /**
   * Used to modify the contents of a particular cell within the Page object.
   * 
   * @param nRow Row of the desired cell to modify.  Type: short
   * @param nColumn Column of the desired cell to modify.  Type: short
   * @param rPagecell constant PageCell object that will be copied into the desired cell of the Page object.
   * Type: const PageCell
   */
  public void setPageCell(short nRow, short nColumn, final PageCell rPagecell)
  {
    //Check to make sure that the specified cell is within the page bounds before updating the cell.
    if( (nRow > 0 && nRow <= _nNumberOfRows) &&
        (nColumn > 0 && nColumn <= _nNumberOfColumns) )
    {
      //Assign the element to the offset of the page image determined by the row and column specified
      _page.page[((((nRow) - 1) * _nNumberOfColumns) + ((nColumn) - 1)) ] = (CharType)rPagecell.toCharType().clone();
    }
  }

  /**
   * Default constructor that constructs a 25 row by 80 column page object.
   *
   */
  public Page()
  {
    _nNumberOfRows = 25;
    _nNumberOfColumns = 80;
    _updateList = new ListType();
    _page = new PageType((short)25, (short)80);
    _page.status.initStatus();
  }

  /**
   * Gets the char type.
   *
   * @param nRow the n row
   * @param nColumn the n column
   * @return the char type
   */
  private CharType getCharType(short nRow, short nColumn)
  {
      return _page.page[((((nRow) - 1) * _nNumberOfColumns) + ((nColumn) - 1))];
  }

  /**
   * Out of bounds.
   *
   * @param nRow the n row
   * @param nColumn the n column
   * @return true, if successful
   */
  private boolean outOfBounds(short nRow, short nColumn)
  {
      return nRow > _nNumberOfRows ||
          nColumn > _nNumberOfColumns ||
          nRow <= 0 ||
          nColumn <= 0;
  }
}

