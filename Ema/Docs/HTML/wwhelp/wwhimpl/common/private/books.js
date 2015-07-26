// Copyright (c) 2001-2003 Quadralay Corporation.  All rights reserved.
//

function  WWHBookGroups_Books(ParamTop)
{


  ParamTop.fAddDirectory("Documentation%20Portal", null, null, null, null);
  ParamTop.fAddDirectory("EMA%20Developers%20Guide", null, null, null, null);
  ParamTop.fAddDirectory("EMA%20RDM%20Usage%20Guide", null, null, null, null);
  ParamTop.fAddDirectory("EMA%20Reference%20Manual", null, null, null, null);
  ParamTop.fAddDirectory("EMA%20Configuration%20Guide", null, null, null, null);
  ParamTop.fAddDirectory("EMA%20Training%20Examples", null, null, null, null);
  ParamTop.fAddDirectory("Using%20this%20Library", null, null, null, null);
}

function  WWHBookGroups_ShowBooks()
{
  return true;
}

function  WWHBookGroups_ExpandAllAtTop()
{
  return false;
}
