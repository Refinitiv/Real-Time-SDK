// Copyright (c) 2001-2003 Quadralay Corporation.  All rights reserved.
//

function  WWHJavaScriptMessages_Object()
{
  // Set default messages
  //
  WWHJavaScriptMessages_Set_en(this);

  this.fSetByLocale = WWHJavaScriptMessages_SetByLocale;
}

function  WWHJavaScriptMessages_SetByLocale(ParamLocale)
{
  var  LocaleFunction = null;


  // Match locale
  //
  if ((ParamLocale.length > 1) &&
      (eval("typeof(WWHJavaScriptMessages_Set_" + ParamLocale + ")") == "function"))
  {
    LocaleFunction = eval("WWHJavaScriptMessages_Set_" + ParamLocale);
  }
  else if ((ParamLocale.length > 1) &&
           (eval("typeof(WWHJavaScriptMessages_Set_" + ParamLocale.substring(0, 2) + ")") == "function"))
  {
    LocaleFunction = eval("WWHJavaScriptMessages_Set_" + ParamLocale.substring(0, 2));
  }

  // Default already set, only override if locale found
  //
  if (LocaleFunction != null)
  {
    LocaleFunction(this);
  }
}

function  WWHJavaScriptMessages_Set_de(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "Daten werden geladen...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "Inhalt";
  ParamMessages.mTabsIndexLabel     = "Index";
  ParamMessages.mTabsSearchLabel    = "Suchen";
  ParamMessages.mTabsFavoritesLabel = "Favoriten";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "Die aktuelle Seite wurde nicht im Inhalt gefunden.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "Das von Ihnen gew\u00e4hlte Indexwort bzw. der gew\u00e4hlte Indexbegriff tritt in mehreren Dokumenten auf.";
  ParamMessages.mIndexSelectMessage2 = "W\u00e4hlen Sie ein Dokument aus.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "Suchen";
  ParamMessages.mSearchScopeAllLabel       = "Alle verf\u00fcgbaren B\u00fccher";
  ParamMessages.mSearchDefaultMessage      = "Geben Sie das Wort bzw. die Worte ein, nach denen gesucht werden soll:";
  ParamMessages.mSearchSearchingMessage    = "(Suchen)";
  ParamMessages.mSearchNothingFoundMessage = "(keine Ergebnisse)";
  ParamMessages.mSearchRankLabel           = "Rang";
  ParamMessages.mSearchTitleLabel          = "Titel";
  ParamMessages.mSearchBookLabel           = "Buch";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "Hinzuf\u00fcgen";
  ParamMessages.mFavoritesRemoveButtonLabel  = "Entfernen";
  ParamMessages.mFavoritesDisplayButtonLabel = "Anzeigen";
  ParamMessages.mFavoritesCurrentPageLabel   = "Aktuelle Seite:";
  ParamMessages.mFavoritesListLabel          = "Seiten:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "%s ausw\u00e4hlen";
  ParamMessages.mAccessibilityNavigationFrameName = "%s-Navigation";
  ParamMessages.mAccessibilityActiveTab           = "Registerkarte '%s' aktiv";
  ParamMessages.mAccessibilityInactiveTab         = "Zu Registerkarte '%s' umschalten";
  ParamMessages.mAccessibilityTOCBookExpanded     = "Buch '%s' erweitert";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "Buch '%s' zusammegezogen";
  ParamMessages.mAccessibilityTOCTopic            = "Thema '%s'";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s von %s";
  ParamMessages.mAccessibilityIndexEntry          = "Thema '%s', Buch '%s'";
  ParamMessages.mAccessibilityIndexSecondEntry    = "Thema '%s', Buch '%s', Link '%s'";
}

function  WWHJavaScriptMessages_Set_en(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "Loading data...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "Contents";
  ParamMessages.mTabsIndexLabel     = "Index";
  ParamMessages.mTabsSearchLabel    = "Search";
  ParamMessages.mTabsFavoritesLabel = "Favorites";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "The current page could not be found in the table of contents.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "The index word or phrase you chose occurs in multiple documents.";
  ParamMessages.mIndexSelectMessage2 = "Please choose one.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "Go!";
  ParamMessages.mSearchScopeAllLabel       = "All Available Books";
  ParamMessages.mSearchDefaultMessage      = "Type in the word(s) to search for:";
  ParamMessages.mSearchSearchingMessage    = "(searching)";
  ParamMessages.mSearchNothingFoundMessage = "(no results)";
  ParamMessages.mSearchRankLabel           = "Rank";
  ParamMessages.mSearchTitleLabel          = "Title";
  ParamMessages.mSearchBookLabel           = "Book";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "Add";
  ParamMessages.mFavoritesRemoveButtonLabel  = "Remove";
  ParamMessages.mFavoritesDisplayButtonLabel = "Display";
  ParamMessages.mFavoritesCurrentPageLabel   = "Current page:";
  ParamMessages.mFavoritesListLabel          = "Pages:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "Select %s";
  ParamMessages.mAccessibilityNavigationFrameName = "%s navigation";
  ParamMessages.mAccessibilityActiveTab           = "%s tab is active";
  ParamMessages.mAccessibilityInactiveTab         = "Switch to %s tab";
  ParamMessages.mAccessibilityTOCBookExpanded     = "Book %s expanded";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "Book %s collapsed";
  ParamMessages.mAccessibilityTOCTopic            = "Topic %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s of %s";
  ParamMessages.mAccessibilityIndexEntry          = "Topic %s of Book %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "Topic %s of Book %s link %s";
}

