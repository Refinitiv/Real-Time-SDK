/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.ansipage;


import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.Vector;

import com.refinitiv.eta.ansipage.CellColor;
import com.refinitiv.eta.ansipage.CellStyle;
import com.refinitiv.eta.ansipage.GraphicSet;
import com.refinitiv.eta.ansipage.Page;
import com.refinitiv.eta.ansipage.PageCell;
import com.refinitiv.eta.ansipage.PageUpdate;

/**
 * This example demonstrates using the AnsiPage encoder and decoder in an application.
 * <p>
 * <em>Summary</em>
 * <p>
 * This class demonstrates how the ANSIPage API can be used to do the following:
 * </p>
 * <ul>
 *  <li>Encode ANSI strings into Page objects 
 *  <li>Decode ANSI strings from Page objects
 *  <li>Encode and decode PageUpdate lists
 * </ul>
 * <p>
 * <em>Setup Environment</em>
 * <p>
 * No special setup is required.
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runAnsiPageExample<br>
 * Windows: gradlew.bat runAnsiPageExample
 */

public class AnsiPageExample
{
  //Create a page on the stack
  //By default it will contain 25 rows and 80 columns
  Page page = new Page();
  final int MAX_UPDATES = 1000;
  //This array will be used to hold a list of updates
  //PageUpdate[] updateList = new PageUpdate[MAX_UPDATES];
  Vector<PageUpdate> updateList = new Vector<PageUpdate>();
  //Initialize page using the ANSI "ESC c" sequence
  byte[] strInit = {'\033', 'c'};
  ByteArrayInputStream bais = new ByteArrayInputStream(strInit);
  int numUpdates = 0;

  /**
   * Instantiates a new ansi page example.
   */
  public AnsiPageExample()
  {
  }

  /**
   * decodeInitialPage decodes an empty page and print the
   * contents in standard out.
   */
  public void decodeInitialPage()
  {
    //At this point, the page is empty.
    page.decode(bais,  updateList);//, MAX_UPDATES, numUpdates);
    System.out.print(page.toString(true));
  }


  /**
   * setContents sets some page cells.
   */
  public void setContents()
  {
    //Create a model cell that will be used to modify
    //specific cells of the page image.
    //
    //We'll also keep track of each modification to the page
    //in an update list so we can create an ANSI encoded
    //update string.
    PageCell modelCell = new PageCell(' ', //The character itself
                                      GraphicSet.USAscii,    //Graphic Set being used
                                      CellStyle.plain,      //Cell style attribute
                                      CellStyle.underline,  //Cell fade style attribute
                                      CellColor.green,      //Cell foreground color
                                      CellColor.black,      //Cell background color
                                      CellColor.black,      //Cell foreground fade color
                                      CellColor.red );      //Cell background fade color

    //Update page with three updates
    //and fill in the update list.
    //This is update 1 ( at element [0] )
    modelCell.setChar( 'H' );
    page.setPageCell( (short)2,  (short)7, modelCell );
    modelCell.setChar( 'E' );
    page.setPageCell( (short)2,  (short)8, modelCell );
    modelCell.setChar( 'L' );
    page.setPageCell( (short)2,  (short)9,modelCell );
    modelCell.setChar( 'L' );
    page.setPageCell( (short)2, (short)10, modelCell );
    modelCell.setChar( 'O' );
    page.setPageCell( (short)2, (short)11, modelCell );
    PageUpdate tempUpdate = new PageUpdate();
    tempUpdate.setRow( (short)2 );
    tempUpdate.setBeginningColumn( (short)7 );
    tempUpdate.setEndingColumn( (short)12 );
    updateList.add(0, tempUpdate);

    //This is update 2 ( at element [1] )
    modelCell.setChar( 'M' );
    page.setPageCell( (short)3, (short)13, modelCell );
    modelCell.setChar( 'y' );
    page.setPageCell( (short)3, (short)14, modelCell );
    tempUpdate = new PageUpdate();
    tempUpdate.setRow( (short)3 );
    tempUpdate.setBeginningColumn( (short)13 );
    tempUpdate.setEndingColumn( (short)15 );
    updateList.add(1, tempUpdate);

    //This is update 3 ( at element [2] )
    //(notice that friend is spelled wrong...
    //we'll fix that later using an ANSI encoded
    //update string)
    modelCell.setChar( 'F' );
    page.setPageCell( (short)4, (short)16, modelCell );
    modelCell.setChar( 'r' );
    page.setPageCell( (short)4, (short)17, modelCell );
    modelCell.setChar( 'e' );
    page.setPageCell( (short)4, (short)18, modelCell );
    modelCell.setChar( 'i' );
    page.setPageCell( (short)4, (short)19, modelCell );
    modelCell.setChar( 'n' );
    page.setPageCell( (short)4, (short)20, modelCell );
    modelCell.setChar( 'd' );
    page.setPageCell( (short)4, (short)21, modelCell );
    tempUpdate = new PageUpdate();
    tempUpdate.setRow( (short)4 );
    tempUpdate.setBeginningColumn( (short)16 );
    tempUpdate.setEndingColumn( (short)22 );
    updateList.add(2, tempUpdate);
    //Display the page to show modifications
    System.out.print(page.toString(true));
  }

  /**
   * updatePage updates the page with update list.
   */
  public void updatePage()
  {
    //Now, let's create an ANSI encoded update string
    //based upon the current contents of the page image
    //and the updates currently in the update list.
    byte[] strUpdateBuf = new byte[1024];
    ByteArrayInputStream is = new ByteArrayInputStream(strUpdateBuf);
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    page.encode( true, updateList, os );
    //Normally, this update string would be sent from
    //a source application to another application
    //so that it may sync it's copy of the page image with
    //the source's copy.  For simplicity, we'll create a
    //new page within this same application.

    //Let's use this update string to update
    //a new page image and update list.
    //(this time we'll create a page on the heap,
    //for illustration purposes)
    Page destinationPage = new Page();
    Vector<PageUpdate> destinationUpdateList = new Vector<PageUpdate>();
    destinationPage.decode(is, destinationUpdateList);

    //Display the page to show that the updates
    //have been successfully decoded into the
    //new page image.
    System.out.println(destinationPage.toString(true));
    //Use an ANSI encoded string to update the
    //typo from "ei" to "ie" in the word "friend"
    //located in the original page
    byte[] strUpdate = {'\033', '[', '4', ';',
      '1', '8', 'H', 'i', 'e'};
    ByteArrayInputStream strUpdateStream = new ByteArrayInputStream(strUpdate);
    page.decode(strUpdateStream, updateList);
    //At this point, the page has been updated
    //based upon what was in the update string
    System.out.println(page.toString(true));
  }

  /**
   * resetPage sets the page to empty state.
   */
  public void resetPage()
  {
    //Now that we are done with the page
    //let's reset the contents
    page.reset( updateList);

    //At this point, the page has been reset
    //back to the default (empty) state
    System.out.println(page.toString(true));
  }

  /**
   * main driver of the AnsiPage example program.
   *
   * @param args the arguments
   */
  public static void main(String[] args)
  {
    AnsiPageExample ansiExample = new AnsiPageExample();
    ansiExample.decodeInitialPage();
    ansiExample.setContents();
    ansiExample.updatePage();
    ansiExample.resetPage();
  }
}

