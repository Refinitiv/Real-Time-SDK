// Copyright (c) 2009-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */

// Derived from icu/source/data/unidata/DerivedCoreProperties.txt
//
// Editor: Visual Studio C# Express 2005
//
//   ^{[0-9A-F][0-9A-F][0-9A-F][0-9A-F]}\.\.{[0-9A-F][0-9A-F][0-9A-F][0-9A-F]} +;.*$
//     if (('\\u\1' <= c) && (c <= '\\u\2')) return true;
//
//   ^{[0-9A-F][0-9A-F][0-9A-F][0-9A-F]} +;.*$
//     if (c == '\\u\1') return true;
//
//   ^{[0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]}\.\.{[0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]} +;.*$
//     if (('\\u\1' <= c) && (c <= '\\u\2')) return true;
//
//   ^{[0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]} +;.*$
//    if (c == '\\u\1') return true;

function WWHUnicodeInfo_Alphabetic(c)
{
  if (('\u0041' <= c) && (c <= '\u005A')) { return true; }
  if (('\u0061' <= c) && (c <= '\u007A')) { return true; }
  if (c === '\u00AA') { return true; }
  if (c === '\u00B5') { return true; }
  if (c === '\u00BA') { return true; }
  if (('\u00C0' <= c) && (c <= '\u00D6')) { return true; }
  if (('\u00D8' <= c) && (c <= '\u00F6')) { return true; }
  if (('\u00F8' <= c) && (c <= '\u01BA')) { return true; }
  if (c === '\u01BB') { return true; }
  if (('\u01BC' <= c) && (c <= '\u01BF')) { return true; }
  if (('\u01C0' <= c) && (c <= '\u01C3')) { return true; }
  if (('\u01C4' <= c) && (c <= '\u0236')) { return true; }
  if (('\u0250' <= c) && (c <= '\u02AF')) { return true; }
  if (('\u02B0' <= c) && (c <= '\u02C1')) { return true; }
  if (('\u02C6' <= c) && (c <= '\u02D1')) { return true; }
  if (('\u02E0' <= c) && (c <= '\u02E4')) { return true; }
  if (c === '\u02EE') { return true; }
  if (c === '\u0345') { return true; }
  if (c === '\u037A') { return true; }
  if (c === '\u0386') { return true; }
  if (('\u0388' <= c) && (c <= '\u038A')) { return true; }
  if (c === '\u038C') { return true; }
  if (('\u038E' <= c) && (c <= '\u03A1')) { return true; }
  if (('\u03A3' <= c) && (c <= '\u03CE')) { return true; }
  if (('\u03D0' <= c) && (c <= '\u03F5')) { return true; }
  if (('\u03F7' <= c) && (c <= '\u03FB')) { return true; }
  if (('\u0400' <= c) && (c <= '\u0481')) { return true; }
  if (('\u048A' <= c) && (c <= '\u04CE')) { return true; }
  if (('\u04D0' <= c) && (c <= '\u04F5')) { return true; }
  if (('\u04F8' <= c) && (c <= '\u04F9')) { return true; }
  if (('\u0500' <= c) && (c <= '\u050F')) { return true; }
  if (('\u0531' <= c) && (c <= '\u0556')) { return true; }
  if (c === '\u0559') { return true; }
  if (('\u0561' <= c) && (c <= '\u0587')) { return true; }
  if (('\u05B0' <= c) && (c <= '\u05B9')) { return true; }
  if (('\u05BB' <= c) && (c <= '\u05BD')) { return true; }
  if (c === '\u05BF') { return true; }
  if (('\u05C1' <= c) && (c <= '\u05C2')) { return true; }
  if (c === '\u05C4') { return true; }
  if (('\u05D0' <= c) && (c <= '\u05EA')) { return true; }
  if (('\u05F0' <= c) && (c <= '\u05F2')) { return true; }
  if (('\u0610' <= c) && (c <= '\u0615')) { return true; }
  if (('\u0621' <= c) && (c <= '\u063A')) { return true; }
  if (c === '\u0640') { return true; }
  if (('\u0641' <= c) && (c <= '\u064A')) { return true; }
  if (('\u064B' <= c) && (c <= '\u0657')) { return true; }
  if (('\u066E' <= c) && (c <= '\u066F')) { return true; }
  if (c === '\u0670') { return true; }
  if (('\u0671' <= c) && (c <= '\u06D3')) { return true; }
  if (c === '\u06D5') { return true; }
  if (('\u06D6' <= c) && (c <= '\u06DC')) { return true; }
  if (('\u06E1' <= c) && (c <= '\u06E4')) { return true; }
  if (('\u06E5' <= c) && (c <= '\u06E6')) { return true; }
  if (('\u06E7' <= c) && (c <= '\u06E8')) { return true; }
  if (c === '\u06ED') { return true; }
  if (('\u06EE' <= c) && (c <= '\u06EF')) { return true; }
  if (('\u06FA' <= c) && (c <= '\u06FC')) { return true; }
  if (c === '\u06FF') { return true; }
  if (c === '\u0710') { return true; }
  if (c === '\u0711') { return true; }
  if (('\u0712' <= c) && (c <= '\u072F')) { return true; }
  if (('\u0730' <= c) && (c <= '\u073F')) { return true; }
  if (('\u074D' <= c) && (c <= '\u074F')) { return true; }
  if (('\u0780' <= c) && (c <= '\u07A5')) { return true; }
  if (('\u07A6' <= c) && (c <= '\u07B0')) { return true; }
  if (c === '\u07B1') { return true; }
  if (('\u0901' <= c) && (c <= '\u0902')) { return true; }
  if (c === '\u0903') { return true; }
  if (('\u0904' <= c) && (c <= '\u0939')) { return true; }
  if (c === '\u093D') { return true; }
  if (('\u093E' <= c) && (c <= '\u0940')) { return true; }
  if (('\u0941' <= c) && (c <= '\u0948')) { return true; }
  if (('\u0949' <= c) && (c <= '\u094C')) { return true; }
  if (c === '\u0950') { return true; }
  if (('\u0958' <= c) && (c <= '\u0961')) { return true; }
  if (('\u0962' <= c) && (c <= '\u0963')) { return true; }
  if (c === '\u0981') { return true; }
  if (('\u0982' <= c) && (c <= '\u0983')) { return true; }
  if (('\u0985' <= c) && (c <= '\u098C')) { return true; }
  if (('\u098F' <= c) && (c <= '\u0990')) { return true; }
  if (('\u0993' <= c) && (c <= '\u09A8')) { return true; }
  if (('\u09AA' <= c) && (c <= '\u09B0')) { return true; }
  if (c === '\u09B2') { return true; }
  if (('\u09B6' <= c) && (c <= '\u09B9')) { return true; }
  if (c === '\u09BD') { return true; }
  if (('\u09BE' <= c) && (c <= '\u09C0')) { return true; }
  if (('\u09C1' <= c) && (c <= '\u09C4')) { return true; }
  if (('\u09C7' <= c) && (c <= '\u09C8')) { return true; }
  if (('\u09CB' <= c) && (c <= '\u09CC')) { return true; }
  if (c === '\u09D7') { return true; }
  if (('\u09DC' <= c) && (c <= '\u09DD')) { return true; }
  if (('\u09DF' <= c) && (c <= '\u09E1')) { return true; }
  if (('\u09E2' <= c) && (c <= '\u09E3')) { return true; }
  if (('\u09F0' <= c) && (c <= '\u09F1')) { return true; }
  if (('\u0A01' <= c) && (c <= '\u0A02')) { return true; }
  if (c === '\u0A03') { return true; }
  if (('\u0A05' <= c) && (c <= '\u0A0A')) { return true; }
  if (('\u0A0F' <= c) && (c <= '\u0A10')) { return true; }
  if (('\u0A13' <= c) && (c <= '\u0A28')) { return true; }
  if (('\u0A2A' <= c) && (c <= '\u0A30')) { return true; }
  if (('\u0A32' <= c) && (c <= '\u0A33')) { return true; }
  if (('\u0A35' <= c) && (c <= '\u0A36')) { return true; }
  if (('\u0A38' <= c) && (c <= '\u0A39')) { return true; }
  if (('\u0A3E' <= c) && (c <= '\u0A40')) { return true; }
  if (('\u0A41' <= c) && (c <= '\u0A42')) { return true; }
  if (('\u0A47' <= c) && (c <= '\u0A48')) { return true; }
  if (('\u0A4B' <= c) && (c <= '\u0A4C')) { return true; }
  if (('\u0A59' <= c) && (c <= '\u0A5C')) { return true; }
  if (c === '\u0A5E') { return true; }
  if (('\u0A70' <= c) && (c <= '\u0A71')) { return true; }
  if (('\u0A72' <= c) && (c <= '\u0A74')) { return true; }
  if (('\u0A81' <= c) && (c <= '\u0A82')) { return true; }
  if (c === '\u0A83') { return true; }
  if (('\u0A85' <= c) && (c <= '\u0A8D')) { return true; }
  if (('\u0A8F' <= c) && (c <= '\u0A91')) { return true; }
  if (('\u0A93' <= c) && (c <= '\u0AA8')) { return true; }
  if (('\u0AAA' <= c) && (c <= '\u0AB0')) { return true; }
  if (('\u0AB2' <= c) && (c <= '\u0AB3')) { return true; }
  if (('\u0AB5' <= c) && (c <= '\u0AB9')) { return true; }
  if (c === '\u0ABD') { return true; }
  if (('\u0ABE' <= c) && (c <= '\u0AC0')) { return true; }
  if (('\u0AC1' <= c) && (c <= '\u0AC5')) { return true; }
  if (('\u0AC7' <= c) && (c <= '\u0AC8')) { return true; }
  if (c === '\u0AC9') { return true; }
  if (('\u0ACB' <= c) && (c <= '\u0ACC')) { return true; }
  if (c === '\u0AD0') { return true; }
  if (('\u0AE0' <= c) && (c <= '\u0AE1')) { return true; }
  if (('\u0AE2' <= c) && (c <= '\u0AE3')) { return true; }
  if (c === '\u0B01') { return true; }
  if (('\u0B02' <= c) && (c <= '\u0B03')) { return true; }
  if (('\u0B05' <= c) && (c <= '\u0B0C')) { return true; }
  if (('\u0B0F' <= c) && (c <= '\u0B10')) { return true; }
  if (('\u0B13' <= c) && (c <= '\u0B28')) { return true; }
  if (('\u0B2A' <= c) && (c <= '\u0B30')) { return true; }
  if (('\u0B32' <= c) && (c <= '\u0B33')) { return true; }
  if (('\u0B35' <= c) && (c <= '\u0B39')) { return true; }
  if (c === '\u0B3D') { return true; }
  if (c === '\u0B3E') { return true; }
  if (c === '\u0B3F') { return true; }
  if (c === '\u0B40') { return true; }
  if (('\u0B41' <= c) && (c <= '\u0B43')) { return true; }
  if (('\u0B47' <= c) && (c <= '\u0B48')) { return true; }
  if (('\u0B4B' <= c) && (c <= '\u0B4C')) { return true; }
  if (c === '\u0B56') { return true; }
  if (c === '\u0B57') { return true; }
  if (('\u0B5C' <= c) && (c <= '\u0B5D')) { return true; }
  if (('\u0B5F' <= c) && (c <= '\u0B61')) { return true; }
  if (c === '\u0B71') { return true; }
  if (c === '\u0B82') { return true; }
  if (c === '\u0B83') { return true; }
  if (('\u0B85' <= c) && (c <= '\u0B8A')) { return true; }
  if (('\u0B8E' <= c) && (c <= '\u0B90')) { return true; }
  if (('\u0B92' <= c) && (c <= '\u0B95')) { return true; }
  if (('\u0B99' <= c) && (c <= '\u0B9A')) { return true; }
  if (c === '\u0B9C') { return true; }
  if (('\u0B9E' <= c) && (c <= '\u0B9F')) { return true; }
  if (('\u0BA3' <= c) && (c <= '\u0BA4')) { return true; }
  if (('\u0BA8' <= c) && (c <= '\u0BAA')) { return true; }
  if (('\u0BAE' <= c) && (c <= '\u0BB5')) { return true; }
  if (('\u0BB7' <= c) && (c <= '\u0BB9')) { return true; }
  if (('\u0BBE' <= c) && (c <= '\u0BBF')) { return true; }
  if (c === '\u0BC0') { return true; }
  if (('\u0BC1' <= c) && (c <= '\u0BC2')) { return true; }
  if (('\u0BC6' <= c) && (c <= '\u0BC8')) { return true; }
  if (('\u0BCA' <= c) && (c <= '\u0BCC')) { return true; }
  if (c === '\u0BD7') { return true; }
  if (('\u0C01' <= c) && (c <= '\u0C03')) { return true; }
  if (('\u0C05' <= c) && (c <= '\u0C0C')) { return true; }
  if (('\u0C0E' <= c) && (c <= '\u0C10')) { return true; }
  if (('\u0C12' <= c) && (c <= '\u0C28')) { return true; }
  if (('\u0C2A' <= c) && (c <= '\u0C33')) { return true; }
  if (('\u0C35' <= c) && (c <= '\u0C39')) { return true; }
  if (('\u0C3E' <= c) && (c <= '\u0C40')) { return true; }
  if (('\u0C41' <= c) && (c <= '\u0C44')) { return true; }
  if (('\u0C46' <= c) && (c <= '\u0C48')) { return true; }
  if (('\u0C4A' <= c) && (c <= '\u0C4C')) { return true; }
  if (('\u0C55' <= c) && (c <= '\u0C56')) { return true; }
  if (('\u0C60' <= c) && (c <= '\u0C61')) { return true; }
  if (('\u0C82' <= c) && (c <= '\u0C83')) { return true; }
  if (('\u0C85' <= c) && (c <= '\u0C8C')) { return true; }
  if (('\u0C8E' <= c) && (c <= '\u0C90')) { return true; }
  if (('\u0C92' <= c) && (c <= '\u0CA8')) { return true; }
  if (('\u0CAA' <= c) && (c <= '\u0CB3')) { return true; }
  if (('\u0CB5' <= c) && (c <= '\u0CB9')) { return true; }
  if (c === '\u0CBD') { return true; }
  if (c === '\u0CBE') { return true; }
  if (c === '\u0CBF') { return true; }
  if (('\u0CC0' <= c) && (c <= '\u0CC4')) { return true; }
  if (c === '\u0CC6') { return true; }
  if (('\u0CC7' <= c) && (c <= '\u0CC8')) { return true; }
  if (('\u0CCA' <= c) && (c <= '\u0CCB')) { return true; }
  if (c === '\u0CCC') { return true; }
  if (('\u0CD5' <= c) && (c <= '\u0CD6')) { return true; }
  if (c === '\u0CDE') { return true; }
  if (('\u0CE0' <= c) && (c <= '\u0CE1')) { return true; }
  if (('\u0D02' <= c) && (c <= '\u0D03')) { return true; }
  if (('\u0D05' <= c) && (c <= '\u0D0C')) { return true; }
  if (('\u0D0E' <= c) && (c <= '\u0D10')) { return true; }
  if (('\u0D12' <= c) && (c <= '\u0D28')) { return true; }
  if (('\u0D2A' <= c) && (c <= '\u0D39')) { return true; }
  if (('\u0D3E' <= c) && (c <= '\u0D40')) { return true; }
  if (('\u0D41' <= c) && (c <= '\u0D43')) { return true; }
  if (('\u0D46' <= c) && (c <= '\u0D48')) { return true; }
  if (('\u0D4A' <= c) && (c <= '\u0D4C')) { return true; }
  if (c === '\u0D57') { return true; }
  if (('\u0D60' <= c) && (c <= '\u0D61')) { return true; }
  if (('\u0D82' <= c) && (c <= '\u0D83')) { return true; }
  if (('\u0D85' <= c) && (c <= '\u0D96')) { return true; }
  if (('\u0D9A' <= c) && (c <= '\u0DB1')) { return true; }
  if (('\u0DB3' <= c) && (c <= '\u0DBB')) { return true; }
  if (c === '\u0DBD') { return true; }
  if (('\u0DC0' <= c) && (c <= '\u0DC6')) { return true; }
  if (('\u0DCF' <= c) && (c <= '\u0DD1')) { return true; }
  if (('\u0DD2' <= c) && (c <= '\u0DD4')) { return true; }
  if (c === '\u0DD6') { return true; }
  if (('\u0DD8' <= c) && (c <= '\u0DDF')) { return true; }
  if (('\u0DF2' <= c) && (c <= '\u0DF3')) { return true; }
  if (('\u0E01' <= c) && (c <= '\u0E30')) { return true; }
  if (c === '\u0E31') { return true; }
  if (('\u0E32' <= c) && (c <= '\u0E33')) { return true; }
  if (('\u0E34' <= c) && (c <= '\u0E3A')) { return true; }
  if (('\u0E40' <= c) && (c <= '\u0E45')) { return true; }
  if (c === '\u0E46') { return true; }
  if (c === '\u0E4D') { return true; }
  if (('\u0E81' <= c) && (c <= '\u0E82')) { return true; }
  if (c === '\u0E84') { return true; }
  if (('\u0E87' <= c) && (c <= '\u0E88')) { return true; }
  if (c === '\u0E8A') { return true; }
  if (c === '\u0E8D') { return true; }
  if (('\u0E94' <= c) && (c <= '\u0E97')) { return true; }
  if (('\u0E99' <= c) && (c <= '\u0E9F')) { return true; }
  if (('\u0EA1' <= c) && (c <= '\u0EA3')) { return true; }
  if (c === '\u0EA5') { return true; }
  if (c === '\u0EA7') { return true; }
  if (('\u0EAA' <= c) && (c <= '\u0EAB')) { return true; }
  if (('\u0EAD' <= c) && (c <= '\u0EB0')) { return true; }
  if (c === '\u0EB1') { return true; }
  if (('\u0EB2' <= c) && (c <= '\u0EB3')) { return true; }
  if (('\u0EB4' <= c) && (c <= '\u0EB9')) { return true; }
  if (('\u0EBB' <= c) && (c <= '\u0EBC')) { return true; }
  if (c === '\u0EBD') { return true; }
  if (('\u0EC0' <= c) && (c <= '\u0EC4')) { return true; }
  if (c === '\u0EC6') { return true; }
  if (c === '\u0ECD') { return true; }
  if (('\u0EDC' <= c) && (c <= '\u0EDD')) { return true; }
  if (c === '\u0F00') { return true; }
  if (('\u0F40' <= c) && (c <= '\u0F47')) { return true; }
  if (('\u0F49' <= c) && (c <= '\u0F6A')) { return true; }
  if (('\u0F71' <= c) && (c <= '\u0F7E')) { return true; }
  if (c === '\u0F7F') { return true; }
  if (('\u0F80' <= c) && (c <= '\u0F81')) { return true; }
  if (('\u0F88' <= c) && (c <= '\u0F8B')) { return true; }
  if (('\u0F90' <= c) && (c <= '\u0F97')) { return true; }
  if (('\u0F99' <= c) && (c <= '\u0FBC')) { return true; }
  if (('\u1000' <= c) && (c <= '\u1021')) { return true; }
  if (('\u1023' <= c) && (c <= '\u1027')) { return true; }
  if (('\u1029' <= c) && (c <= '\u102A')) { return true; }
  if (c === '\u102C') { return true; }
  if (('\u102D' <= c) && (c <= '\u1030')) { return true; }
  if (c === '\u1031') { return true; }
  if (c === '\u1032') { return true; }
  if (c === '\u1036') { return true; }
  if (c === '\u1038') { return true; }
  if (('\u1050' <= c) && (c <= '\u1055')) { return true; }
  if (('\u1056' <= c) && (c <= '\u1057')) { return true; }
  if (('\u1058' <= c) && (c <= '\u1059')) { return true; }
  if (('\u10A0' <= c) && (c <= '\u10C5')) { return true; }
  if (('\u10D0' <= c) && (c <= '\u10F8')) { return true; }
  if (('\u1100' <= c) && (c <= '\u1159')) { return true; }
  if (('\u115F' <= c) && (c <= '\u11A2')) { return true; }
  if (('\u11A8' <= c) && (c <= '\u11F9')) { return true; }
  if (('\u1200' <= c) && (c <= '\u1206')) { return true; }
  if (('\u1208' <= c) && (c <= '\u1246')) { return true; }
  if (c === '\u1248') { return true; }
  if (('\u124A' <= c) && (c <= '\u124D')) { return true; }
  if (('\u1250' <= c) && (c <= '\u1256')) { return true; }
  if (c === '\u1258') { return true; }
  if (('\u125A' <= c) && (c <= '\u125D')) { return true; }
  if (('\u1260' <= c) && (c <= '\u1286')) { return true; }
  if (c === '\u1288') { return true; }
  if (('\u128A' <= c) && (c <= '\u128D')) { return true; }
  if (('\u1290' <= c) && (c <= '\u12AE')) { return true; }
  if (c === '\u12B0') { return true; }
  if (('\u12B2' <= c) && (c <= '\u12B5')) { return true; }
  if (('\u12B8' <= c) && (c <= '\u12BE')) { return true; }
  if (c === '\u12C0') { return true; }
  if (('\u12C2' <= c) && (c <= '\u12C5')) { return true; }
  if (('\u12C8' <= c) && (c <= '\u12CE')) { return true; }
  if (('\u12D0' <= c) && (c <= '\u12D6')) { return true; }
  if (('\u12D8' <= c) && (c <= '\u12EE')) { return true; }
  if (('\u12F0' <= c) && (c <= '\u130E')) { return true; }
  if (c === '\u1310') { return true; }
  if (('\u1312' <= c) && (c <= '\u1315')) { return true; }
  if (('\u1318' <= c) && (c <= '\u131E')) { return true; }
  if (('\u1320' <= c) && (c <= '\u1346')) { return true; }
  if (('\u1348' <= c) && (c <= '\u135A')) { return true; }
  if (('\u13A0' <= c) && (c <= '\u13F4')) { return true; }
  if (('\u1401' <= c) && (c <= '\u166C')) { return true; }
  if (('\u166F' <= c) && (c <= '\u1676')) { return true; }
  if (('\u1681' <= c) && (c <= '\u169A')) { return true; }
  if (('\u16A0' <= c) && (c <= '\u16EA')) { return true; }
  if (('\u16EE' <= c) && (c <= '\u16F0')) { return true; }
  if (('\u1700' <= c) && (c <= '\u170C')) { return true; }
  if (('\u170E' <= c) && (c <= '\u1711')) { return true; }
  if (('\u1712' <= c) && (c <= '\u1713')) { return true; }
  if (('\u1720' <= c) && (c <= '\u1731')) { return true; }
  if (('\u1732' <= c) && (c <= '\u1733')) { return true; }
  if (('\u1740' <= c) && (c <= '\u1751')) { return true; }
  if (('\u1752' <= c) && (c <= '\u1753')) { return true; }
  if (('\u1760' <= c) && (c <= '\u176C')) { return true; }
  if (('\u176E' <= c) && (c <= '\u1770')) { return true; }
  if (('\u1772' <= c) && (c <= '\u1773')) { return true; }
  if (('\u1780' <= c) && (c <= '\u17B3')) { return true; }
  if (c === '\u17B6') { return true; }
  if (('\u17B7' <= c) && (c <= '\u17BD')) { return true; }
  if (('\u17BE' <= c) && (c <= '\u17C5')) { return true; }
  if (c === '\u17C6') { return true; }
  if (('\u17C7' <= c) && (c <= '\u17C8')) { return true; }
  if (c === '\u17D7') { return true; }
  if (c === '\u17DC') { return true; }
  if (('\u1820' <= c) && (c <= '\u1842')) { return true; }
  if (c === '\u1843') { return true; }
  if (('\u1844' <= c) && (c <= '\u1877')) { return true; }
  if (('\u1880' <= c) && (c <= '\u18A8')) { return true; }
  if (c === '\u18A9') { return true; }
  if (('\u1900' <= c) && (c <= '\u191C')) { return true; }
  if (('\u1920' <= c) && (c <= '\u1922')) { return true; }
  if (('\u1923' <= c) && (c <= '\u1926')) { return true; }
  if (('\u1927' <= c) && (c <= '\u1928')) { return true; }
  if (('\u1929' <= c) && (c <= '\u192B')) { return true; }
  if (('\u1930' <= c) && (c <= '\u1931')) { return true; }
  if (c === '\u1932') { return true; }
  if (('\u1933' <= c) && (c <= '\u1938')) { return true; }
  if (('\u1950' <= c) && (c <= '\u196D')) { return true; }
  if (('\u1970' <= c) && (c <= '\u1974')) { return true; }
  if (('\u1D00' <= c) && (c <= '\u1D2B')) { return true; }
  if (('\u1D2C' <= c) && (c <= '\u1D61')) { return true; }
  if (('\u1D62' <= c) && (c <= '\u1D6B')) { return true; }
  if (('\u1E00' <= c) && (c <= '\u1E9B')) { return true; }
  if (('\u1EA0' <= c) && (c <= '\u1EF9')) { return true; }
  if (('\u1F00' <= c) && (c <= '\u1F15')) { return true; }
  if (('\u1F18' <= c) && (c <= '\u1F1D')) { return true; }
  if (('\u1F20' <= c) && (c <= '\u1F45')) { return true; }
  if (('\u1F48' <= c) && (c <= '\u1F4D')) { return true; }
  if (('\u1F50' <= c) && (c <= '\u1F57')) { return true; }
  if (c === '\u1F59') { return true; }
  if (c === '\u1F5B') { return true; }
  if (c === '\u1F5D') { return true; }
  if (('\u1F5F' <= c) && (c <= '\u1F7D')) { return true; }
  if (('\u1F80' <= c) && (c <= '\u1FB4')) { return true; }
  if (('\u1FB6' <= c) && (c <= '\u1FBC')) { return true; }
  if (c === '\u1FBE') { return true; }
  if (('\u1FC2' <= c) && (c <= '\u1FC4')) { return true; }
  if (('\u1FC6' <= c) && (c <= '\u1FCC')) { return true; }
  if (('\u1FD0' <= c) && (c <= '\u1FD3')) { return true; }
  if (('\u1FD6' <= c) && (c <= '\u1FDB')) { return true; }
  if (('\u1FE0' <= c) && (c <= '\u1FEC')) { return true; }
  if (('\u1FF2' <= c) && (c <= '\u1FF4')) { return true; }
  if (('\u1FF6' <= c) && (c <= '\u1FFC')) { return true; }
  if (c === '\u2071') { return true; }
  if (c === '\u207F') { return true; }
  if (c === '\u2102') { return true; }
  if (c === '\u2107') { return true; }
  if (('\u210A' <= c) && (c <= '\u2113')) { return true; }
  if (c === '\u2115') { return true; }
  if (('\u2119' <= c) && (c <= '\u211D')) { return true; }
  if (c === '\u2124') { return true; }
  if (c === '\u2126') { return true; }
  if (c === '\u2128') { return true; }
  if (('\u212A' <= c) && (c <= '\u212D')) { return true; }
  if (('\u212F' <= c) && (c <= '\u2131')) { return true; }
  if (('\u2133' <= c) && (c <= '\u2134')) { return true; }
  if (('\u2135' <= c) && (c <= '\u2138')) { return true; }
  if (c === '\u2139') { return true; }
  if (('\u213D' <= c) && (c <= '\u213F')) { return true; }
  if (('\u2145' <= c) && (c <= '\u2149')) { return true; }
  if (('\u2160' <= c) && (c <= '\u2183')) { return true; }
  if (c === '\u3005') { return true; }
  if (c === '\u3006') { return true; }
  if (c === '\u3007') { return true; }
  if (('\u3021' <= c) && (c <= '\u3029')) { return true; }
  if (('\u3031' <= c) && (c <= '\u3035')) { return true; }
  if (('\u3038' <= c) && (c <= '\u303A')) { return true; }
  if (c === '\u303B') { return true; }
  if (c === '\u303C') { return true; }
  if (('\u3041' <= c) && (c <= '\u3096')) { return true; }
  if (('\u309D' <= c) && (c <= '\u309E')) { return true; }
  if (c === '\u309F') { return true; }
  if (('\u30A1' <= c) && (c <= '\u30FA')) { return true; }
  if (('\u30FC' <= c) && (c <= '\u30FE')) { return true; }
  if (c === '\u30FF') { return true; }
  if (('\u3105' <= c) && (c <= '\u312C')) { return true; }
  if (('\u3131' <= c) && (c <= '\u318E')) { return true; }
  if (('\u31A0' <= c) && (c <= '\u31B7')) { return true; }
  if (('\u31F0' <= c) && (c <= '\u31FF')) { return true; }
  if (('\u3400' <= c) && (c <= '\u4DB5')) { return true; }
  if (('\u4E00' <= c) && (c <= '\u9FA5')) { return true; }
  if (('\uA000' <= c) && (c <= '\uA48C')) { return true; }
  if (('\uAC00' <= c) && (c <= '\uD7A3')) { return true; }
  if (('\uF900' <= c) && (c <= '\uFA2D')) { return true; }
  if (('\uFA30' <= c) && (c <= '\uFA6A')) { return true; }
  if (('\uFB00' <= c) && (c <= '\uFB06')) { return true; }
  if (('\uFB13' <= c) && (c <= '\uFB17')) { return true; }
  if (c === '\uFB1D') { return true; }
  if (c === '\uFB1E') { return true; }
  if (('\uFB1F' <= c) && (c <= '\uFB28')) { return true; }
  if (('\uFB2A' <= c) && (c <= '\uFB36')) { return true; }
  if (('\uFB38' <= c) && (c <= '\uFB3C')) { return true; }
  if (c === '\uFB3E') { return true; }
  if (('\uFB40' <= c) && (c <= '\uFB41')) { return true; }
  if (('\uFB43' <= c) && (c <= '\uFB44')) { return true; }
  if (('\uFB46' <= c) && (c <= '\uFBB1')) { return true; }
  if (('\uFBD3' <= c) && (c <= '\uFD3D')) { return true; }
  if (('\uFD50' <= c) && (c <= '\uFD8F')) { return true; }
  if (('\uFD92' <= c) && (c <= '\uFDC7')) { return true; }
  if (('\uFDF0' <= c) && (c <= '\uFDFB')) { return true; }
  if (('\uFE70' <= c) && (c <= '\uFE74')) { return true; }
  if (('\uFE76' <= c) && (c <= '\uFEFC')) { return true; }
  if (('\uFF21' <= c) && (c <= '\uFF3A')) { return true; }
  if (('\uFF41' <= c) && (c <= '\uFF5A')) { return true; }
  if (('\uFF66' <= c) && (c <= '\uFF6F')) { return true; }
  if (c === '\uFF70') { return true; }
  if (('\uFF71' <= c) && (c <= '\uFF9D')) { return true; }
  if (('\uFF9E' <= c) && (c <= '\uFF9F')) { return true; }
  if (('\uFFA0' <= c) && (c <= '\uFFBE')) { return true; }
  if (('\uFFC2' <= c) && (c <= '\uFFC7')) { return true; }
  if (('\uFFCA' <= c) && (c <= '\uFFCF')) { return true; }
  if (('\uFFD2' <= c) && (c <= '\uFFD7')) { return true; }
  if (('\uFFDA' <= c) && (c <= '\uFFDC')) { return true; }
  if (('\u10000' <= c) && (c <= '\u1000B')) { return true; }
  if (('\u1000D' <= c) && (c <= '\u10026')) { return true; }
  if (('\u10028' <= c) && (c <= '\u1003A')) { return true; }
  if (('\u1003C' <= c) && (c <= '\u1003D')) { return true; }
  if (('\u1003F' <= c) && (c <= '\u1004D')) { return true; }
  if (('\u10050' <= c) && (c <= '\u1005D')) { return true; }
  if (('\u10080' <= c) && (c <= '\u100FA')) { return true; }
  if (('\u10300' <= c) && (c <= '\u1031E')) { return true; }
  if (('\u10330' <= c) && (c <= '\u10349')) { return true; }
  if (c === '\u1034A') { return true; }
  if (('\u10380' <= c) && (c <= '\u1039D')) { return true; }
  if (('\u10400' <= c) && (c <= '\u1044F')) { return true; }
  if (('\u10450' <= c) && (c <= '\u1049D')) { return true; }
  if (('\u10800' <= c) && (c <= '\u10805')) { return true; }
  if (c === '\u10808') { return true; }
  if (('\u1080A' <= c) && (c <= '\u10835')) { return true; }
  if (('\u10837' <= c) && (c <= '\u10838')) { return true; }
  if (c === '\u1083C') { return true; }
  if (c === '\u1083F') { return true; }
  if (('\u1D400' <= c) && (c <= '\u1D454')) { return true; }
  if (('\u1D456' <= c) && (c <= '\u1D49C')) { return true; }
  if (('\u1D49E' <= c) && (c <= '\u1D49F')) { return true; }
  if (c === '\u1D4A2') { return true; }
  if (('\u1D4A5' <= c) && (c <= '\u1D4A6')) { return true; }
  if (('\u1D4A9' <= c) && (c <= '\u1D4AC')) { return true; }
  if (('\u1D4AE' <= c) && (c <= '\u1D4B9')) { return true; }
  if (c === '\u1D4BB') { return true; }
  if (('\u1D4BD' <= c) && (c <= '\u1D4C3')) { return true; }
  if (('\u1D4C5' <= c) && (c <= '\u1D505')) { return true; }
  if (('\u1D507' <= c) && (c <= '\u1D50A')) { return true; }
  if (('\u1D50D' <= c) && (c <= '\u1D514')) { return true; }
  if (('\u1D516' <= c) && (c <= '\u1D51C')) { return true; }
  if (('\u1D51E' <= c) && (c <= '\u1D539')) { return true; }
  if (('\u1D53B' <= c) && (c <= '\u1D53E')) { return true; }
  if (('\u1D540' <= c) && (c <= '\u1D544')) { return true; }
  if (c === '\u1D546') { return true; }
  if (('\u1D54A' <= c) && (c <= '\u1D550')) { return true; }
  if (('\u1D552' <= c) && (c <= '\u1D6A3')) { return true; }
  if (('\u1D6A8' <= c) && (c <= '\u1D6C0')) { return true; }
  if (('\u1D6C2' <= c) && (c <= '\u1D6DA')) { return true; }
  if (('\u1D6DC' <= c) && (c <= '\u1D6FA')) { return true; }
  if (('\u1D6FC' <= c) && (c <= '\u1D714')) { return true; }
  if (('\u1D716' <= c) && (c <= '\u1D734')) { return true; }
  if (('\u1D736' <= c) && (c <= '\u1D74E')) { return true; }
  if (('\u1D750' <= c) && (c <= '\u1D76E')) { return true; }
  if (('\u1D770' <= c) && (c <= '\u1D788')) { return true; }
  if (('\u1D78A' <= c) && (c <= '\u1D7A8')) { return true; }
  if (('\u1D7AA' <= c) && (c <= '\u1D7C2')) { return true; }
  if (('\u1D7C4' <= c) && (c <= '\u1D7C9')) { return true; }
  if (('\u20000' <= c) && (c <= '\u2A6D6')) { return true; }
  if (('\u2F800' <= c) && (c <= '\u2FA1D')) { return true; }

  return false;
}

