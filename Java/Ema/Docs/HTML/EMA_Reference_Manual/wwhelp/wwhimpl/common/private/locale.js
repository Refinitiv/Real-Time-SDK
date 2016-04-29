// Copyright (c) 2001-2003 Quadralay Corporation.  All rights reserved.
//

function  WWHCommonMessages_Object()
{
  // Set default messages
  //
  WWHCommonMessages_Set_en(this);

  this.fSetByLocale = WWHCommonMessages_SetByLocale;
}

function  WWHCommonMessages_SetByLocale(ParamLocale)
{
  var  LocaleFunction = null;


  // Match locale
  //
  if ((ParamLocale.length > 1) &&
      (eval("typeof(WWHCommonMessages_Set_" + ParamLocale + ")") == "function"))
  {
    LocaleFunction = eval("WWHCommonMessages_Set_" + ParamLocale);
  }
  else if ((ParamLocale.length > 1) &&
           (eval("typeof(WWHCommonMessages_Set_" + ParamLocale.substring(0, 2) + ")") == "function"))
  {
    LocaleFunction = eval("WWHCommonMessages_Set_" + ParamLocale.substring(0, 2));
  }

  // Default already set, only override if locale found
  //
  if (LocaleFunction != null)
  {
    LocaleFunction(this);
  }
}

function  WWHCommonMessages_Set_de(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Navigation anzeigen";
  ParamMessages.mSyncIconLabel           = "Im Inhalt anzeigen";
  ParamMessages.mPrevIconLabel           = "Zur\u00fcck";
  ParamMessages.mNextIconLabel           = "Weiter";
  ParamMessages.mRelatedTopicsIconLabel  = "Verwandte Themen";
  ParamMessages.mEmailIconLabel          = "E-Mail";
  ParamMessages.mPrintIconLabel          = "Drucken";
  ParamMessages.mBookmarkIconLabel       = "Lesezeichen";
  ParamMessages.mBookmarkLinkMessage     = "Klicken Sie mit der rechten Maustaste auf die Verkn\u00fcpfung, und f\u00fcgen Sie sie Ihren Lesezeichen hinzu.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Siehe auch";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Brauchen Sie Unterst\u00fctzung f\u00fcr zugreifbare HTML?";
  ParamMessages.mBrowserNotSupported = "Ihr Webbrowser unterst\u00fctzt die zum ordnungsgem\u00e4\u00dfen Anzeigen\\ndieser Seite erforderlichen Funktionen nicht. Folgende Browser werden unterst\u00fctzt:\\n\\n IE4 und h\u00f6her f\u00fcr Windows und UNIX\\n IE5 und h\u00f6her f\u00fcr Mac\\n Netscape 6.1 und h\u00f6her f\u00fcr Windows, Mac und UNIX\\n Netscape 4.x f\u00fcr Windows, Mac und UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Dokument";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Deaktivierte Schaltfl\u00e4che %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Klicken Sie hier, um zum Quelldokument zu gelangen.";
}

function  WWHCommonMessages_Set_en(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Show Navigation";
  ParamMessages.mSyncIconLabel           = "Show in Contents";
  ParamMessages.mPrevIconLabel           = "Previous";
  ParamMessages.mNextIconLabel           = "Next";
  ParamMessages.mRelatedTopicsIconLabel  = "Related Topics";
  ParamMessages.mEmailIconLabel          = "E-mail";
  ParamMessages.mPrintIconLabel          = "Print";
  ParamMessages.mBookmarkIconLabel       = "Bookmark";
  ParamMessages.mBookmarkLinkMessage     = "Right-click link and add it to your bookmarks.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "See Also";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Do you require accessible HTML support?";
  ParamMessages.mBrowserNotSupported = "Your web browser does not support the necessary features\\nrequired to view this page properly.  Supported browsers are:\\n\\n  IE4 and later on Windows and UNIX\\n  IE5 and later on Mac\\n  Netscape 6.1 and later on Windows, Mac, and UNIX\\n  Netscape 4.x on Windows, Mac, and UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Document";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Disabled button %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Click here to go to the source document.";
}