function  WWHJavaScriptMessages_Set_es(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "Cargando datos...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "Contenido";
  ParamMessages.mTabsIndexLabel     = "\u00cdndice";
  ParamMessages.mTabsSearchLabel    = "Buscar";
  ParamMessages.mTabsFavoritesLabel = "Favoritos";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "Esta p\u00e1gina no se encontr\u00f3 en el \u00edndice de contenidos.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "La frase o palabra del \u00edndice elegida aparece en varios documentos.";
  ParamMessages.mIndexSelectMessage2 = "Elija uno.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "Ir";
  ParamMessages.mSearchScopeAllLabel       = "Todos los libros disponibles";
  ParamMessages.mSearchDefaultMessage      = "Escriba las palabras que desee buscar:";
  ParamMessages.mSearchSearchingMessage    = "(buscando)";
  ParamMessages.mSearchNothingFoundMessage = "(ning\u00fan resultado)";
  ParamMessages.mSearchRankLabel           = "Clase";
  ParamMessages.mSearchTitleLabel          = "T\u00edtulo";
  ParamMessages.mSearchBookLabel           = "Libro";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "Agregar";
  ParamMessages.mFavoritesRemoveButtonLabel  = "Quitar";
  ParamMessages.mFavoritesDisplayButtonLabel = "Mostrar";
  ParamMessages.mFavoritesCurrentPageLabel   = "P\u00e1gina actual:";
  ParamMessages.mFavoritesListLabel          = "P\u00e1ginas:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "Seleccione %s";
  ParamMessages.mAccessibilityNavigationFrameName = "Navegaci\u00f3n %s";
  ParamMessages.mAccessibilityActiveTab           = "La ficha %s est\u00e1 activa";
  ParamMessages.mAccessibilityInactiveTab         = "Cambie a la ficha %s";
  ParamMessages.mAccessibilityTOCBookExpanded     = "El libro %s est\u00e1 expandido";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "El libro %s est\u00e1 contra\u00eddo";
  ParamMessages.mAccessibilityTOCTopic            = "Tema %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s de %s";
  ParamMessages.mAccessibilityIndexEntry          = "Tema %s del libro %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "Tema %s del libro %s v\u00ednculo %s";
}

function  WWHJavaScriptMessages_Set_fr(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "Chargement des donn\u00e9es...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "Table des mati\u00e8res";
  ParamMessages.mTabsIndexLabel     = "Index";
  ParamMessages.mTabsSearchLabel    = "Rechercher";
  ParamMessages.mTabsFavoritesLabel = "Favoris";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "Page introuvable dans la table des mati\u00e8res.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "Le terme ou l'expression d'index que vous avez choisis apparaissent dans plusieurs documents.";
  ParamMessages.mIndexSelectMessage2 = "S\u00e9lectionnez-en un.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "Lancer";
  ParamMessages.mSearchScopeAllLabel       = "Tous les livres disponibles";
  ParamMessages.mSearchDefaultMessage      = "Saisissez un ou plusieurs mots cl\u00e9s\u00a0:";
  ParamMessages.mSearchSearchingMessage    = "(recherche en cours)";
  ParamMessages.mSearchNothingFoundMessage = "(aucun r\u00e9sultat)";
  ParamMessages.mSearchRankLabel           = "Pertinence";
  ParamMessages.mSearchTitleLabel          = "Titre";
  ParamMessages.mSearchBookLabel           = "Livre";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "Ajouter";
  ParamMessages.mFavoritesRemoveButtonLabel  = "Supprimer";
  ParamMessages.mFavoritesDisplayButtonLabel = "Afficher";
  ParamMessages.mFavoritesCurrentPageLabel   = "Page courante\u00a0:";
  ParamMessages.mFavoritesListLabel          = "Pages\u00a0:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "S\u00e9lectionner %s";
  ParamMessages.mAccessibilityNavigationFrameName = "Navigation %s";
  ParamMessages.mAccessibilityActiveTab           = "L'onglet %s est actif";
  ParamMessages.mAccessibilityInactiveTab         = "Placez-vous sous l'onglet %s";
  ParamMessages.mAccessibilityTOCBookExpanded     = "Livre %s \u00e9tendu";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "Livre %s r\u00e9duit";
  ParamMessages.mAccessibilityTOCTopic            = "Rubrique %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s/%s";
  ParamMessages.mAccessibilityIndexEntry          = "Rubrique %s du livre %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "Rubrique %s du lien %s du livre %s";
}