function WWHUnicodeInfo_Grapheme_Extend(c)
{
  if (('\u0300' <= c) && (c <= '\u0357')) { return true; }
  if (('\u035D' <= c) && (c <= '\u036F')) { return true; }
  if (('\u0483' <= c) && (c <= '\u0486')) { return true; }
  if (('\u0488' <= c) && (c <= '\u0489')) { return true; }
  if (('\u0591' <= c) && (c <= '\u05A1')) { return true; }
  if (('\u05A3' <= c) && (c <= '\u05B9')) { return true; }
  if (('\u05BB' <= c) && (c <= '\u05BD')) { return true; }
  if (c === '\u05BF') { return true; }
  if (('\u05C1' <= c) && (c <= '\u05C2')) { return true; }
  if (c === '\u05C4') { return true; }
  if (('\u0610' <= c) && (c <= '\u0615')) { return true; }
  if (('\u064B' <= c) && (c <= '\u0658')) { return true; }
  if (c === '\u0670') { return true; }
  if (('\u06D6' <= c) && (c <= '\u06DC')) { return true; }
  if (c === '\u06DE') { return true; }
  if (('\u06DF' <= c) && (c <= '\u06E4')) { return true; }
  if (('\u06E7' <= c) && (c <= '\u06E8')) { return true; }
  if (('\u06EA' <= c) && (c <= '\u06ED')) { return true; }
  if (c === '\u0711') { return true; }
  if (('\u0730' <= c) && (c <= '\u074A')) { return true; }
  if (('\u07A6' <= c) && (c <= '\u07B0')) { return true; }
  if (('\u0901' <= c) && (c <= '\u0902')) { return true; }
  if (c === '\u093C') { return true; }
  if (('\u0941' <= c) && (c <= '\u0948')) { return true; }
  if (c === '\u094D') { return true; }
  if (('\u0951' <= c) && (c <= '\u0954')) { return true; }
  if (('\u0962' <= c) && (c <= '\u0963')) { return true; }
  if (c === '\u0981') { return true; }
  if (c === '\u09BC') { return true; }
  if (c === '\u09BE') { return true; }
  if (('\u09C1' <= c) && (c <= '\u09C4')) { return true; }
  if (c === '\u09CD') { return true; }
  if (c === '\u09D7') { return true; }
  if (('\u09E2' <= c) && (c <= '\u09E3')) { return true; }
  if (('\u0A01' <= c) && (c <= '\u0A02')) { return true; }
  if (c === '\u0A3C') { return true; }
  if (('\u0A41' <= c) && (c <= '\u0A42')) { return true; }
  if (('\u0A47' <= c) && (c <= '\u0A48')) { return true; }
  if (('\u0A4B' <= c) && (c <= '\u0A4D')) { return true; }
  if (('\u0A70' <= c) && (c <= '\u0A71')) { return true; }
  if (('\u0A81' <= c) && (c <= '\u0A82')) { return true; }
  if (c === '\u0ABC') { return true; }
  if (('\u0AC1' <= c) && (c <= '\u0AC5')) { return true; }
  if (('\u0AC7' <= c) && (c <= '\u0AC8')) { return true; }
  if (c === '\u0ACD') { return true; }
  if (('\u0AE2' <= c) && (c <= '\u0AE3')) { return true; }
  if (c === '\u0B01') { return true; }
  if (c === '\u0B3C') { return true; }
  if (c === '\u0B3E') { return true; }
  if (c === '\u0B3F') { return true; }
  if (('\u0B41' <= c) && (c <= '\u0B43')) { return true; }
  if (c === '\u0B4D') { return true; }
  if (c === '\u0B56') { return true; }
  if (c === '\u0B57') { return true; }
  if (c === '\u0B82') { return true; }
  if (c === '\u0BBE') { return true; }
  if (c === '\u0BC0') { return true; }
  if (c === '\u0BCD') { return true; }
  if (c === '\u0BD7') { return true; }
  if (('\u0C3E' <= c) && (c <= '\u0C40')) { return true; }
  if (('\u0C46' <= c) && (c <= '\u0C48')) { return true; }
  if (('\u0C4A' <= c) && (c <= '\u0C4D')) { return true; }
  if (('\u0C55' <= c) && (c <= '\u0C56')) { return true; }
  if (c === '\u0CBC') { return true; }
  if (c === '\u0CBF') { return true; }
  if (c === '\u0CC2') { return true; }
  if (c === '\u0CC6') { return true; }
  if (('\u0CCC' <= c) && (c <= '\u0CCD')) { return true; }
  if (('\u0CD5' <= c) && (c <= '\u0CD6')) { return true; }
  if (c === '\u0D3E') { return true; }
  if (('\u0D41' <= c) && (c <= '\u0D43')) { return true; }
  if (c === '\u0D4D') { return true; }
  if (c === '\u0D57') { return true; }
  if (c === '\u0DCA') { return true; }
  if (c === '\u0DCF') { return true; }
  if (('\u0DD2' <= c) && (c <= '\u0DD4')) { return true; }
  if (c === '\u0DD6') { return true; }
  if (c === '\u0DDF') { return true; }
  if (c === '\u0E31') { return true; }
  if (('\u0E34' <= c) && (c <= '\u0E3A')) { return true; }
  if (('\u0E47' <= c) && (c <= '\u0E4E')) { return true; }
  if (c === '\u0EB1') { return true; }
  if (('\u0EB4' <= c) && (c <= '\u0EB9')) { return true; }
  if (('\u0EBB' <= c) && (c <= '\u0EBC')) { return true; }
  if (('\u0EC8' <= c) && (c <= '\u0ECD')) { return true; }
  if (('\u0F18' <= c) && (c <= '\u0F19')) { return true; }
  if (c === '\u0F35') { return true; }
  if (c === '\u0F37') { return true; }
  if (c === '\u0F39') { return true; }
  if (('\u0F71' <= c) && (c <= '\u0F7E')) { return true; }
  if (('\u0F80' <= c) && (c <= '\u0F84')) { return true; }
  if (('\u0F86' <= c) && (c <= '\u0F87')) { return true; }
  if (('\u0F90' <= c) && (c <= '\u0F97')) { return true; }
  if (('\u0F99' <= c) && (c <= '\u0FBC')) { return true; }
  if (c === '\u0FC6') { return true; }
  if (('\u102D' <= c) && (c <= '\u1030')) { return true; }
  if (c === '\u1032') { return true; }
  if (('\u1036' <= c) && (c <= '\u1037')) { return true; }
  if (c === '\u1039') { return true; }
  if (('\u1058' <= c) && (c <= '\u1059')) { return true; }
  if (('\u1712' <= c) && (c <= '\u1714')) { return true; }
  if (('\u1732' <= c) && (c <= '\u1734')) { return true; }
  if (('\u1752' <= c) && (c <= '\u1753')) { return true; }
  if (('\u1772' <= c) && (c <= '\u1773')) { return true; }
  if (('\u17B7' <= c) && (c <= '\u17BD')) { return true; }
  if (c === '\u17C6') { return true; }
  if (('\u17C9' <= c) && (c <= '\u17D3')) { return true; }
  if (c === '\u17DD') { return true; }
  if (('\u180B' <= c) && (c <= '\u180D')) { return true; }
  if (c === '\u18A9') { return true; }
  if (('\u1920' <= c) && (c <= '\u1922')) { return true; }
  if (('\u1927' <= c) && (c <= '\u1928')) { return true; }
  if (c === '\u1932') { return true; }
  if (('\u1939' <= c) && (c <= '\u193B')) { return true; }
  if (('\u200C' <= c) && (c <= '\u200D')) { return true; }
  if (('\u20D0' <= c) && (c <= '\u20DC')) { return true; }
  if (('\u20DD' <= c) && (c <= '\u20E0')) { return true; }
  if (c === '\u20E1') { return true; }
  if (('\u20E2' <= c) && (c <= '\u20E4')) { return true; }
  if (('\u20E5' <= c) && (c <= '\u20EA')) { return true; }
  if (('\u302A' <= c) && (c <= '\u302F')) { return true; }
  if (('\u3099' <= c) && (c <= '\u309A')) { return true; }
  if (c === '\uFB1E') { return true; }
  if (('\uFE00' <= c) && (c <= '\uFE0F')) { return true; }
  if (('\uFE20' <= c) && (c <= '\uFE23')) { return true; }
  if (c === '\u1D165') { return true; }
  if (('\u1D167' <= c) && (c <= '\u1D169')) { return true; }
  if (('\u1D16E' <= c) && (c <= '\u1D16F')) { return true; }
  if (('\u1D17B' <= c) && (c <= '\u1D182')) { return true; }
  if (('\u1D185' <= c) && (c <= '\u1D18B')) { return true; }
  if (('\u1D1AA' <= c) && (c <= '\u1D1AD')) { return true; }
  if (('\uE0100' <= c) && (c <= '\uE01EF')) { return true; }

  return false;
}

