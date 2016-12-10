// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint sloppy: true, vars: true, white: true */
/*global WWHFrame */

function  WWHPopupFormat_Translate(ParamText)
{
  return ParamText;
}

function  WWHPopupFormat_Format(ParamWidth,
                                ParamTextID,
                                ParamText)
{
  var  FormattedText   = "";
  var  BackgroundColor = WWHFrame.WWHHelp.mSettings.mPopup.mBackgroundColor;
  var  BorderColor     = WWHFrame.WWHHelp.mSettings.mPopup.mBorderColor;
  var  ImageDir        = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images";
  var  ReqSpacer1w2h   = "<img src=\"" + ImageDir + "/spc1w2h.gif\" width=\"1\" height=\"2\" alt=\"\">";
  var  ReqSpacer2w1h   = "<img src=\"" + ImageDir + "/spc2w1h.gif\" width=\"2\" height=\"1\" alt=\"\">";
  var  StyleAttribute;


  // Set style attribute to insure small image height
  //
  StyleAttribute = " style=\"font-size: 1px; line-height: 1px;\"";

  FormattedText += "<table width=\"4\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" bgcolor=\"" + BackgroundColor + "\">";
  FormattedText += " <tr>";
  FormattedText += "  <td" + StyleAttribute + " height=\"2\" colspan=\"3\" bgcolor=\"" + BorderColor + "\">" + ReqSpacer1w2h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += "  <td width=\"100%\" id=\"" + ParamTextID + "\">" + ParamText + "</td>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td" + StyleAttribute + " height=\"2\" colspan=\"3\" bgcolor=\"" + BorderColor + "\">" + ReqSpacer1w2h + "</td>";
  FormattedText += " </tr>";
  FormattedText += "</table>";

  return FormattedText;
}