function  WWHCommonMessages_Set_es(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Mostrar barra de navegaci\u00f3n";
  ParamMessages.mSyncIconLabel           = "Mostrar en Contenido";
  ParamMessages.mPrevIconLabel           = "Atr\u00e1s";
  ParamMessages.mNextIconLabel           = "Adelante";
  ParamMessages.mRelatedTopicsIconLabel  = "Temas relacionados";
  ParamMessages.mEmailIconLabel          = "E-mail";
  ParamMessages.mPrintIconLabel          = "Imprimir";
  ParamMessages.mBookmarkIconLabel       = "Marcador";
  ParamMessages.mBookmarkLinkMessage     = "Haga clic con el bot\u00f3n derecho del mouse en el v\u00ednculo para agregarlo a la lista de favoritos.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Consulte tambi\u00e9n";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "\u00bfrequiere asistencia accesible a trav\u00e9s de HTML?";
  ParamMessages.mBrowserNotSupported = "Su explorador de Internet no es compatible con las funciones\\nnecesarias para ver esta p\u00e1gina correctamente. Los exploradores compatibles son:\\n\\n IE4 y posteriores para Windows y UNIX\\n IE5 y posteriores para Mac\\n Netscape 6.1 y posteriores para Windows, Mac y UNIX\\n Netscape 4.x para Windows, Mac y UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Documento";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Bot\u00f3n desactivado %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Haga clic aqu\u00ed para ir al documento original.";
}

function  WWHCommonMessages_Set_fr(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Navigation";
  ParamMessages.mSyncIconLabel           = "Afficher dans la table des mati\u00e8res";
  ParamMessages.mPrevIconLabel           = "Pr\u00e9c\u00e9dent";
  ParamMessages.mNextIconLabel           = "Suivant";
  ParamMessages.mRelatedTopicsIconLabel  = "Rubriques associ\u00e9es";
  ParamMessages.mEmailIconLabel          = "Courrier \u00e9lectronique";
  ParamMessages.mPrintIconLabel          = "Imprimer";
  ParamMessages.mBookmarkIconLabel       = "Ajouter aux Favoris";
  ParamMessages.mBookmarkLinkMessage     = "Cliquez sur ce lien avec le bouton droit de la souris et ajoutez-le \u00e0 vos Favoris.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Voir aussi";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Avez-vous besoin de support pour le HTML accessible\u00a0?";
  ParamMessages.mBrowserNotSupported = "Votre navigateur Web ne prend pas en charge les fonctions\\nrequises pour visualiser cette page de mani\u00e8re correcte. Les navigateurs pris en charge sont\u00a0:\\n\\n IE4 et version ult\u00e9rieure sous Windows et UNIX\\n IE5 et version ult\u00e9rieure sur Mac\\n Netscape 6.1 et version ult\u00e9rieure sous Windows, Mac et UNIX\\n Netscape 4.x sous Windows, Mac et UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Document";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Bouton d\u00e9sactiv\u00e9 %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Cliquez ici pour atteindre le document source.";
}

function  WWHCommonMessages_Set_it(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Mostra navigazione";
  ParamMessages.mSyncIconLabel           = "Mostra in Contenuto";
  ParamMessages.mPrevIconLabel           = "Precedente";
  ParamMessages.mNextIconLabel           = "Avanti";
  ParamMessages.mRelatedTopicsIconLabel  = "Argomenti correlati";
  ParamMessages.mEmailIconLabel          = "E-mail";
  ParamMessages.mPrintIconLabel          = "Stampa";
  ParamMessages.mBookmarkIconLabel       = "Segnalibro";
  ParamMessages.mBookmarkLinkMessage     = "Fare clic con il tasto destro del mouse per aggiungere ai Segnalibri.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Vedere anche";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "\u00c8 necessario supporto HTML accessibile?";
  ParamMessages.mBrowserNotSupported = "Il browser Web in uso non supporta le funzioni necessarie\\nper visualizzare correttamente questa pagina. I browser supportati sono:\\n\\n IE4 e versioni successive su Windows e UNIX\\n IE5 e versioni successive su Mac\\n Netscape 6.1 e versioni successive per Windows, Mac e UNIX\\n Netscape 4.x su Windows, Mac e UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Documento";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Pulsante disabilitato %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Fare clic qui per andare al documento di origine.";
}