function WWHUnicodeInfo_Extend(c)
{
  return WWHUnicodeInfo_Grapheme_Extend(c);
}

// http://en.wikipedia.org/wiki/Kana
//
// Hiragana range in Unicode is U+3040 ... U+309F, and the Katakana range is U+30A0 ... U+30FF.
//

function WWHUnicodeInfo_Hiragana(c)
{
  if (('\u3040' <= c) && (c <= '\u309F')) { return true; }

  return false;
}

function WWHUnicodeInfo_Katakana(c)
{
  if (('\u30A0' <= c) && (c <= '\u30FF')) { return true; }
  if (c === '\u30FC') { return true; }
  if (c === '\uFF70') { return true; }
  if (c === '\uFF9E') { return true; }
  if (c === '\uFF9F') { return true; }

  return false;
}

function WWHUnicodeInfo_Ideographic(c)
{
  if (('\u1100' <= c) && (c <= '\u1159')) { return true; }
  if (c === '\u115F') { return true; }
  if (('\u2E80' <= c) && (c <= '\u2E99')) { return true; }
  if (('\u2E9B' <= c) && (c <= '\u2EF3')) { return true; }
  if (('\u2F00' <= c) && (c <= '\u2FD5')) { return true; }
  if (('\u2FF0' <= c) && (c <= '\u2FFB')) { return true; }
  if (c === '\u3000') { return true; }
  if (('\u3003' <= c) && (c <= '\u3004')) { return true; }
  if (('\u3006' <= c) && (c <= '\u3007')) { return true; }
  if (('\u3012' <= c) && (c <= '\u3013')) { return true; }
  if (('\u3020' <= c) && (c <= '\u3029')) { return true; }
  if (('\u3030' <= c) && (c <= '\u303A')) { return true; }
  if (('\u303D' <= c) && (c <= '\u303F')) { return true; }
  if (c === '\u3042') { return true; }
  if (c === '\u3044') { return true; }
  if (c === '\u3046') { return true; }
  if (c === '\u3048') { return true; }
  if (('\u304A' <= c) && (c <= '\u3062')) { return true; }
  if (('\u3064' <= c) && (c <= '\u3082')) { return true; }
  if (c === '\u3084') { return true; }
  if (c === '\u3086') { return true; }
  if (('\u3088' <= c) && (c <= '\u308D')) { return true; }
  if (('\u308F' <= c) && (c <= '\u3094')) { return true; }
  if (c === '\u309F') { return true; }
  if (c === '\u30A2') { return true; }
  if (c === '\u30A4') { return true; }
  if (c === '\u30A6') { return true; }
  if (c === '\u30A8') { return true; }
  if (('\u30AA' <= c) && (c <= '\u30C2')) { return true; }
  if (('\u30C4' <= c) && (c <= '\u30E2')) { return true; }
  if (c === '\u30E4') { return true; }
  if (c === '\u30E6') { return true; }
  if (('\u30E8' <= c) && (c <= '\u30ED')) { return true; }
  if (('\u30EF' <= c) && (c <= '\u30F4')) { return true; }
  if (('\u30F7' <= c) && (c <= '\u30FA')) { return true; }
  if (c === '\u30FC') { return true; }
  if (('\u30FE' <= c) && (c <= '\u30FF')) { return true; }
  if (('\u3105' <= c) && (c <= '\u312C')) { return true; }
  if (('\u3131' <= c) && (c <= '\u318E')) { return true; }
  if (('\u3190' <= c) && (c <= '\u31B7')) { return true; }
  if (('\u3200' <= c) && (c <= '\u321C')) { return true; }
  if (('\u3220' <= c) && (c <= '\u3243')) { return true; }
  if (('\u3251' <= c) && (c <= '\u327B')) { return true; }
  if (('\u327F' <= c) && (c <= '\u32CB')) { return true; }
  if (('\u32D0' <= c) && (c <= '\u32FE')) { return true; }
  if (('\u3300' <= c) && (c <= '\u3376')) { return true; }
  if (('\u337B' <= c) && (c <= '\u33DD')) { return true; }
  if (('\u33E0' <= c) && (c <= '\u33FE')) { return true; }
  if (('\u3400' <= c) && (c <= '\u4DB5')) { return true; }
  if (('\u4E00' <= c) && (c <= '\u9FA5')) { return true; }
  if (('\uA000' <= c) && (c <= '\uA48C')) { return true; }
  if (('\uA490' <= c) && (c <= '\uA4C6')) { return true; }
  if (('\uAC00' <= c) && (c <= '\uD7A3')) { return true; }
  if (('\uF900' <= c) && (c <= '\uFA2D')) { return true; }
  if (('\uFA30' <= c) && (c <= '\uFA6A')) { return true; }
  if (('\uFE30' <= c) && (c <= '\uFE34')) { return true; }
  if (('\uFE45' <= c) && (c <= '\uFE46')) { return true; }
  if (('\uFE49' <= c) && (c <= '\uFE4F')) { return true; }
  if (c === '\uFE51') { return true; }
  if (c === '\uFE58') { return true; }
  if (('\uFE5F' <= c) && (c <= '\uFE66')) { return true; }
  if (c === '\uFE68') { return true; }
  if (c === '\uFE6B') { return true; }
  if (('\uFF02' <= c) && (c <= '\uFF03')) { return true; }
  if (('\uFF06' <= c) && (c <= '\uFF07')) { return true; }
  if (('\uFF0A' <= c) && (c <= '\uFF0B')) { return true; }
  if (c === '\uFF0D') { return true; }
  if (('\uFF0F' <= c) && (c <= '\uFF19')) { return true; }
  if (('\uFF1C' <= c) && (c <= '\uFF1E')) { return true; }
  if (('\uFF20' <= c) && (c <= '\uFF3A')) { return true; }
  if (c === '\uFF3C') { return true; }
  if (('\uFF3E' <= c) && (c <= '\uFF5A')) { return true; }
  if (c === '\uFF5C') { return true; }
  if (c === '\uFF5E') { return true; }
  if (('\uFFE2' <= c) && (c <= '\uFFE4')) { return true; }

  return false;
}