function  WWHJavaScriptMessages_Set_it(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "Caricamento dati in corso...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "Contenuto";
  ParamMessages.mTabsIndexLabel     = "Indice";
  ParamMessages.mTabsSearchLabel    = "Cerca";
  ParamMessages.mTabsFavoritesLabel = "Preferiti";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "La pagina corrente non \u00e8 stata trovata nel Sommario.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "La parola o la frase cercata nell'indice \u00e8 presente in pi\u00f9 documenti.";
  ParamMessages.mIndexSelectMessage2 = "Sceglierne uno.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "Vai!";
  ParamMessages.mSearchScopeAllLabel       = "Tutti i libri disponibili";
  ParamMessages.mSearchDefaultMessage      = "Digitare le parole da cercare:";
  ParamMessages.mSearchSearchingMessage    = "(ricerca in corso)";
  ParamMessages.mSearchNothingFoundMessage = "(nessun risultato)";
  ParamMessages.mSearchRankLabel           = "Classe";
  ParamMessages.mSearchTitleLabel          = "Titolo";
  ParamMessages.mSearchBookLabel           = "Libro";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "Aggiungi";
  ParamMessages.mFavoritesRemoveButtonLabel  = "Rimuovi";
  ParamMessages.mFavoritesDisplayButtonLabel = "Visualizza";
  ParamMessages.mFavoritesCurrentPageLabel   = "Pagina corrente:";
  ParamMessages.mFavoritesListLabel          = "Pagine:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "Seleziona %s";
  ParamMessages.mAccessibilityNavigationFrameName = "Navigazione %s";
  ParamMessages.mAccessibilityActiveTab           = "La scheda %s \u00e8 attiva";
  ParamMessages.mAccessibilityInactiveTab         = "Passa alla scheda %s";
  ParamMessages.mAccessibilityTOCBookExpanded     = "Libro %s espanso";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "Libro %s compresso";
  ParamMessages.mAccessibilityTOCTopic            = "Argomento %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s di %s";
  ParamMessages.mAccessibilityIndexEntry          = "Argomento %s del libro %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "Argomento %s del libro %s collegamento %s";
}