function  WWHCommonMessages_Set_ja(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "\u30ca\u30d3\u30b2\u30fc\u30b7\u30e7\u30f3 \u30d0\u30fc\u306e\u8868\u793a";
  ParamMessages.mSyncIconLabel           = "\u5185\u5bb9\u306e\u8868\u793a";
  ParamMessages.mPrevIconLabel           = "\u524d\u3078";
  ParamMessages.mNextIconLabel           = "\u6b21\u3078";
  ParamMessages.mRelatedTopicsIconLabel  = "\u95a2\u9023\u30c8\u30d4\u30c3\u30af";
  ParamMessages.mEmailIconLabel          = "\u96fb\u5b50\u30e1\u30fc\u30eb";
  ParamMessages.mPrintIconLabel          = "\u5370\u5237";
  ParamMessages.mBookmarkIconLabel       = "\u30d6\u30c3\u30af\u30de\u30fc\u30af";
  ParamMessages.mBookmarkLinkMessage     = "\u30ea\u30f3\u30af\u3092\u53f3\u30af\u30ea\u30c3\u30af\u3057\u3066\u3001\u30d6\u30c3\u30af\u30de\u30fc\u30af\u306b\u8ffd\u52a0\u3057\u307e\u3059\u3002";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "JA \u95a2\u9023\u9805\u76ee";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "\u30b0\u30e9\u30d5\u30a3\u30c3\u30af\u3092\u8868\u793a\u3067\u304d\u306a\u3044\u5834\u5408\u306e HTML \u30b5\u30dd\u30fc\u30c8\u6a5f\u80fd\u304c\u5fc5\u8981\u3067\u3059\u304b?";
  ParamMessages.mBrowserNotSupported = "\u4f7f\u7528\u4e2d\u306e\u30d6\u30e9\u30a6\u30b6\u306f\u3001\u3053\u306e\u30da\u30fc\u30b8\u3092\u6b63\u3057\u304f\u8868\u793a\u3059\u308b\u305f\u3081\u306b\\n\u5fc5\u8981\u306a\u6a5f\u80fd\u3092\u30b5\u30dd\u30fc\u30c8\u3057\u3066\u3044\u307e\u305b\u3093\u3002 \u30b5\u30dd\u30fc\u30c8\u3055\u308c\u3066\u3044\u308b\u30d6\u30e9\u30a6\u30b6: \\n\\nWindows \u304a\u3088\u3073 UNIX \u4e0a\u3067\u5b9f\u884c\u3055\u308c\u308b IE4 \u4ee5\u964d\\nMac \u4e0a\u3067\u5b9f\u884c\u3055\u308c\u308b IE5 \u4ee5\u964d\\nWindows\u3001Mac\u3001UNIX \u4e0a\u3067\u5b9f\u884c\u3055\u308c\u308b Netscape 6.1 \u4ee5\u964d\\nWindows\u3001Mac\u3001UNIX \u4e0a\u3067\u5b9f\u884c\u3055\u308c\u308b Netscape 4.x";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "\u30c9\u30ad\u30e5\u30e1\u30f3\u30c8";
  ParamMessages.mAccessibilityDisabledNavigationButton = "\u30dc\u30bf\u30f3 %s \u306f\u4f7f\u7528\u3067\u304d\u307e\u305b\u3093";
  ParamMessages.mAccessibilityPopupClickThrough        = "\u30bd\u30fc\u30b9 \u30c9\u30ad\u30e5\u30e1\u30f3\u30c8\u306b\u79fb\u52d5\u3059\u308b\u306b\u306f\u3001\u3053\u3053\u3092\u30af\u30ea\u30c3\u30af\u3057\u307e\u3059\u3002";
}

