/////////////////////////////////////////////////////////
//                                                     //
// Tento soubor byl vygenerovan automaticky programem  //
// PalmOSgen. Mista oznacena komentarem /* ... */ jsou //
// urcena k editaci uzivatelem.                        //
//                                                     //
/////////////////////////////////////////////////////////

#define ERROR_CHECKING ERROR_CHECKING_FULL

#include <PalmOS.h>
#include <stdarg.h>

#include "resource.rh"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// dulezite definice

#define CREATOR_ID 'THdd'
#define APP_VERSION 0.1 /* <-- VERZE VASI APLIKACE PRO PREFERENCE */
#define MAIN_DB_TYPE 'THdd' /* <-- 4 PISMENNY KOD VASI HLAVNI DATABASE */
#define MAIN_DB_NAME "THdd-DayByDay" /* <-- JMENO VASI HLAVNI DATABAZE */


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// prototypy funkci


// Prototypy standardnich funkci tvoricich smycku zpracovani udalosti

static Err StartApplication();
static void StopApplication();
static void EventLoop();
static Boolean ApplicationHandleEvent(EventPtr event);

// Standardne pouzivane aplikacni funkce

static MemPtr Id2Ptr(UInt16 id);

// Funkce pro praci s databazemi

static DmOpenRef DbOpenOrCreate(UInt32 creatorId, UInt32 dbType, Char* dbName);
static Int16 DbMainCompareH(MemPtr rec1, MemPtr rec2, Int16 other, SortRecordInfoPtr rec1SortInfo, SortRecordInfoPtr rec2SortInfo, MemHandle appInfoH);

// Prototypy funkci pro zpracovani udalosti formularu

static Boolean FormMAINEventH(EventPtr event);
static Boolean FormRECEventH(EventPtr event);

// Table handling functions
static void TableMainTableDataInit();
static void TabulkaHlavniDataTableUpdate();
static Boolean TabulkaHlavniDataTableSelectH(EventPtr event);
static void TblMainTableDrawCell(MemPtr table, Int16 row, Int16 column, RectangleType* bounds);


// Prototypy pomocnych funkci

void SetListSelection(ListPtr list, ControlPtr dropdown, UInt16 selection);
static void ShowDialog(UInt16 formId);
void EmulatorLog(Char* format, ...);


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// datove typy


// Definice datoveho typu pro uschovani nastaveni programu

typedef struct
{
	/*ZDE DOPLNTE DEFINICI DATOVE STRUKTURY PRO ULOZENI NASTAVENI*/
} PreferencesDataType;

// Definice typu zaznamu database

typedef struct
{
	/*ZDE DOPLNTE DEFINICI DATOVE STRUKTURY ZAZNAMU*/
} DbRecordType;


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// globalni promenne

PreferencesDataType gPreferences; // ulozeni nastaveni programu
DmOpenRef gDbMain = NULL; // ulozeni ovladace hlavni databaze aplikace


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// definice funkci

/////////////////////////////////////////////////////////
//
// Funkce PilotMain() je vstupnim bodem aplikace
//
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error;

	if(cmd == sysAppLaunchCmdNormalLaunch)
	{
		error = StartApplication();
		if(error) return error;
		EventLoop();
		StopApplication();
	}

	return 0;
}

/////////////////////////////////////////////////////////
//
// Funkce StartApplication() je volana pri standardnim spusteni aplikace
//
static Err StartApplication()
{
	UInt16 prefSize;
	Int16 version;


	prefSize = sizeof(PreferencesDataType);
	version = PrefGetAppPreferences(CREATOR_ID, 0, &gPreferences, &prefSize, true);

	if(version != APP_VERSION || prefSize != sizeof(PreferencesDataType))
	{
		MemSet(&gPreferences, sizeof(PreferencesDataType), 0);

		/* NASTAVENI NEBYLO NAHRANO - NUTNA INICIALIZACE */
	}



	gDbMain = DbOpenOrCreate(CREATOR_ID, MAIN_DB_TYPE, MAIN_DB_NAME);


	FrmGotoForm(frmMAIN);

	return false;
}

/////////////////////////////////////////////////////////
//
// Funkce StopApplication() je volana jako posledni funkce
//
static void StopApplication()
{
	if(gDbMain)
	{
		DmCloseDatabase(gDbMain);
	}
	PrefSetAppPreferences(CREATOR_ID, 0, APP_VERSION, &gPreferences, sizeof(PreferencesDataType), true);
	FrmCloseAllForms();
}

/////////////////////////////////////////////////////////
//
// Smycka zpracovani udalosti
//
static void EventLoop()
{
	EventType event;

	do
	{
		EvtGetEvent(&event, evtWaitForever);
		if (SysHandleEvent(&event)) continue;
		if(ApplicationHandleEvent(&event)) continue;
		FrmDispatchEvent(&event);
	}
	while(event.eType != appStopEvent);
}