function  WWHJavaScriptMessages_Set_ja(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "\u30c7\u30fc\u30bf\u3092\u8aad\u307f\u8fbc\u3093\u3067\u3044\u307e\u3059\u3002";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "\u76ee\u6b21";
  ParamMessages.mTabsIndexLabel     = "\u7d22\u5f15";
  ParamMessages.mTabsSearchLabel    = "\u691c\u7d22";
  ParamMessages.mTabsFavoritesLabel = "\u304a\u6c17\u306b\u5165\u308a";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "\u73fe\u5728\u306e\u30da\u30fc\u30b8\u306f\u76ee\u6b21\u5185\u3067\u898b\u3064\u304b\u308a\u307e\u305b\u3093\u3067\u3057\u305f\u3002";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "\u9078\u629e\u3057\u305f\u7d22\u5f15\u306e\u5358\u8a9e\u307e\u305f\u306f\u53e5\u306f\u3001\u8907\u6570\u306e\u30c9\u30ad\u30e5\u30e1\u30f3\u30c8\u5185\u3067\u4f7f\u7528\u3055\u308c\u3066\u3044\u307e\u3059\u3002";
  ParamMessages.mIndexSelectMessage2 = "1 \u3064\u306e\u30c9\u30ad\u30e5\u30e1\u30f3\u30c8\u3092\u9078\u629e\u3057\u3066\u304f\u3060\u3055\u3044\u3002";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "\u79fb\u52d5";
  ParamMessages.mSearchScopeAllLabel       = "\u5229\u7528\u53ef\u80fd\u306a\u3059\u3079\u3066\u306e\u30d6\u30c3\u30af";
  ParamMessages.mSearchDefaultMessage      = "\u691c\u7d22\u5bfe\u8c61\u8a9e:";
  ParamMessages.mSearchSearchingMessage    = "(\u691c\u7d22\u4e2d)";
  ParamMessages.mSearchNothingFoundMessage = "(\u7d50\u679c\u306a\u3057)";
  ParamMessages.mSearchRankLabel           = "\u30e9\u30f3\u30af";
  ParamMessages.mSearchTitleLabel          = "\u30bf\u30a4\u30c8\u30eb";
  ParamMessages.mSearchBookLabel           = "\u30d6\u30c3\u30af";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "\u8ffd\u52a0";
  ParamMessages.mFavoritesRemoveButtonLabel  = "\u524a\u9664";
  ParamMessages.mFavoritesDisplayButtonLabel = "\u8868\u793a";
  ParamMessages.mFavoritesCurrentPageLabel   = "\u73fe\u5728\u306e\u30da\u30fc\u30b8:";
  ParamMessages.mFavoritesListLabel          = "\u30da\u30fc\u30b8:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "%s \u306e\u9078\u629e";
  ParamMessages.mAccessibilityNavigationFrameName = "%s \u30ca\u30d3\u30b2\u30fc\u30b7\u30e7\u30f3";
  ParamMessages.mAccessibilityActiveTab           = "%s \u30bf\u30d6\u304c\u9078\u629e\u3055\u308c\u3066\u3044\u307e\u3059";
  ParamMessages.mAccessibilityInactiveTab         = "%s \u30bf\u30d6\u3078\u306e\u5207\u308a\u66ff\u3048";
  ParamMessages.mAccessibilityTOCBookExpanded     = "JA \u30d6\u30c3\u30af %s \u304c\u5c55\u958b\u8868\u793a\u3055\u308c\u3066\u3044\u307e\u3059";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "JA \u30d6\u30c3\u30af %s \u304c\u6298\u308a\u305f\u305f\u307e\u308c\u3066\u3044\u307e\u3059";
  ParamMessages.mAccessibilityTOCTopic            = "JA \u30c8\u30d4\u30c3\u30af %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "JA %s/%s";
  ParamMessages.mAccessibilityIndexEntry          = "JA \u30c8\u30d4\u30c3\u30af %s/\u30d6\u30c3\u30af %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "JA \u30c8\u30d4\u30c3\u30af %s/\u30d6\u30c3\u30af %s \u30ea\u30f3\u30af %s";
}

function  WWHJavaScriptMessages_Set_ko(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "\ub370\uc774\ud130 \ub85c\ub4dc \uc911...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "\ucee8\ud150\uce20";
  ParamMessages.mTabsIndexLabel     = "\uc0c9\uc778";
  ParamMessages.mTabsSearchLabel    = "\uac80\uc0c9";
  ParamMessages.mTabsFavoritesLabel = "\uad00\uc2ec \ud56d\ubaa9";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "\ubaa9\ucc28\uc5d0\uc11c \ud604\uc7ac \ud398\uc774\uc9c0\ub97c \ucc3e\uc744 \uc218 \uc5c6\uc2b5\ub2c8\ub2e4.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "\uc120\ud0dd\ud55c \uc0c9\uc778 \ub2e8\uc5b4 \ub610\ub294 \uad6c\ubb38\uc774 \uc5ec\ub7ec \ubb38\uc11c\uc5d0 \uc874\uc7ac\ud569\ub2c8\ub2e4.";
  ParamMessages.mIndexSelectMessage2 = "\ud558\ub098\ub97c \uc120\ud0dd\ud558\uc2ed\uc2dc\uc624.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "\uac80\uc0c9";
  ParamMessages.mSearchScopeAllLabel       = "\ubaa8\ub4e0 \ucc45";
  ParamMessages.mSearchDefaultMessage      = "\uac80\uc0c9\uc5b4\ub97c \uc785\ub825\ud558\uc2ed\uc2dc\uc624.";
  ParamMessages.mSearchSearchingMessage    = "(\uac80\uc0c9 \uc911)";
  ParamMessages.mSearchNothingFoundMessage = "(\uacb0\uacfc \uc5c6\uc74c)";
  ParamMessages.mSearchRankLabel           = "\ub4f1\uae09";
  ParamMessages.mSearchTitleLabel          = "\uc81c\ubaa9";
  ParamMessages.mSearchBookLabel           = "\ucc45";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "\ucd94\uac00";
  ParamMessages.mFavoritesRemoveButtonLabel  = "\uc81c\uac70";
  ParamMessages.mFavoritesDisplayButtonLabel = "\ud45c\uc2dc";
  ParamMessages.mFavoritesCurrentPageLabel   = "\ud604\uc7ac \ud398\uc774\uc9c0:";
  ParamMessages.mFavoritesListLabel          = "\ud398\uc774\uc9c0:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "%s \uc120\ud0dd";
  ParamMessages.mAccessibilityNavigationFrameName = "%s \ub124\ube44\uac8c\uc774\uc158";
  ParamMessages.mAccessibilityActiveTab           = "%s \ud0ed \ud65c\uc131";
  ParamMessages.mAccessibilityInactiveTab         = "%s \ud0ed \uc804\ud658";
  ParamMessages.mAccessibilityTOCBookExpanded     = "%s \ucc45 \ud655\uc7a5";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "%s \ucc45 \uc555\ucd95";
  ParamMessages.mAccessibilityTOCTopic            = "%s \ud56d\ubaa9";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s\uc758 %s";
  ParamMessages.mAccessibilityIndexEntry          = "%s \ucc45\uc758 %s \ud56d\ubaa9";
  ParamMessages.mAccessibilityIndexSecondEntry    = "%s \ucc45 %s \ub9c1\ud06c\uc758 %s \ud56d\ubaa9";
}