function  WWHCommonMessages_Set_ko(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "\ub124\ube44\uac8c\uc774\uc158 \ud45c\uc2dc";
  ParamMessages.mSyncIconLabel           = "\ucee8\ud150\uce20\uc5d0\uc11c \ud45c\uc2dc";
  ParamMessages.mPrevIconLabel           = "\uc774\uc804";
  ParamMessages.mNextIconLabel           = "\ub2e4\uc74c";
  ParamMessages.mRelatedTopicsIconLabel  = "\uad00\ub828 \ud56d\ubaa9";
  ParamMessages.mEmailIconLabel          = "\uba54\uc77c";
  ParamMessages.mPrintIconLabel          = "\uc778\uc1c4";
  ParamMessages.mBookmarkIconLabel       = "\ubd81\ub9c8\ud06c";
  ParamMessages.mBookmarkLinkMessage     = "\ub9c1\ud06c\ub97c \ub9c8\uc6b0\uc2a4 \uc624\ub978\ucabd \ub2e8\ucd94\ub85c \ud074\ub9ad\ud558\uc5ec \ubd81\ub9c8\ud06c\uc5d0 \ucd94\uac00\ud569\ub2c8\ub2e4.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "\ucc38\uc870";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "HTML \uc811\uadfc \uc9c0\uc6d0\uc774 \ud544\uc694\ud569\ub2c8\uae4c?";
  ParamMessages.mBrowserNotSupported = "\uc6f9 \ube0c\ub77c\uc6b0\uc800\uac00 \uc774 \ud398\uc774\uc9c0\ub97c \uc62c\ubc14\ub974\uac8c \ud45c\uc2dc\ud558\ub294 \ub370 \\n\ud544\uc694\ud55c \uae30\ub2a5\uc744 \uc9c0\uc6d0\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4. \uc9c0\uc6d0\ub418\ub294 \ube0c\ub77c\uc6b0\uc800:\\n\\nInternet Explorer 4 \uc774\uc0c1(Windows, UNIX )\\n Internet Explorer 5 \uc774\uc0c1(Mac)\\n Netscape 6.1 \uc774\uc0c1(Windows, Mac, UNIX)\\n Netscape 4.x(Windows, Mac, UNIX)";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = "KO ,";
  ParamMessages.mAccessibilityDocumentFrameName        = "\ubb38\uc11c";
  ParamMessages.mAccessibilityDisabledNavigationButton = "\ube44\ud65c\uc131\ud654\ub41c \ub2e8\ucd94 %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "\uc18c\uc2a4 \ubb38\uc11c\ub85c \uc774\ub3d9\ud558\ub824\uba74 \uc5ec\uae30\ub97c \ud074\ub9ad\ud558\uc2ed\uc2dc\uc624.";
}

function  WWHCommonMessages_Set_pt(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Mostrar navega\u00e7\u00e3o";
  ParamMessages.mSyncIconLabel           = "Mostrar em conte\u00fado";
  ParamMessages.mPrevIconLabel           = "Voltar";
  ParamMessages.mNextIconLabel           = "Avan\u00e7ar";
  ParamMessages.mRelatedTopicsIconLabel  = "T\u00f3picos relacionados";
  ParamMessages.mEmailIconLabel          = "E-mail";
  ParamMessages.mPrintIconLabel          = "Imprimir";
  ParamMessages.mBookmarkIconLabel       = "Favoritos";
  ParamMessages.mBookmarkLinkMessage     = "Clique com o bot\u00e3o direito no link para adicion\u00e1-lo aos seus Favoritos.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Ver tamb\u00e9m";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Precisa de suporte HTML acess\u00edvel?";
  ParamMessages.mBrowserNotSupported = "Seu navegador da web n\u00e3o suporta os recursos\\nnecess\u00e1rios \u00e0 visualiza\u00e7\u00e3o desta p\u00e1gina. Os navegadores suportados s\u00e3o:\\n\\n IE4 e vers\u00e3o posterior para Windows e UNIX\\n IE5 e vers\u00e3o posterior para Mac\\n Netscape 6.1 e vers\u00e3o posterior para Windows, Mac e UNIX\\n Netscape 4.x para Windows, Mac e UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Documento";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Bot\u00e3o %s desativado";
  ParamMessages.mAccessibilityPopupClickThrough        = "Clique aqui para ver o documento de origem.";
}

