// Copyright (c) 2001-2003 Quadralay Corporation.  All rights reserved.
//

function  WWHBookGroups_Books(ParamTop)
{


  ParamTop.fAddDirectory("Documentation_Portal", null, null, null, null);
  ParamTop.fAddDirectory("Concepts_Guide", null, null, null, null);
  ParamTop.fAddDirectory("EMA_Developers_Guide", null, null, null, null);
  ParamTop.fAddDirectory("EMA_RDM_Usage_Guide", null, null, null, null);
  ParamTop.fAddDirectory("EMA_Reference_Manual", null, null, null, null);
  ParamTop.fAddDirectory("EMA_Configuration_Guide", null, null, null, null);
  ParamTop.fAddDirectory("EMA_Training_Examples", null, null, null, null);
  ParamTop.fAddDirectory("Using_this_Library", null, null, null, null);
}

function  WWHBookGroups_ShowBooks()
{
  return true;
}

function  WWHBookGroups_ExpandAllAtTop()
{
  return false;
}