function WWHUnicodeInfo_ALetter(c)
{
  if (c === '\u05F3') { return true; }
  if (WWHUnicodeInfo_Ideographic(c)) { return false; }
  if (WWHUnicodeInfo_Katakana(c)) { return false; }
  if (WWHUnicodeInfo_Alphabetic(c)) { return true; }

  return false;
}

function WWHUnicodeInfo_ABaseLetter(c)
{
  if (WWHUnicodeInfo_Grapheme_Extend(c)) { return false; }
  if (WWHUnicodeInfo_ALetter(c)) { return true; }

  return false;
}

function WWHUnicodeInfo_ACMLetter(c)
{
  if (WWHUnicodeInfo_Grapheme_Extend(c))
  {
    if (WWHUnicodeInfo_ALetter(c)) { return true; }
  }

  return false;
}

function WWHUnicodeInfo_MidLetter(c)
{
  if (c === '\u0027') { return true; }
  if (c === '\u00B7') { return true; }
  if (c === '\u05F4') { return true; }
  if (c === '\u2019') { return true; }
  if (c === '\u2027') { return true; }

  return false;
}

function WWHUnicodeInfo_MidNumLet(c)
{
  if (c === '\u002E') { return true; }
  if (c === '\u003A') { return true; }

  return false;
}