function  WWHCommonMessages_Set_sv(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "Visa navigering";
  ParamMessages.mSyncIconLabel           = "Visa i inneh\u00e5ll";
  ParamMessages.mPrevIconLabel           = "F\u00f6reg\u00e5ende";
  ParamMessages.mNextIconLabel           = "N\u00e4sta";
  ParamMessages.mRelatedTopicsIconLabel  = "N\u00e4rliggande information";
  ParamMessages.mEmailIconLabel          = "E-post";
  ParamMessages.mPrintIconLabel          = "Skriv ut";
  ParamMessages.mBookmarkIconLabel       = "Bokm\u00e4rke";
  ParamMessages.mBookmarkLinkMessage     = "H\u00f6gerklicka p\u00e5 l\u00e4nken om du vill l\u00e4gga till den till dina bokm\u00e4rken.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "Se \u00e4ven";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "Beh\u00f6ver du funktioner f\u00f6r Accessible HTML?";
  ParamMessages.mBrowserNotSupported = "Webbl\u00e4saren inneh\u00e5ller inte de funktioner som kr\u00e4vs f\u00f6r\\natt visa sidan p\u00e5 r\u00e4tt s\u00e4tt. Webbl\u00e4sare som kan anv\u00e4ndas \u00e4r:\\n\\n Internet Explorer 4 eller senare i Windows och UNIX\\n Internet Explorer 5 eller senare i Mac OS\\n Netscape 6.1 eller senare i Windows, Mac OS och UNIX\\n Netscape 4.x i Windows, Mac OS och UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "Dokument";
  ParamMessages.mAccessibilityDisabledNavigationButton = "Avaktiverad knapp %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "Klicka h\u00e4r om du vill visa k\u00e4lldokumentet.";
}

function  WWHCommonMessages_Set_zh(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "\u663e\u793a\u5bfc\u822a";
  ParamMessages.mSyncIconLabel           = "\u5728\u76ee\u5f55\u4e2d\u663e\u793a";
  ParamMessages.mPrevIconLabel           = "\u4e0a\u4e00\u9875";
  ParamMessages.mNextIconLabel           = "\u4e0b\u4e00\u9875";
  ParamMessages.mRelatedTopicsIconLabel  = "\u76f8\u5173\u4e3b\u9898";
  ParamMessages.mEmailIconLabel          = "\u7535\u5b50\u90ae\u4ef6";
  ParamMessages.mPrintIconLabel          = "\u6253\u5370";
  ParamMessages.mBookmarkIconLabel       = "\u4e66\u7b7e";
  ParamMessages.mBookmarkLinkMessage     = "\u53f3\u952e\u5355\u51fb\u94fe\u63a5\uff0c\u5c06\u5176\u6dfb\u52a0\u5230\u4e66\u7b7e\u4e2d\u3002";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "\u53e6\u8bf7\u53c2\u9605";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "\u60a8\u662f\u5426\u9700\u8981\u53ef\u4ee5\u83b7\u5f97\u7684 HTML \u652f\u6301\uff1f";
  ParamMessages.mBrowserNotSupported = "\u60a8\u7684 Web \u6d4f\u89c8\u5668\u4e0d\u652f\u6301\u6b63\u786e\u67e5\u770b\u672c\u9875\u8981\u6c42\u7684\\n\u5fc5\u9700\u529f\u80fd\u3002 \u652f\u6301\u7684\u6d4f\u89c8\u5668\u6709\uff1a\\n\\nIE4 \u53ca\u66f4\u9ad8\u7248\u672c\uff08Windows \u548c UNIX\uff09\\nIE5 \u53ca\u66f4\u9ad8\u7248\u672c\uff08Mac\uff09\\nNetscape 6.1 \u53ca\u66f4\u9ad8\u7248\u672c\uff08Windows\u3001Mac \u548c UNIX\uff09\\nNetscape 4.x\uff08Windows\u3001Mac \u548c UNIX\uff09";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "\u6587\u6863";
  ParamMessages.mAccessibilityDisabledNavigationButton = "\u7981\u7528\u6309\u94ae %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "\u5355\u51fb\u6b64\u5904\u8fdb\u5165\u6e90\u6587\u6863\u3002";
}