/////////////////////////////////////////////////////////
//
// Zpracovani udalosti aplikaci - hlavne vymena formularu
//
static Boolean ApplicationHandleEvent(EventPtr event)
{
	FormPtr form;
	Boolean handled = false;

	if(event->eType == frmLoadEvent)
	{
		form = FrmInitForm(event->data.frmLoad.formID);
		FrmSetActiveForm(form);

		switch(event->data.frmLoad.formID)
		{
			case frmMAIN:
				FrmSetEventHandler(form,FormMAINEventH);
				handled = true;
				break;

			case frmREC:
				FrmSetEventHandler(form,FormRECEventH);
				handled = true;
				break;

			default:
				ErrFatalDisplay("Neplatny formular");
				break;
		}
	}

	return handled;
}

/////////////////////////////////////////////////////////
//
// Prevede jednoznacne ID objektu na ukazatel
//
static MemPtr Id2Ptr(UInt16 id)
{
	FormPtr form = FrmGetActiveForm();
	MemPtr object;

	ErrNonFatalDisplayIf(!form, "Neexistujici formular");
	object = FrmGetObjectPtr(form, FrmGetObjectIndex(form, id));
	ErrNonFatalDisplayIf(!object, "Neexistujici objekt");
	return object;
}

/////////////////////////////////////////////////////////
//
// Funkce, ktera inicializuje databazi na zacatku programu
//
static DmOpenRef DbOpenOrCreate(UInt32 creatorId, UInt32 dbType, Char* dbName)
{
	DmOpenRef gDB = NULL;
	Err error;

	gDB = DmOpenDatabaseByTypeCreator(dbType, creatorId, dmModeReadWrite);

	if(!gDB)
	{
		error = DmCreateDatabase(0, dbName, creatorId, dbType, false);

		if(!error)
		{
			gDB = DmOpenDatabaseByTypeCreator(dbType, creatorId, dmModeReadWrite);
		}
	}
	return gDB;
}

/////////////////////////////////////////////////////////
//
// Tridici funkce pro hlavni databazi
//
static Int16 DbMainCompareH(MemPtr rec1, MemPtr rec2, Int16 other, SortRecordInfoPtr rec1SortInfo, SortRecordInfoPtr rec2SortInfo, MemHandle appInfoH)
{
	DbRecordType* record1 = (DbRecordType*)rec1;
	DbRecordType* record2 = (DbRecordType*)rec2;

	return 0; /* <-- VYSLEDEK POROVNANI */
	/* VRATTE HODNOTU -1 JE-LI *record1 MENSI NEZ *record2 A +1 JE-LI VETSI */
}

/////////////////////////////////////////////////////////
//
// ShowDialog(): Zobrazi na displeji "nad" aktivnim dialogem jiny dialog a ceka na stisknuti tlacitka
//
static void ShowDialog(UInt16 formId)
{
	FormPtr form = FrmInitForm(formId);

	if(form)
	{
		FrmDoDialog(form);
		FrmDeleteForm(form);
	}
}

/////////////////////////////////////////////////////////
//
// EmulatorLog(): Pri spusteni v emulatoru zapisuje ladici hlasky do souboru hlaseni
//
void EmulatorLog(Char* format, ...)
{
	va_list args;
	UInt32 dummy, romVersion;
	Char s[100];

	va_start(args, format);
	StrVPrintF(s, format, args);
	va_end(args);

	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

	if(romVersion >= sysMakeROMVersion(3,0,0,sysROMStageRelease,0) && FtrGet('pose', 0, &dummy) == 0)
	{
		HostFPutS(s, HostLogFile());
	}
}




// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// definice funkci pro formulare

/////////////////////////////////////////////////////////
//
// Funkce pro zpracovani udalosti formulare
//
static Boolean FormRECEventH(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;

	switch(event->eType)
	{
		case frmOpenEvent:
			form = FrmGetActiveForm();
			FrmDrawForm(form);
			handled = true;
			break;
	}
	return handled;
}


static Boolean FormMAINEventH(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
	ListPtr list;
	ControlPtr control;

	switch(event->eType)
	{
		case frmOpenEvent:
			form = FrmGetActiveForm();
			FrmDrawForm(form);
			TableMainTableDataInit();
			handled = true;
			break;

		case popSelectEvent:
			control = event->data.popSelect.controlP;
			list = event->data.popSelect.listP;
			SetListSelection(list, control, LstGetSelection(list));
			switch(event->data.popSelect.controlID)
			{
				case CatListHolder:
					
					// ZPRACOVANI VYBERU Z NABIDKY MOZNOSTI
					handled = true;
					break;

				default:
					break;
			}
			break;

		case lstSelectEvent:
			list = event->data.lstSelect.pList;
			switch(event->data.lstSelect.listID)
			{
				case CatList:
					// ZPRACOVANI VYBERU Z TOHOTO SEZNAMU
					handled = true;
					break;

				default:
					break;
			}
			break;

		case tblSelectEvent:
			TabulkaHlavniDataTableSelectH(event);
			break;

		default:
			break;
	}

	return handled;
}


void SetListSelection(ListPtr list, ControlPtr dropdown, UInt16 selection)
{
	if ((selection >= 0) && (selection <= LstGetNumberOfItems(list)))
	{
		CtlSetLabel(dropdown, LstGetSelectionText(list, selection));
	}
	else
		ErrFatalDisplay("Invalid list / selection");


}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// definice funkci pro menu



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// definice funkci pro tabulky