function WWHUnicodeInfo_MidNum(c)
{
  if (c === '\u002C') { return true; }
  if (c === '\u002E') { return true; }
  if (c === '\u003a') { return true; }
  if (c === '\u003b') { return true; }
  if (c === '\u0589') { return true; }

  return false;
}

function WWHUnicodeInfo_Numeric(c)
{
  if (('\u0030' <= c) && (c <= '\u0039')) { return true; }
  if (('\u0660' <= c) && (c <= '\u0669')) { return true; }
  if (('\u066B' <= c) && (c <= '\u066C')) { return true; }
  if (('\u06F0' <= c) && (c <= '\u06F9')) { return true; }
  if (('\u07C0' <= c) && (c <= '\u07C9')) { return true; }
  if (('\u0966' <= c) && (c <= '\u096F')) { return true; }
  if (('\u09E6' <= c) && (c <= '\u09EF')) { return true; }
  if (('\u0A66' <= c) && (c <= '\u0A6F')) { return true; }
  if (('\u0AE6' <= c) && (c <= '\u0AEF')) { return true; }
  if (('\u0B66' <= c) && (c <= '\u0B6F')) { return true; }
  if (('\u0BE6' <= c) && (c <= '\u0BEF')) { return true; }
  if (('\u0C66' <= c) && (c <= '\u0C6F')) { return true; }
  if (('\u0CE6' <= c) && (c <= '\u0CEF')) { return true; }
  if (('\u0D66' <= c) && (c <= '\u0D6F')) { return true; }
  if (('\u0E50' <= c) && (c <= '\u0E59')) { return true; }
  if (('\u0ED0' <= c) && (c <= '\u0ED9')) { return true; }
  if (('\u0F20' <= c) && (c <= '\u0F29')) { return true; }
  if (('\u1040' <= c) && (c <= '\u1049')) { return true; }
  if (('\u17E0' <= c) && (c <= '\u17E9')) { return true; }
  if (('\u1810' <= c) && (c <= '\u1819')) { return true; }
  if (('\u1946' <= c) && (c <= '\u194F')) { return true; }
  if (('\u19D0' <= c) && (c <= '\u19D9')) { return true; }
  if (('\u1B50' <= c) && (c <= '\u1B59')) { return true; }
  if (('\u104A0' <= c) && (c <= '\u104A9')) { return true; }
  if (('\u1D7CE' <= c) && (c <= '\u1D7FF')) { return true; }

  return false;
}