function  WWHJavaScriptMessages_Set_pt(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "Carregando dados...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "Conte\u00fado";
  ParamMessages.mTabsIndexLabel     = "\u00cdndice remissivo";
  ParamMessages.mTabsSearchLabel    = "Procurar";
  ParamMessages.mTabsFavoritesLabel = "Favoritos";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "A p\u00e1gina atual n\u00e3o p\u00f4de ser encontrada no Sum\u00e1rio.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "A palavra ou frase escolhida no \u00edndice remissivo consta de mais de um documento.";
  ParamMessages.mIndexSelectMessage2 = "Escolha um.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "Prosseguir";
  ParamMessages.mSearchScopeAllLabel       = "Todos os livros dispon\u00edveis";
  ParamMessages.mSearchDefaultMessage      = "Digite a(s) palavra(s) a ser(em) procurada(s):";
  ParamMessages.mSearchSearchingMessage    = "(procurando)";
  ParamMessages.mSearchNothingFoundMessage = "(nenhum resultado)";
  ParamMessages.mSearchRankLabel           = "Escopo";
  ParamMessages.mSearchTitleLabel          = "T\u00edtulo";
  ParamMessages.mSearchBookLabel           = "Livro";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "Adicionar";
  ParamMessages.mFavoritesRemoveButtonLabel  = "Remover";
  ParamMessages.mFavoritesDisplayButtonLabel = "Mostrar";
  ParamMessages.mFavoritesCurrentPageLabel   = "P\u00e1gina atual:";
  ParamMessages.mFavoritesListLabel          = "P\u00e1ginas:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "Selecione %s";
  ParamMessages.mAccessibilityNavigationFrameName = "navega\u00e7\u00e3o %s";
  ParamMessages.mAccessibilityActiveTab           = "A guia %s est\u00e1 ativa";
  ParamMessages.mAccessibilityInactiveTab         = "Alterne para a guia %s";
  ParamMessages.mAccessibilityTOCBookExpanded     = "Livro %s expandido";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "Livro %s recolhido";
  ParamMessages.mAccessibilityTOCTopic            = "T\u00f3pico %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s de %s";
  ParamMessages.mAccessibilityIndexEntry          = "T\u00f3pico %s do livro %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "T\u00f3pico %s do livro %s, link %s";
}

function  WWHJavaScriptMessages_Set_sv(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "L\u00e4ser in data...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "Inneh\u00e5ll";
  ParamMessages.mTabsIndexLabel     = "Index";
  ParamMessages.mTabsSearchLabel    = "S\u00f6k";
  ParamMessages.mTabsFavoritesLabel = "Favoriter";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "Det gick inte att hitta den aktuella sidan i inneh\u00e5llsf\u00f6rteckningen.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "Det indexord eller den fras du valde f\u00f6rekommer i flera dokument.";
  ParamMessages.mIndexSelectMessage2 = "V\u00e4lj ett dokument.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "Visa";
  ParamMessages.mSearchScopeAllLabel       = "Alla tillg\u00e4ngliga b\u00f6cker";
  ParamMessages.mSearchDefaultMessage      = "Ange de ord du vill s\u00f6ka efter:";
  ParamMessages.mSearchSearchingMessage    = "(s\u00f6ker)";
  ParamMessages.mSearchNothingFoundMessage = "(inga resultat)";
  ParamMessages.mSearchRankLabel           = "Relevans";
  ParamMessages.mSearchTitleLabel          = "Rubrik";
  ParamMessages.mSearchBookLabel           = "Bok";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "L\u00e4gg till";
  ParamMessages.mFavoritesRemoveButtonLabel  = "Ta bort";
  ParamMessages.mFavoritesDisplayButtonLabel = "Visa";
  ParamMessages.mFavoritesCurrentPageLabel   = "Aktuell sida:";
  ParamMessages.mFavoritesListLabel          = "Sidor:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "V\u00e4lj %s";
  ParamMessages.mAccessibilityNavigationFrameName = "%s-navigering";
  ParamMessages.mAccessibilityActiveTab           = "Fliken %s \u00e4r aktiv";
  ParamMessages.mAccessibilityInactiveTab         = "V\u00e4xla till fliken %s";
  ParamMessages.mAccessibilityTOCBookExpanded     = "Boken %s maximerades";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "Boken %s minimerades";
  ParamMessages.mAccessibilityTOCTopic            = "Avsnitt %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s av %s";
  ParamMessages.mAccessibilityIndexEntry          = "Avsnitt %s i boken %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "Avsnitt %s i boken %s, l\u00e4nk %s";
}

