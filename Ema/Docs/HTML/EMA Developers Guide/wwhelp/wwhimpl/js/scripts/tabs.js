// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHStringUtilities_EscapeHTML */
/*global WWHStringUtilities_FormatMessage */
/*global WWHTabs_BodyHTML */
/*global WWHTabs_HeadHTML */
/*global WWHTabs_Loaded */
/*global WWHTabs_Reload */

function  WWHTabs_Object(ParamPanels)
{
  this.mWidth = null;

  this.fReload   = WWHTabs_Reload;
  this.fHeadHTML = WWHTabs_HeadHTML;
  this.fBodyHTML = WWHTabs_BodyHTML;
  this.fLoaded   = WWHTabs_Loaded;

  // Calculate width based on number of panels
  //
  if (ParamPanels > 0)
  {
    this.mWidth = String(100 / ParamPanels) + "%";
  }
}

function  WWHTabs_Reload()
{
  WWHFrame.WWHHelp.fReplaceLocation("WWHTabsFrame", WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/html/tabs.htm");
}

function  WWHTabs_HeadHTML()
{
  var  StylesHTML = "";


  // Generate style section
  //
  StylesHTML += "<style type=\"text/css\">\n";
  StylesHTML += " <!--\n";
  StylesHTML += "  a.active\n";
  StylesHTML += "  {\n";
  StylesHTML += "    text-decoration: none;\n";
  StylesHTML += "    color: " + WWHFrame.WWHJavaScript.mSettings.mTabs.mSelectedTabForegroundColor + ";\n";
  StylesHTML += "    " + WWHFrame.WWHJavaScript.mSettings.mTabs.mFontStyle + ";\n";
  StylesHTML += "  }\n";
  StylesHTML += "  a.inactive\n";
  StylesHTML += "  {\n";
  StylesHTML += "    text-decoration: none;\n";
  StylesHTML += "    color: " + WWHFrame.WWHJavaScript.mSettings.mTabs.mDefaultTabForegroundColor + ";\n";
  StylesHTML += "    " + WWHFrame.WWHJavaScript.mSettings.mTabs.mFontStyle + ";\n";
  StylesHTML += "  }\n";
  StylesHTML += "  th\n";
  StylesHTML += "  {\n";
  StylesHTML += "    color: " + WWHFrame.WWHJavaScript.mSettings.mTabs.mSelectedTabForegroundColor + ";\n";
  StylesHTML += "    " + WWHFrame.WWHJavaScript.mSettings.mTabs.mFontStyle + ";\n";
  StylesHTML += "  }\n";
  StylesHTML += "  td\n";
  StylesHTML += "  {\n";
  StylesHTML += "    color: " + WWHFrame.WWHJavaScript.mSettings.mTabs.mDefaultTabForegroundColor + ";\n";
  StylesHTML += "    " + WWHFrame.WWHJavaScript.mSettings.mTabs.mFontStyle + ";\n";
  StylesHTML += "  }\n";
  StylesHTML += " -->\n";
  StylesHTML += "</style>\n";

  return StylesHTML;
}

function  WWHTabs_BodyHTML()
{
  var  TabsHTML = "";
  var  ImageDir = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/images";
  var  MaxIndex;
  var  Index;
  var  VarTabTitle;
  var  VarAccessibilityTitle = "";
  var  CellType;
  var  WrapPrefix;
  var  WrapSuffix;
  var  StyleAttribute;
  var  OnClick;
  var  ImgSuffix;
  var  TableWidth;


  // Set style attribute to insure small image height
  //
  StyleAttribute = " style=\"font-size: 1px; line-height: 1px;\"";

  // Force on one line with a table, except for Netscape 4.x
  //
  if (WWHFrame.WWHBrowser.mBrowser !== 1)  // Shorthand for Netscape
  {
    TabsHTML += "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" width=\"100%\" role=\"presentation\">\n";
    TabsHTML += "<tr>";
  }

  // Tabs
  //
  for (MaxIndex = WWHFrame.WWHJavaScript.mPanels.mPanelEntries.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    // Get tab title
    //
    VarTabTitle = WWHFrame.WWHJavaScript.mPanels.mPanelEntries[Index].mPanelObject.mPanelTabTitle;

    // Display anchor only if not selected
    //
    if (Index === WWHFrame.WWHJavaScript.mCurrentTab)
    {
      // Determine title for accessibility
      //
      if (WWHFrame.WWHHelp.mbAccessible)
      {
        VarAccessibilityTitle = WWHStringUtilities_FormatMessage(WWHFrame.WWHJavaScript.mMessages.mAccessibilityActiveTab,
                                                                 VarTabTitle);
        VarAccessibilityTitle = " title=\"" + WWHStringUtilities_EscapeHTML(VarAccessibilityTitle) + "\"";
      }

      CellType = "th";
      WrapPrefix = "<b><a class=\"active\" name=\"tab" + Index + "\" href=\"javascript:void(0);\"" + VarAccessibilityTitle + ">";
      WrapSuffix = "</a></b>";
      OnClick = "";
      ImgSuffix = "";
    }
    else
    {
      // Determine title for accessibility
      //
      if (WWHFrame.WWHHelp.mbAccessible)
      {
        VarAccessibilityTitle = WWHStringUtilities_FormatMessage(WWHFrame.WWHJavaScript.mMessages.mAccessibilityInactiveTab,
                                                                 VarTabTitle);
        VarAccessibilityTitle = " title=\"" + WWHStringUtilities_EscapeHTML(VarAccessibilityTitle) + "\"";
      }

      CellType = "td";
      WrapPrefix = "<b><a class=\"inactive\" name=\"tab" + Index + "\" href=\"javascript:WWHFrame.WWHJavaScript.fClickedChangeTab(" + Index + ");\"" + VarAccessibilityTitle + ">";
      WrapSuffix = "</a></b>";
      OnClick = " onclick=\"WWHFrame.WWHJavaScript.fClickedChangeTabWithDelay(" + Index + ");\"";
      ImgSuffix = "x";
    }

    // Force on one line with a table, except for Netscape 4.x
    //
    if (WWHFrame.WWHBrowser.mBrowser !== 1)  // Shorthand for Netscape
    {
      TabsHTML += "<td width=\"" + this.mWidth + "\">";
    }

    // Force on one line with a table, except for Netscape 4.x
    //
    if (WWHFrame.WWHBrowser.mBrowser === 1)  // Shorthand for Netscape
    {
      TableWidth = this.mWidth;
    }
    else
    {
      TableWidth = "100%";
    }
    TabsHTML += "<table align=\"left\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" width=\"" + TableWidth + "\" role=\"presentation\">";

    // Top spacer
    //
    TabsHTML += "<tr>";
    TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_top" + ImgSuffix + ".gif\" alt=\"\"></div></td>";
    TabsHTML += "</tr>";

    // Top row
    //
    TabsHTML += "<tr>";
    if (Index === 0)
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabl.gif\" alt=\"\"></div></td>\n";
    }
    else
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabm.gif\" alt=\"\"></div></td>\n";
    }

    TabsHTML += "<td" + OnClick + "><div" + StyleAttribute + "><img src=\"" + ImageDir + "/btn_nw" + ImgSuffix + ".gif\" alt=\"\"></div></td>\n";
    TabsHTML += "<td" + OnClick + " background=\"" + ImageDir + "/btn_n" + ImgSuffix + ".gif\"><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_n.gif\" alt=\"\"></div></td>\n";
    TabsHTML += "<td" + OnClick + "><div" + StyleAttribute + "><img src=\"" + ImageDir + "/btn_ne" + ImgSuffix + ".gif\" alt=\"\"></div></td>\n";

    if ((Index + 1) === MaxIndex)
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabr.gif\" alt=\"\"></div></td>\n";
    }
    TabsHTML += "</tr>";

    // Middle row
    //
    TabsHTML += "<tr>";
    if (Index === 0)
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabl.gif\" alt=\"\"></div></td>\n";
    }
    else
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabm.gif\" alt=\"\"></div></td>\n";
    }

    TabsHTML += "<td" + OnClick + " background=\"" + ImageDir + "/btn_w" + ImgSuffix + ".gif\"><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_w.gif\" alt=\"\"></div></td>\n";
    TabsHTML += "<" + CellType + " background=\"" + ImageDir + "/btn_bg" + ImgSuffix + ".gif\" nowrap align=\"center\" width=\"100%\"" + OnClick + ">";
    TabsHTML += WrapPrefix;
    TabsHTML += VarTabTitle;
    TabsHTML += WrapSuffix;
    TabsHTML += "</" + CellType + ">";
    TabsHTML += "<td" + OnClick + " background=\"" + ImageDir + "/btn_e" + ImgSuffix + ".gif\"><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_e.gif\" alt=\"\"></div></td>\n";

    if ((Index + 1) === MaxIndex)
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabr.gif\" alt=\"\"></div></td>\n";
    }
    TabsHTML += "</tr>";

    // Bottom row
    //
    TabsHTML += "<tr>";
    if (Index === 0)
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabl.gif\" alt=\"\"></div></td>\n";
    }
    else
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabm.gif\" alt=\"\"></div></td>\n";
    }

    TabsHTML += "<td" + OnClick + "><div" + StyleAttribute + "><img src=\"" + ImageDir + "/btn_sw" + ImgSuffix + ".gif\" alt=\"\"></div></td>\n";
    TabsHTML += "<td" + OnClick + " background=\"" + ImageDir + "/btn_s" + ImgSuffix + ".gif\"><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_s" + ImgSuffix + ".gif\" alt=\"\"></div></td>\n";
    TabsHTML += "<td" + OnClick + "><div" + StyleAttribute + "><img src=\"" + ImageDir + "/btn_se" + ImgSuffix + ".gif\" alt=\"\"></div></td>\n";

    if ((Index + 1) === MaxIndex)
    {
      TabsHTML += "<td><div" + StyleAttribute + "><img src=\"" + ImageDir + "/spc_tabr.gif\" alt=\"\"></div></td>\n";
    }
    TabsHTML += "</tr>";

    TabsHTML += "</table>";

    // Force on one line with a table, except for Netscape 4.x
    //
    if (WWHFrame.WWHBrowser.mBrowser !== 1)  // Shorthand for Netscape
    {
      TabsHTML += "</td>\n";
    }
  }

  // Force on one line with a table, except for Netscape 4.x
  //
  if (WWHFrame.WWHBrowser.mBrowser !== 1)  // Shorthand for Netscape
  {
    TabsHTML += "</tr>\n";
    TabsHTML += "</table>\n";
  }

  return TabsHTML;
}

function  WWHTabs_Loaded()
{
  // Set frame name for accessibility
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    WWHFrame.WWHHelp.fSetFrameName("WWHTabsFrame");
  }

  // Display requested panel
  //
  WWHFrame.WWHJavaScript.mPanels.fChangePanel(WWHFrame.WWHJavaScript.mCurrentTab);
}