static void TableMainTableDataInit()
{
	TablePtr table = (TablePtr)Id2Ptr(tblMAIN);
	UInt16 numRows = TblGetNumberOfRows(table);
	ListPtr lstCategory = (ListPtr)Id2Ptr(CatList);
	UInt16 row;


	for(row = 0; row < numRows; row++)
	{
		TblSetItemStyle(table, row, 0, dateTableItem);

		TblSetItemStyle(table, row, 1, popupTriggerTableItem);
		TblSetItemPtr(table, row, 1, lstCategory);

		TblSetItemStyle(table, row, 2, customTableItem);
		TblSetCustomDrawProcedure(table,2, TblMainTableDrawCell);

		TblSetItemStyle(table, row, 3, customTableItem);
		TblSetCustomDrawProcedure(table, 3, TblMainTableDrawCell);

		TblSetRowUsable(table, row, true);
		TblMarkRowInvalid(table, row);
	}

	// Nastaveni sloupecku tabulky

	TblSetColumnUsable(table, 0, true);
	TblSetColumnUsable(table, 1, true);
	TblSetColumnUsable(table, 2, true);
	TblSetColumnUsable(table, 3, true);

	TblRedrawTable(table);
}

/////////////////////////////////////////////////////////
//
// Funkce pro aktualizaci dat tabulky
//
static void TabulkaHlavniDataTableUpdate()
{
	TablePtr table = (TablePtr)Id2Ptr(tblMAIN);
	UInt16 numRows = TblGetNumberOfRows(table);
	UInt16 numItems = 100 /* <-- CELKEM RADKU V DATABAZI TREBA DmNumRecords()) */ ;
	Int16 lastPossibleTopItem = (numItems > numRows) ? (numItems - numRows) : 0;
	UInt16 currentItem = 0;
	UInt16 i;

	for(i = 0; i < numRows; i++)
	{
		TblSetRowUsable(table, i, true);
		TblSetRowID(table, i, currentItem);
		TblMarkRowInvalid(table, i);
	}

	TblUnhighlightSelection(table);
	TblRedrawTable(table);
}

/////////////////////////////////////////////////////////
//
// Zpracovani udalosti tabulky
//
static Boolean TabulkaHlavniDataTableSelectH(EventPtr event)
{
	UInt16 row = event->data.tblSelect.row;
	UInt16 column = event->data.tblSelect.column;
	TablePtr table = event->data.tblSelect.pTable;

	DateTimeType today;

	UInt16 iday;
	UInt16 imonth;
	UInt16 iyear;

	UInt32 now = TimGetSeconds();
	TimSecondsToDateTime(now, &today); 
	iday =  (UInt16) today.day;
	imonth = (UInt16) today.month;
	iyear = (UInt16) today.year;
	
	EmulatorLog("Day: %d Month: %d Year: %d\n", iday, imonth, iyear);

	if (column == 0)
	{
		SelectDay(selectDayByDay, &imonth, &iday, &iyear, "Hallo!");
	}	


	if (column == 3)
	{
		FrmGotoForm(frmREC);
	}
	/* ZPRACOVANI VYBERU TETO BUNKY TABULKY */
}

/////////////////////////////////////////////////////////
//
// Prekresleni tabulky
//
static void TblMainTableDrawCell(MemPtr table, Int16 row, Int16 column, RectangleType* bounds)
{
	UInt16 recNum = TblGetRowID((TablePtr)table, row);
	Char *obsah = "XyZZy";
	Char *note = "\011";
	FontID origFont = FntGetFont();


	switch(column)
	{
		case 0:
			/* PREKRESLENI BUNKY V TOMTO SLOUPCI TABULKY */
			WinSetClip(bounds);
			WinDrawChars(obsah, StrLen(obsah), 
							bounds->topLeft.x, bounds->topLeft.y);
			WinResetClip();
			
			break;

		case 1:
			/* PREKRESLENI BUNKY V TOMTO SLOUPCI TABULKY */
			WinSetClip(bounds);
			FntSetFont(boldFont);
			WinDrawChars(obsah, StrLen(obsah), 
							bounds->topLeft.x, bounds->topLeft.y);
			WinResetClip();
			
			break;

		case 2:
			/* PREKRESLENI BUNKY V TOMTO SLOUPCI TABULKY */
			WinSetClip(bounds);
			WinDrawChars(obsah, StrLen(obsah), 
							bounds->topLeft.x, bounds->topLeft.y);
			WinResetClip();
			
			break;

		case 3:
			/* PREKRESLENI BUNKY V TOMTO SLOUPCI TABULKY */
			WinSetClip(bounds);
			FntSetFont(symbolFont);
			WinDrawChars(note, StrLen(note), 
							bounds->topLeft.x, bounds->topLeft.y);
			WinResetClip();
			
			break;

		default:
			break;
	}

	FntSetFont(origFont);

}




// Konec automaticky generovaneho souboru