function  WWHJavaScriptMessages_Set_zh(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "\u6b63\u5728\u52a0\u8f7d\u6570\u636e...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "\u76ee\u5f55";
  ParamMessages.mTabsIndexLabel     = "\u7d22\u5f15";
  ParamMessages.mTabsSearchLabel    = "\u641c\u7d22";
  ParamMessages.mTabsFavoritesLabel = "\u6536\u85cf\u5939";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "\u76ee\u5f55\u4e2d\u627e\u4e0d\u5230\u5f53\u524d\u9875\u3002";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "\u6240\u9009\u7684\u7d22\u5f15\u5b57\u8bcd\u51fa\u73b0\u5728\u591a\u4e2a\u6587\u6863\u4e2d\u3002";
  ParamMessages.mIndexSelectMessage2 = "\u8bf7\u9009\u62e9\u4e00\u4e2a\u3002";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "\u5f00\u59cb\uff01";
  ParamMessages.mSearchScopeAllLabel       = "\u6240\u6709\u4e66\u7c4d";
  ParamMessages.mSearchDefaultMessage      = "\u952e\u5165\u8981\u641c\u7d22\u7684\u5355\u8bcd\uff1a";
  ParamMessages.mSearchSearchingMessage    = "\uff08\u641c\u7d22\uff09";
  ParamMessages.mSearchNothingFoundMessage = "\uff08\u65e0\u7ed3\u679c\uff09";
  ParamMessages.mSearchRankLabel           = "\u7ea7\u522b";
  ParamMessages.mSearchTitleLabel          = "\u6807\u9898";
  ParamMessages.mSearchBookLabel           = "\u4e66\u7c4d";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "\u6dfb\u52a0";
  ParamMessages.mFavoritesRemoveButtonLabel  = "\u5220\u9664";
  ParamMessages.mFavoritesDisplayButtonLabel = "\u663e\u793a";
  ParamMessages.mFavoritesCurrentPageLabel   = "\u5f53\u524d\u9875\uff1a";
  ParamMessages.mFavoritesListLabel          = "\u9875\uff1a";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "\u9009\u62e9 %s";
  ParamMessages.mAccessibilityNavigationFrameName = "%s \u5bfc\u822a";
  ParamMessages.mAccessibilityActiveTab           = "%s \u6807\u7b7e\u5df2\u6fc0\u6d3b";
  ParamMessages.mAccessibilityInactiveTab         = "\u5207\u6362\u81f3 %s \u6807\u7b7e";
  ParamMessages.mAccessibilityTOCBookExpanded     = "\u5df2\u5c55\u5f00 %s \u4e66";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "\u5df2\u6298\u53e0 %s \u4e66";
  ParamMessages.mAccessibilityTOCTopic            = "%s \u4e3b\u9898";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "\u7b2c %s\uff0c\u5171 %s";
  ParamMessages.mAccessibilityIndexEntry          = "%s \u4e66\u4e2d\u7684 %s \u4e3b\u9898";
  ParamMessages.mAccessibilityIndexSecondEntry    = "%s \u4e66\u7684 %s \u94fe\u63a5\u4e2d\u7684 %s \u4e3b\u9898";
}

