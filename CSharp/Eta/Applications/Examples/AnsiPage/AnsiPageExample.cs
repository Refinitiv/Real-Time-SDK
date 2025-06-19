/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.IO;

using LSEG.Eta.AnsiPage;

namespace LSEG.Eta.Example.AnsiPageExample;

/// <summary>
/// This example demonstrates using the AnsiPage encoder and decoder in an application.
/// </summary>
///
/// <remarks>
/// <h2>Summary</h2>
/// <p>
/// This class demonstrates how the ANSIPage API can be used to do the following:
/// </p>
/// <ul>
///  <li>Encode ANSI strings into Page objects
///  <li>Decode ANSI strings from Page objects
///  <li>Encode and decode PageUpdate lists
/// </ul>
///
/// <h2>Setup Environment</h2>
/// <p>
/// No special setup is required.
///
/// <h2>Running the application:</h2>
/// <p>
/// Change directory to the <i>source</i> directory and issue the following <c>dotnet</c> command.
/// <p>
/// <code>
/// dotnet run
/// </code>
/// </remarks>
public class AnsiPageExample
{
    // Create a page
    // By default it contains 25 rows and 80 columns
    Page page = new Page();

    // This array will be used to hold a list of updates
    // PageUpdate[] updateList = new PageUpdate[MAX_UPDATES];
    List<PageUpdate> updateList = new List<PageUpdate>();

    // Initialize page using the ANSI "ESC c" sequence
    readonly static byte[] strInit = { 0x1B, (byte)'c' };
    Stream bais = new MemoryStream(strInit);

    /// <summary>
    /// Instantiates a new Ansi Page example.
    /// </summary>
    public AnsiPageExample()
    {
    }

    /// <summary>
    /// Decodes an empty page and print the contents in standard out.
    /// </summary>
    public void DecodeInitialPage()
    {
        // At this point, the page is empty.
        page.Decode(bais, updateList);
        Console.WriteLine(page.ToString(true));
    }

    /// <summary>
    /// Sets some page cells.
    /// </summary>
    public void SetContents()
    {
        // Create a model cell that will be used to modify specific cells of the page
        // image.
        //
        // We'll also keep track of each modification to the page in an update list so we
        // can create an ANSI encoded update string.
        PageCell modelCell = new PageCell(' ', // The character itself
                                          GraphicSet.USAscii,   // Graphic Set being used
                                          CellStyle.Plain,      // Cell style attribute
                                          CellStyle.Underline,  // Cell fade style attribute
                                          CellColor.Green,      // Cell foreground color
                                          CellColor.Black,      // Cell background color
                                          CellColor.Black,      // Cell foreground fade color
                                          CellColor.Red);       // Cell background fade color

        // Update page with three updates and fill in the update list.
        //
        // This is update 1 ( at element [0] )
        modelCell.SetChar('H');
        page.SetPageCell((short)2, (short)7, modelCell);
        modelCell.SetChar('E');
        page.SetPageCell((short)2, (short)8, modelCell);
        modelCell.SetChar('L');
        page.SetPageCell((short)2, (short)9, modelCell);
        modelCell.SetChar('L');
        page.SetPageCell((short)2, (short)10, modelCell);
        modelCell.SetChar('O');
        page.SetPageCell((short)2, (short)11, modelCell);
        PageUpdate tempUpdate = new PageUpdate();
        tempUpdate.SetRow((short)2);
        tempUpdate.SetBeginningColumn((short)7);
        tempUpdate.SetEndingColumn((short)12);
        updateList.Add(tempUpdate);

        // This is update 2 ( at element [1] )
        modelCell.SetChar('M');
        page.SetPageCell((short)3, (short)13, modelCell);
        modelCell.SetChar('y');
        page.SetPageCell((short)3, (short)14, modelCell);
        tempUpdate = new PageUpdate();
        tempUpdate.SetRow((short)3);
        tempUpdate.SetBeginningColumn((short)13);
        tempUpdate.SetEndingColumn((short)15);
        updateList.Add(tempUpdate);

        // This is update 3 ( at element [2] )
        //
        // (notice that friend is spelled wrong...  we'll fix that later using an ANSI
        // encoded update string)
        modelCell.SetChar('F');
        page.SetPageCell((short)4, (short)16, modelCell);
        modelCell.SetChar('r');
        page.SetPageCell((short)4, (short)17, modelCell);
        modelCell.SetChar('e');
        page.SetPageCell((short)4, (short)18, modelCell);
        modelCell.SetChar('i');
        page.SetPageCell((short)4, (short)19, modelCell);
        modelCell.SetChar('n');
        page.SetPageCell((short)4, (short)20, modelCell);
        modelCell.SetChar('d');
        page.SetPageCell((short)4, (short)21, modelCell);
        tempUpdate = new PageUpdate();
        tempUpdate.SetRow((short)4);
        tempUpdate.SetBeginningColumn((short)16);
        tempUpdate.SetEndingColumn((short)22);
        updateList.Add(tempUpdate);

        // Display the page to show modifications
        Console.WriteLine(page.ToString(true));
    }

    /// <summary>
    /// Updates the page with update list.
    /// </summary>
    public void UpdatePage()
    {
        // Now, let's create an ANSI encoded update string based upon the current contents
        // of the page image and the updates currently in the update list.
        byte[] strUpdateBuf = new byte[1024];
        Stream inStream = new MemoryStream(strUpdateBuf);
        Stream outStream = new MemoryStream();
        page.Encode(true, updateList, outStream);

        // Normally, this update string would be sent from a source application to another
        // application so that it may sync it's copy of the page image with the source's
        // copy.  For simplicity, we'll create a new page within this same application.

        // Let's use this update string to update a new page image and update list.  (this
        // time we'll create a page on the heap, for illustration purposes)
        Page destinationPage = new Page();
        List<PageUpdate> destinationUpdateList = new List<PageUpdate>();
        destinationPage.Decode(inStream, destinationUpdateList);

        // Display the page to show that the updates have been successfully decoded into
        // the new page image.
        Console.WriteLine(destinationPage.ToString(true));

        // Use an ANSI encoded string to update the typo from "ei" to "ie" in the word
        // "friend" located in the original page
        byte[] strUpdate = {0x1B, (byte)'[', (byte)'4', (byte)';', (byte)'1',
            (byte)'8', (byte)'H', (byte)'i', (byte)'e' };
        Stream strUpdateStream = new MemoryStream(strUpdate);
        page.Decode(strUpdateStream, updateList);

        // At this point, the page has been updated based upon what was in the update
        // string
        Console.WriteLine(page.ToString(true));
    }

    /// <summary>
    /// Sets the page to empty state.
    /// </summary>
    public void ResetPage()
    {
        // Now that we are done with the page let's reset the contents
        page.Reset(updateList);

        // At this point, the page has been reset back to the default (empty) state
        Console.WriteLine(page.ToString(true));
    }

    /// <summary>
    /// Main driver of the AnsiPage example program.
    /// </summary>
    ///
    /// <param name="args">the arguments</param>
    public static void Main(string[] _)
    {
        AnsiPageExample ansiExample = new AnsiPageExample();
        ansiExample.DecodeInitialPage();
        ansiExample.SetContents();
        ansiExample.UpdatePage();
        ansiExample.ResetPage();
    }
}