function WWHUnicodeInfo_Korean_L(c)
{
  if (('\u1100' <= c) && (c <= '\u115f')) { return true; }
  if (('\uac00' <= c) && (c <= '\ud7a3')) { return true; }

  return false;
}

function WWHUnicodeInfo_Korean_V(c)
{
  if (('\u1160' <= c) && (c <= '\u11a2')) { return true; }

  return false;
}

function WWHUnicodeInfo_Korean_T(c)
{
  if (('\u11a8' <= c) && (c <= '\u11f9')) { return true; }

  return false;
}

var WWHUnicodeInfo_Korean_LV_Data = {
'\uac00': true, '\uac1c': true, '\uac38': true, '\uac54': true, '\uac70': true,'\uac8c': true, '\uaca8': true, '\uacc4': true, '\uace0': true, '\uacfc': true, '\uad18': true, '\uad34': true, '\uad50': true, '\uad6c': true, '\uad88': true, '\uada4': true,
'\uadc0': true, '\uaddc': true, '\uadf8': true, '\uae14': true, '\uae30': true, '\uae4c': true, '\uae68': true, '\uae84': true, '\uaea0': true, '\uaebc': true, '\uaed8': true, '\uaef4': true, '\uaf10': true, '\uaf2c': true, '\uaf48': true, '\uaf64': true,
'\uaf80': true, '\uaf9c': true, '\uafb8': true, '\uafd4': true, '\uaff0': true, '\ub00c': true, '\ub028': true, '\ub044': true, '\ub060': true, '\ub07c': true, '\ub098': true, '\ub0b4': true, '\ub0d0': true, '\ub0ec': true, '\ub108': true, '\ub124': true,
'\ub140': true, '\ub15c': true, '\ub178': true, '\ub194': true, '\ub1b0': true, '\ub1cc': true, '\ub1e8': true, '\ub204': true, '\ub220': true, '\ub23c': true, '\ub258': true, '\ub274': true, '\ub290': true, '\ub2ac': true, '\ub2c8': true, '\ub2e4': true,
'\ub300': true, '\ub31c': true, '\ub338': true, '\ub354': true, '\ub370': true, '\ub38c': true, '\ub3a8': true, '\ub3c4': true, '\ub3e0': true, '\ub3fc': true, '\ub418': true, '\ub434': true, '\ub450': true, '\ub46c': true, '\ub488': true, '\ub4a4': true,
'\ub4c0': true, '\ub4dc': true, '\ub4f8': true, '\ub514': true, '\ub530': true, '\ub54c': true, '\ub568': true, '\ub584': true, '\ub5a0': true, '\ub5bc': true, '\ub5d8': true, '\ub5f4': true, '\ub610': true, '\ub62c': true, '\ub648': true, '\ub664': true,
'\ub680': true, '\ub69c': true, '\ub6b8': true, '\ub6d4': true, '\ub6f0': true, '\ub70c': true, '\ub728': true, '\ub744': true, '\ub760': true, '\ub77c': true, '\ub798': true, '\ub7b4': true, '\ub7d0': true, '\ub7ec': true, '\ub808': true, '\ub824': true,
'\ub840': true, '\ub85c': true, '\ub878': true, '\ub894': true, '\ub8b0': true, '\ub8cc': true, '\ub8e8': true, '\ub904': true, '\ub920': true, '\ub93c': true, '\ub958': true, '\ub974': true, '\ub990': true, '\ub9ac': true, '\ub9c8': true, '\ub9e4': true,
'\uba00': true, '\uba1c': true, '\uba38': true, '\uba54': true, '\uba70': true, '\uba8c': true, '\ubaa8': true, '\ubac4': true, '\ubae0': true, '\ubafc': true, '\ubb18': true, '\ubb34': true, '\ubb50': true, '\ubb6c': true, '\ubb88': true, '\ubba4': true,
'\ubbc0': true, '\ubbdc': true, '\ubbf8': true, '\ubc14': true, '\ubc30': true, '\ubc4c': true, '\ubc68': true, '\ubc84': true, '\ubca0': true, '\ubcbc': true, '\ubcd8': true, '\ubcf4': true, '\ubd10': true, '\ubd2c': true, '\ubd48': true, '\ubd64': true,
'\ubd80': true, '\ubd9c': true, '\ubdb8': true, '\ubdd4': true, '\ubdf0': true, '\ube0c': true, '\ube28': true, '\ube44': true, '\ube60': true, '\ube7c': true, '\ube98': true, '\ubeb4': true, '\ubed0': true, '\ubeec': true, '\ubf08': true, '\ubf24': true,
'\ubf40': true, '\ubf5c': true, '\ubf78': true, '\ubf94': true, '\ubfb0': true, '\ubfcc': true, '\ubfe8': true, '\uc004': true, '\uc020': true, '\uc03c': true, '\uc058': true, '\uc074': true, '\uc090': true, '\uc0ac': true, '\uc0c8': true, '\uc0e4': true,
'\uc100': true, '\uc11c': true, '\uc138': true, '\uc154': true, '\uc170': true, '\uc18c': true, '\uc1a8': true, '\uc1c4': true, '\uc1e0': true, '\uc1fc': true, '\uc218': true, '\uc234': true, '\uc250': true, '\uc26c': true, '\uc288': true, '\uc2a4': true,
'\uc2c0': true, '\uc2dc': true, '\uc2f8': true, '\uc314': true, '\uc330': true, '\uc34c': true, '\uc368': true, '\uc384': true, '\uc3a0': true, '\uc3bc': true, '\uc3d8': true, '\uc3f4': true, '\uc410': true, '\uc42c': true, '\uc448': true, '\uc464': true,
'\uc480': true, '\uc49c': true, '\uc4b8': true, '\uc4d4': true, '\uc4f0': true, '\uc50c': true, '\uc528': true, '\uc544': true, '\uc560': true, '\uc57c': true, '\uc598': true, '\uc5b4': true, '\uc5d0': true, '\uc5ec': true, '\uc608': true, '\uc624': true,
'\uc640': true, '\uc65c': true, '\uc678': true, '\uc694': true, '\uc6b0': true, '\uc6cc': true, '\uc6e8': true, '\uc704': true, '\uc720': true, '\uc73c': true, '\uc758': true, '\uc774': true, '\uc790': true, '\uc7ac': true, '\uc7c8': true, '\uc7e4': true,
'\uc800': true, '\uc81c': true, '\uc838': true, '\uc854': true, '\uc870': true, '\uc88c': true, '\uc8a8': true, '\uc8c4': true, '\uc8e0': true, '\uc8fc': true, '\uc918': true, '\uc934': true, '\uc950': true, '\uc96c': true, '\uc988': true, '\uc9a4': true,
'\uc9c0': true, '\uc9dc': true, '\uc9f8': true, '\uca14': true, '\uca30': true, '\uca4c': true, '\uca68': true, '\uca84': true, '\ucaa0': true, '\ucabc': true, '\ucad8': true, '\ucaf4': true, '\ucb10': true, '\ucb2c': true, '\ucb48': true, '\ucb64': true,
'\ucb80': true, '\ucb9c': true, '\ucbb8': true, '\ucbd4': true, '\ucbf0': true, '\ucc0c': true, '\ucc28': true, '\ucc44': true, '\ucc60': true, '\ucc7c': true, '\ucc98': true, '\uccb4': true, '\uccd0': true, '\uccec': true, '\ucd08': true, '\ucd24': true,
'\ucd40': true, '\ucd5c': true, '\ucd78': true, '\ucd94': true, '\ucdb0': true, '\ucdcc': true, '\ucde8': true, '\uce04': true, '\uce20': true, '\uce3c': true, '\uce58': true, '\uce74': true, '\uce90': true, '\uceac': true, '\ucec8': true, '\ucee4': true,
'\ucf00': true, '\ucf1c': true, '\ucf38': true, '\ucf54': true, '\ucf70': true, '\ucf8c': true, '\ucfa8': true, '\ucfc4': true, '\ucfe0': true, '\ucffc': true, '\ud018': true, '\ud034': true, '\ud050': true, '\ud06c': true, '\ud088': true, '\ud0a4': true,
'\ud0c0': true, '\ud0dc': true, '\ud0f8': true, '\ud114': true, '\ud130': true, '\ud14c': true, '\ud168': true, '\ud184': true, '\ud1a0': true, '\ud1bc': true, '\ud1d8': true, '\ud1f4': true, '\ud210': true, '\ud22c': true, '\ud248': true, '\ud264': true,
'\ud280': true, '\ud29c': true, '\ud2b8': true, '\ud2d4': true, '\ud2f0': true, '\ud30c': true, '\ud328': true, '\ud344': true, '\ud360': true, '\ud37c': true, '\ud398': true, '\ud3b4': true, '\ud3d0': true, '\ud3ec': true, '\ud408': true, '\ud424': true,
'\ud440': true, '\ud45c': true, '\ud478': true, '\ud494': true, '\ud4b0': true, '\ud4cc': true, '\ud4e8': true, '\ud504': true, '\ud520': true, '\ud53c': true, '\ud558': true, '\ud574': true, '\ud590': true, '\ud5ac': true, '\ud5c8': true, '\ud5e4': true,
'\ud600': true, '\ud61c': true, '\ud638': true, '\ud654': true, '\ud670': true, '\ud68c': true, '\ud6a8': true, '\ud6c4': true, '\ud6e0': true, '\ud6fc': true, '\ud718': true, '\ud734': true, '\ud750': true, '\ud76c': true, '\ud788': true
                             };