function  WWHJavaScriptMessages_Set_zh_tw(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "\u8f09\u5165\u8cc7\u6599...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "\u76ee\u9304";
  ParamMessages.mTabsIndexLabel     = "\u7d22\u5f15";
  ParamMessages.mTabsSearchLabel    = "\u641c\u5c0b";
  ParamMessages.mTabsFavoritesLabel = "\u6700\u611b";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "\u76ee\u9304\u4e2d\u627e\u4e0d\u5230\u76ee\u524d\u7db2\u9801\u3002";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "\u60a8\u9078\u64c7\u7684\u6587\u5b57\u6216\u8a5e\u5f59\u51fa\u73fe\u5728\u591a\u500b\u6587\u4ef6\u4e2d\u3002";
  ParamMessages.mIndexSelectMessage2 = "\u8acb\u9078\u64c7\u5176\u4e2d\u4e00\u9805\u3002";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "\u958b\u59cb\uff01";
  ParamMessages.mSearchScopeAllLabel       = "\u6240\u6709\u53ef\u7528\u66f8\u7c4d";
  ParamMessages.mSearchDefaultMessage      = "\u8f38\u5165\u8981\u641c\u5c0b\u6587\u5b57\uff1a";
  ParamMessages.mSearchSearchingMessage    = "(\u641c\u5c0b\u4e2d)";
  ParamMessages.mSearchNothingFoundMessage = "(\u6c92\u6709\u7d50\u679c)";
  ParamMessages.mSearchRankLabel           = "\u95dc\u806f\u6027";
  ParamMessages.mSearchTitleLabel          = "\u6a19\u984c";
  ParamMessages.mSearchBookLabel           = "\u66f8\u540d";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "\u65b0\u589e";
  ParamMessages.mFavoritesRemoveButtonLabel  = "\u79fb\u9664";
  ParamMessages.mFavoritesDisplayButtonLabel = "\u986f\u793a";
  ParamMessages.mFavoritesCurrentPageLabel   = "\u76ee\u524d\u7db2\u9801\uff1a";
  ParamMessages.mFavoritesListLabel          = "\u7db2\u9801\uff1a";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "\u9078\u64c7 %s";
  ParamMessages.mAccessibilityNavigationFrameName = "%s \u700f\u89bd";
  ParamMessages.mAccessibilityActiveTab           = "%s \u6a19\u7c64\u5df2\u555f\u7528";
  ParamMessages.mAccessibilityInactiveTab         = "\u5207\u63db\u5230 %s \u6a19\u7c64";
  ParamMessages.mAccessibilityTOCBookExpanded     = "\u66f8\u7c4d %s \u5df2\u5c55\u958b";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "\u66f8\u7c4d %s \u5df2\u6298\u758a";
  ParamMessages.mAccessibilityTOCTopic            = "\u4e3b\u984c %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s / %s";
  ParamMessages.mAccessibilityIndexEntry          = "\u4e3b\u984c %s\uff0c\u66f8\u7c4d %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "\u4e3b\u984c %s\uff0c\u66f8\u7c4d %s \u9023\u7d50 %s";
}