function  WWHCommonMessages_Set_zh_tw(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "\u986f\u793a\u5c0e\u89bd";
  ParamMessages.mSyncIconLabel           = "\u986f\u793a\u5728\u76ee\u9304\u4e2d";
  ParamMessages.mPrevIconLabel           = "\u4e0a\u4e00\u9801";
  ParamMessages.mNextIconLabel           = "\u4e0b\u4e00\u9801";
  ParamMessages.mRelatedTopicsIconLabel  = "\u76f8\u95dc\u4e3b\u984c";
  ParamMessages.mEmailIconLabel          = "\u96fb\u5b50\u90f5\u4ef6";
  ParamMessages.mPrintIconLabel          = "\u5217\u5370";
  ParamMessages.mBookmarkIconLabel       = "\u66f8\u7c64";
  ParamMessages.mBookmarkLinkMessage     = "\u53f3\u9375\u6309\u4e00\u4e0b\u9023\u7d50\uff0c\u5c07\u5b83\u52a0\u5165\u5230\u66f8\u7c64\u4e2d\u3002";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "\u53e6\u8acb\u53c3\u95b1";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "\u60a8\u662f\u5426\u9700\u8981 HTML \u5354\u52a9\u529f\u80fd\u652f\u63f4\uff1f";
  ParamMessages.mBrowserNotSupported = "Web \u700f\u89bd\u5668\u4e0d\u652f\u63f4\u6b63\u78ba\u6aa2\u8996\u672c\u9801\u9700\u8981\u7684\u529f\u80fd\u3002 \u652f\u63f4\u7684\u700f\u89bd\u5668\u5305\u62ec\uff1a\\n\\n Windows \u8207 UNIX \u4e0a\u652f\u63f4 IE 4 \u6216\u66f4\u65b0\u7248\u672c\\n Mac \u4e0a\u652f\u63f4 IE5 \u6216\u66f4\u65b0\u7248\u672c\\n Windows\u3001Mac \u8207 UNIX \u4e0a\u652f\u63f4 Netscape 6.1\\n Windows\u3001Mac \u8207 UNIX \u4e0a\u652f\u63f4 Netscape 4.x";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = "\uff0c";
  ParamMessages.mAccessibilityDocumentFrameName        = "\u6587\u4ef6";
  ParamMessages.mAccessibilityDisabledNavigationButton = "\u505c\u7528\u7684\u6309\u9215 %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "\u6309\u4e00\u4e0b\u9019\u88e1\uff0c\u8df3\u81f3\u4f86\u6e90\u6587\u4ef6\u3002";
}