function WWHUnicodeInfo_Korean_LV(c)
{
  if (WWHUnicodeInfo_Korean_LV_Data[c] === true) { return true; }

  return false;
}

function WWHUnicodeInfo_Korean_LVT(c)
{
  if ( ! WWHUnicodeInfo_Korean_LV(c))
  {
    if (('\uac00' <= c) && (c <= '\ud7a3')) { return true; }
  }
  return false;
}

var WWHUnicodeInfo_WWNoBreak_Data = {
'\u2027': true,
'\u00AD': true,
'\u2011': true,
'\u2010': true,
'\u2043': true,
'\u002D': true,
'\u005f': true,
'\u0332': true,
'\u002a': true,
'\u002f': true,
'\u005c': true,
'\u0040': true,
'\u0026': true,
'\u003d': true,
'\u0024': true
                                    };

function WWHUnicodeInfo_WWNoBreak(c)
{
  if (WWHUnicodeInfo_WWNoBreak_Data[c])
  {
    return true;
  }

  return false;
}

var WWHUnicodeInfo_WWOpenBracket_Data = {
'\u0028': true,
'\u005b': true,
'\u007b': true,
'\u003c': true
                                        };

function WWHUnicodeInfo_WWOpenBracket(c)
{
  if (WWHUnicodeInfo_WWOpenBracket_Data[c])
  {
    return true;
  }

  return false;
}

var WWHUnicodeInfo_WWCloseBracket_Data = {
'\u0029': true,
'\u005d': true,
'\u007d': true,
'\u003e': true
                                         };

function WWHUnicodeInfo_WWCloseBracket(c)
{
  if (WWHUnicodeInfo_WWCloseBracket_Data[c])
  {
    return true;
  }

  return false;
}