function  WWHJavaScriptMessages_Set_ru(ParamMessages)
{
  // Initialization Messages
  //
  ParamMessages.mInitializingMessage = "\u0412\u044b\u043f\u043e\u043b\u043d\u044f\u0435\u0442\u0441\u044f \u0437\u0430\u0433\u0440\u0443\u0437\u043a\u0430 \u0434\u0430\u043d\u043d\u044b\u0445...";

  // Tab Labels
  //
  ParamMessages.mTabsTOCLabel       = "\u0421\u043e\u0434\u0435\u0440\u0436\u0430\u043d\u0438\u0435";
  ParamMessages.mTabsIndexLabel     = "\u0423\u043a\u0430\u0437\u0430\u0442\u0435\u043b\u044c";
  ParamMessages.mTabsSearchLabel    = "\u041f\u043e\u0438\u0441\u043a";
  ParamMessages.mTabsFavoritesLabel = "\u0418\u0437\u0431\u0440\u0430\u043d\u043d\u043e\u0435";

  // TOC Messages
  //
  ParamMessages.mTOCFileNotFoundMessage = "\u041d\u0435\u0432\u043e\u0437\u043c\u043e\u0436\u043d\u043e \u043d\u0430\u0439\u0442\u0438 \u0432 \u043e\u0433\u043b\u0430\u0432\u043b\u0435\u043d\u0438\u0438 \u0442\u0435\u043a\u0443\u0449\u0443\u044e \u0441\u0442\u0440\u0430\u043d\u0438\u0446\u0443.";

  // Index Messages
  //
  ParamMessages.mIndexSelectMessage1 = "\u0412\u044b\u0431\u0440\u0430\u043d\u043d\u043e\u0435 \u0441\u043b\u043e\u0432\u043e (\u0444\u0440\u0430\u0437\u0430) \u0438\u0437 \u0443\u043a\u0430\u0437\u0430\u0442\u0435\u043b\u044f \u0441\u043e\u0434\u0435\u0440\u0436\u0438\u0442\u0441\u044f \u0432 \u043d\u0435\u0441\u043a\u043e\u043b\u044c\u043a\u0438\u0445 \u0434\u043e\u043a\u0443\u043c\u0435\u043d\u0442\u0430\u0445.";
  ParamMessages.mIndexSelectMessage2 = "\u0412\u044b\u0431\u0435\u0440\u0438\u0442\u0435 \u043e\u0434\u043d\u0438 \u0434\u043e\u043a\u0443\u043c\u0435\u043d\u0442.";

  // Search Messages
  //
  ParamMessages.mSearchButtonLabel         = "\u041d\u0430\u0447\u0430\u0442\u044c";
  ParamMessages.mSearchScopeAllLabel       = "\u0412\u0441\u0435 \u0434\u043e\u0441\u0442\u0443\u043f\u043d\u044b\u0435 \u0442\u043e\u043c\u0430";
  ParamMessages.mSearchDefaultMessage      = "\u0412\u0432\u0435\u0434\u0438\u0442\u0435 \u0441\u043b\u043e\u0432\u043e (\u0441\u043b\u043e\u0432\u0430) \u0434\u043b\u044f \u043f\u043e\u0438\u0441\u043a\u0430:";
  ParamMessages.mSearchSearchingMessage    = "(\u043f\u043e\u0438\u0441\u043a)";
  ParamMessages.mSearchNothingFoundMessage = "(\u043d\u0435\u0442 \u0440\u0435\u0437\u0443\u043b\u044c\u0442\u0430\u0442\u043e\u0432)";
  ParamMessages.mSearchRankLabel           = "\u0420\u0430\u043d\u0433";
  ParamMessages.mSearchTitleLabel          = "\u0417\u0430\u0433\u043e\u043b\u043e\u0432\u043e\u043a";
  ParamMessages.mSearchBookLabel           = "\u0422\u043e\u043c";

  // Favorites Messages
  //
  ParamMessages.mFavoritesAddButtonLabel     = "\u0414\u043e\u0431\u0430\u0432\u0438\u0442\u044c";
  ParamMessages.mFavoritesRemoveButtonLabel  = "\u0423\u0434\u0430\u043b\u0438\u0442\u044c";
  ParamMessages.mFavoritesDisplayButtonLabel = "\u041f\u043e\u043a\u0430\u0437\u0430\u0442\u044c";
  ParamMessages.mFavoritesCurrentPageLabel   = "\u0422\u0435\u043a\u0443\u0449\u0430\u044f \u0441\u0442\u0440\u0430\u043d\u0438\u0446\u0430:";
  ParamMessages.mFavoritesListLabel          = "\u0421\u0442\u0440\u0430\u043d\u0438\u0446\u044b:";

  // Accessibility Messages
  //
  ParamMessages.mAccessibilityTabsFrameName       = "\u0412\u044b\u0431\u0440\u0430\u0442\u044c %s";
  ParamMessages.mAccessibilityNavigationFrameName = "\u041d\u0430\u0432\u0438\u0433\u0430\u0446\u0438\u044f %s";
  ParamMessages.mAccessibilityActiveTab           = "\u0412\u043a\u043b\u0430\u0434\u043a\u0430 %s \u0430\u043a\u0442\u0438\u0432\u043d\u0430";
  ParamMessages.mAccessibilityInactiveTab         = "\u041f\u0435\u0440\u0435\u0439\u0442\u0438 \u043d\u0430 \u0432\u043a\u043b\u0430\u0434\u043a\u0443 %s";
  ParamMessages.mAccessibilityTOCBookExpanded     = "\u0422\u043e\u043c %s \u0440\u0430\u0437\u0432\u0435\u0440\u043d\u0443\u0442";
  ParamMessages.mAccessibilityTOCBookCollapsed    = "\u0422\u043e\u043c %s \u0441\u0432\u0435\u0440\u043d\u0443\u0442";
  ParamMessages.mAccessibilityTOCTopic            = "\u0420\u0430\u0437\u0434\u0435\u043b %s";
  ParamMessages.mAccessibilityTOCOneOfTotal       = "%s \u0438\u0437 %s";
  ParamMessages.mAccessibilityIndexEntry          = "\u0420\u0430\u0437\u0434\u0435\u043b %s \u0432 \u0442\u043e\u043c\u0435 %s";
  ParamMessages.mAccessibilityIndexSecondEntry    = "\u0421\u0441\u044b\u043b\u043a\u043a\u0430 %s \u043d\u0430 \u0440\u0430\u0437\u0434\u0435\u043b %s \u0432 \u0442\u043e\u043c\u0435 %";
}