function  WWHCommonMessages_Set_ru(ParamMessages)
{
  // Icon Labels
  //
  ParamMessages.mShowNavigationIconLabel = "\u041f\u043e\u043a\u0430\u0437\u0430\u0442\u044c \u043f\u0430\u043d\u0435\u043b\u044c \u043d\u0430\u0432\u0438\u0433\u0430\u0446\u0438\u0438";
  ParamMessages.mSyncIconLabel           = "\u041f\u043e\u043a\u0430\u0437\u0430\u0442\u044c \u0432 \u0441\u043e\u0434\u0435\u0440\u0436\u0430\u043d\u0438\u0438";
  ParamMessages.mPrevIconLabel           = "\u041d\u0430\u0437\u0430\u0434";
  ParamMessages.mNextIconLabel           = "\u0414\u0430\u043b\u0435\u0435";
  ParamMessages.mRelatedTopicsIconLabel  = "\u0414\u043e\u043f\u043e\u043b\u043d\u0438\u0442\u0435\u043b\u044c\u043d\u0430\u044f \u0438\u043d\u0444\u043e\u0440\u043c\u0430\u0446\u0438\u044f";
  ParamMessages.mEmailIconLabel          = "\u042d\u043b\u0435\u043a\u0442\u0440\u043e\u043d\u043d\u0430\u044f \u043f\u043e\u0447\u0442\u0430";
  ParamMessages.mPrintIconLabel          = "\u041f\u0435\u0447\u0430\u0442\u044c";
  ParamMessages.mBookmarkIconLabel       = "\u0417\u0430\u043a\u043b\u0430\u0434\u043a\u0430";
  ParamMessages.mBookmarkLinkMessage     = "\u0429\u0435\u043b\u043a\u043d\u0438\u0442\u0435 \u043d\u0430 \u0441\u0441\u044b\u043b\u043a\u0435 \u043f\u0440\u0430\u0432\u043e\u0439 \u043a\u043d\u043e\u043f\u043a\u043e\u0439 \u043c\u044b\u0448\u0438 \u0438 \u0434\u043e\u0431\u0430\u0432\u044c\u0442\u0435 \u0435\u0435 \u043a \u0437\u0430\u043a\u043b\u0430\u0434\u043a\u0430\u043c.";
  ParamMessages.mPDFIconLabel            = "PDF";

  // ALinks support
  //
  ParamMessages.mSeeAlsoLabel = "\u0421\u043c. \u0442\u0430\u043a\u0436\u0435";

  // Browser support messages
  //
  ParamMessages.mUseAccessibleHTML   = "\u0422\u0440\u0435\u0431\u0443\u0435\u0442\u0441\u044f \u043b\u0438 \u0434\u043e\u0441\u0442\u0443\u043f \u043a \u0441\u0440\u0435\u0434\u0441\u0442\u0432\u0443 \u043f\u043e\u0434\u0434\u0435\u0440\u0436\u043a\u0438 HTML?";
  ParamMessages.mBrowserNotSupported = "\u0418\u0441\u043f\u043e\u043b\u044c\u0437\u0443\u0435\u043c\u044b\u0439 web-\u0431\u0440\u0430\u0443\u0437\u0435\u0440 \u043d\u0435 \u043f\u043e\u0434\u0434\u0435\u0440\u0436\u0438\u0432\u0430\u0435\u0442 \u0444\u0443\u043d\u043a\u0446\u0438\u0438,\\n\u043d\u0435\u043e\u0431\u0445\u043e\u0434\u0438\u043c\u044b\u0435 \u0434\u043b\u044f \u043f\u0440\u0430\u0432\u0438\u043b\u044c\u043d\u043e\u0433\u043e \u043f\u0440\u043e\u0441\u043c\u043e\u0442\u0440\u0430 \u044d\u0442\u043e\u0439 \u0441\u0442\u0440\u0430\u043d\u0438\u0446\u044b.  \u042d\u0442\u0438 \u0444\u0443\u043d\u043a\u0446\u0438\u0438 \u043f\u043e\u0434\u0434\u0435\u0440\u0436\u0438\u0432\u0430\u044e\u0442\u0441\u044f \u0441\u043b\u0435\u0434\u0443\u044e\u0449\u0438\u043c\u0438 \u0431\u0440\u0430\u0443\u0437\u0435\u0440\u0430\u043c\u0438:\\n\\n  IE4 \u0438 \u0431\u043e\u043b\u0435\u0435 \u043f\u043e\u0437\u0434\u043d\u0438\u0435 \u0432\u0435\u0440\u0441\u0438\u0438 \u0434\u043b\u044f Windows \u0438 UNIX\\n  IE5 \u0438 \u0431\u043e\u043b\u0435\u0435 \u043f\u043e\u0437\u0434\u043d\u0438\u0435 \u0432\u0435\u0440\u0441\u0438\u0438 \u0434\u043b\u044f Mac\\n  Netscape 6.1 \u0438 \u0431\u043e\u043b\u0435\u0435 \u043f\u043e\u0437\u0434\u043d\u0438\u0435 \u0432\u0435\u0440\u0441\u0438\u0438 \u0434\u043b\u044f Windows, Mac \u0438 UNIX\\n  Netscape 4.x \u0434\u043b\u044f Windows, Mac \u0438 UNIX";

  // Accessibility messages
  //
  ParamMessages.mAccessibilityListSeparator            = ",";
  ParamMessages.mAccessibilityDocumentFrameName        = "\u0414\u043e\u043a\u0443\u043c\u0435\u043d\u0442";
  ParamMessages.mAccessibilityDisabledNavigationButton = "\u041e\u0442\u043a\u043b\u044e\u0447\u0435\u043d\u0430 \u043a\u043d\u043e\u043f\u043a\u0430 %s";
  ParamMessages.mAccessibilityPopupClickThrough        = "\u0429\u0435\u043b\u043a\u043d\u0438\u0442\u0435 \u0437\u0434\u0435\u0441\u044c \u0434\u043b\u044f \u043f\u0435\u0440\u0435\u0445\u043e\u0434\u0430 \u043a \u0438\u0441\u0445\u043e\u0434\u043d\u043e\u043c\u0443 \u0434\u043e\u043a\u0443\u043c\u0435\u043d\u0442\u0443.";
}
