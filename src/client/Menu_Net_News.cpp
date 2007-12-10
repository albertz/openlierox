/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - News
// Created 12/8/02
// Jason Boettcher


#include "LieroX.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "CButton.h"
#include "CBrowser.h"
#include "CMediaPlayer.h"
#include "CListview.h"
#include "HTTP.h"
#include "StringUtils.h"


// Here goes old code not used anywhere passed down from JasonB

static CGuiLayout	cNews;

enum {
	nw_Back = 0,
	nw_NewsBrowser
};


///////////////////
// Initialize the news net menu
int Menu_Net_NewsInitialize(void)
{
	iNetMode = net_main;

	// Setup the gui layout
	cNews.Shutdown();
	cNews.Initialize();

	cNews.Add( new CButton(BUT_BACK, tMenu->bmpButtons), nw_Back, 25,440, 50,15);
	cNews.Add( new CBrowser(), nw_NewsBrowser, 50, 160, 540, 260);


	// Load the news
	CBrowser *b = (CBrowser *)cNews.getWidget(nw_NewsBrowser);
	b->Load("news/news.txt");
 


	
	

	return true;
}


///////////////////
// The net news menu frame
void Menu_Net_NewsFrame(int mouse)
{
	gui_event_t *ev = NULL;


	// Process & Draw the gui
#ifdef WITH_MEDIAPLAYER
	if (!cMediaPlayer.GetDrawPlayer())
#endif
		ev = cNews.Process();
	cNews.Draw( tMenu->bmpScreen );


	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case nw_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					cNews.Shutdown();

					// Back to main menu					
					Menu_MainInitialize();
				}
				break;
		}

	}


	// Draw the mouse
	DrawCursor(tMenu->bmpScreen);
}

// News implementation using GUI skins and listbox - what else could we use? 
// CBrowser class passed down from JasonB isn't used anywhere and contains really old code.

static std::string sNewsText = "Loading news...\n";	// Last newline required
static CHttp* cNewsDownload = NULL; // HINT: cannot use type here because constructor depends on uninited things (sHttpError)
enum { NEWS_NONE=0, NEWS_LXALLIANCE, NEWS_SOURCEFORGE, NEWS_END = NEWS_SOURCEFORGE + 20 };
static int iNewsDownloaded = NEWS_NONE;
static int iNewsSourceforgeLastRevision = 0;

static void NewsListview_Update( const std::string & param, CWidget * source )
{

	if( iNewsDownloaded >= NEWS_END )
	{
		CGuiSkin::DeRegisterUpdateCallback( source );
	}
	else if( iNewsDownloaded == NEWS_NONE )
	{
		iNewsDownloaded = NEWS_LXALLIANCE;
		cNewsDownload->RequestData( "http://lxalliance.net/smf/index.php?page=5" );
	}
	else
	{
		if( cNewsDownload->ProcessRequest() == HTTP_PROC_PROCESSING )
			return;	// No need to update listview
		if( cNewsDownload->ProcessRequest() == HTTP_PROC_ERROR )
		{
			iNewsDownloaded = NEWS_END;
			sNewsText += "Error loading news.\n";
		};
		if( cNewsDownload->ProcessRequest() == HTTP_PROC_FINISHED )
		{
			std::string data = cNewsDownload->GetData();
			if( iNewsDownloaded == NEWS_LXALLIANCE )
			{
				sNewsText = "---------- News from LXAlliance.net:\n";
				data = StripHtmlTags( data );
				// TODO: Uhm, this code kinda bulky, maybe use regexp or something
				// Skip to required position
				if( data.find("Welcome, Guest. Please login or register.") != std::string::npos )
					data = data.substr( data.find("Welcome, Guest. Please login or register.") + 
													fix_strnlen( "Welcome, Guest. Please login or register." ) );
				if( data.find("Welcome, Guest. Please login or register.") != std::string::npos )
					data = data.substr( data.find("Welcome, Guest. Please login or register.") + 
													fix_strnlen( "Welcome, Guest. Please login or register." ) );
				if( data.find("Login with username,") != std::string::npos )
					data = data.substr( data.find("Login with username,") + fix_strnlen("Login with username,") );
				if( data.find("\n\n\n\n\n") != std::string::npos )
					data = data.substr( data.find("\n\n\n\n\n") + fix_strnlen("\n\n\n\n\n") );
				// Strip extra data at the end
				if( data.find("\n\n\n\n\n") != std::string::npos )
					data = data.substr( 0, data.find("\n\n\n\n\n") );
				TrimSpaces( data );

				for(	std::string::size_type f = 0; f < data.size() && f != std::string::npos; )
				{
					while( data[f] == '\n' ) f++;
					std::string topic = data.substr( f, data.find( "\n", f )-f );
					TrimSpaces( topic );
					f = data.find( "\n", f ) + 1;
					while( data[f] == '\n' ) f++;
					std::string author = data.substr( f, data.find( "\n", f )-f );
					TrimSpaces( author );
					f = data.find( "\n", f ) + 1;
					while( data[f] == '\n' ) f++;
					std::string text = data.substr( f, data.find( "\n\n", f )-f );
					TrimSpaces( text );
					sNewsText += "- " + topic + "\n" + author + "\n" + text + "\n";
					f = data.find( "\n\n\n" , f );
					if( f != std::string::npos )
						while( data[f] == '\n' ) f++;
				};
				
				iNewsDownloaded = NEWS_SOURCEFORGE;
				cNewsDownload->RequestData( "http://openlierox.svn.sourceforge.net/viewvc/openlierox?view=rev" );
			}
			else if( iNewsDownloaded >= NEWS_SOURCEFORGE )
			{
				data = StripHtmlTags( data );
				// TODO: Another bulky code
				int rev = 1000;
				if( data.find( "Revision " ) != std::string::npos )
					rev = atoi( data.substr( data.find("Revision ") + fix_strnlen("Revision ") ) );
				if( iNewsDownloaded == NEWS_SOURCEFORGE )
				{
					sNewsText += "---------- Last 20 changes from SourceForge.net:\n";
					iNewsSourceforgeLastRevision = rev;
				}
				std::string author = "?";
				if( data.find( "Author:\n" ) != std::string::npos )
				{
					author = data.substr( data.find("Author:\n") + fix_strnlen("Author:\n") );
					author = author.substr( 0, author.find("\n") );
				};
				std::string date = "?";
				if( data.find( "Date:\n" ) != std::string::npos )
				{
					date = data.substr( data.find("Date:\n") + fix_strnlen("Date:\n") );
					date = date.substr( 0, date.find("\n") );
					if( date.find("(") != std::string::npos )
					{
						date = date.substr( date.find("(") + 1 );
						date = date.substr( 0, date.find(")") );
					};
				};
				std::string message = "";
				if( data.find( "Log Message:\n" ) != std::string::npos )
				{
					message = data.substr( data.find("Log Message:\n") + fix_strnlen("Log Message:\n") );
					message = message.substr( 0, message.find("\n\n") );
				};
				sNewsText += "- r" + itoa(iNewsSourceforgeLastRevision) + " by " + author + " " + date + ":\n";
				if( message != "" )
					sNewsText += message + "\n";
				iNewsSourceforgeLastRevision --;
				cNewsDownload->RequestData( "http://openlierox.svn.sourceforge.net/viewvc/openlierox?view=rev&revision=" + 
												itoa(iNewsSourceforgeLastRevision) );
				iNewsDownloaded ++;
			};
			if( iNewsDownloaded >= NEWS_END )
			{
				cNewsDownload->CancelProcessing();
				printf( "News downloading finished\n" );
				CGuiSkin::DeRegisterUpdateCallback( source );
			};
		};
	};
	// Update listview
	CListview * news = (CListview *) source;
	news->SaveScrollbarPos();
	news->Clear();
	int itemCount = 0;
	Uint32 color = StrToCol(param);
	for( 	std::string::size_type f = 0, f1 = sNewsText.find( "\n" ) ; 
			f < sNewsText.size() && f1 != std::string::npos ; 
			f = f1 + 1, f1 = sNewsText.find( "\n", f ) )
	{
		std::vector<std::string> lines = splitstring( sNewsText.substr( f, f1-f ), 4096, news->getWidth() - 20, tLX->cFont );
		for( unsigned it = 0; it < lines.size(); it++ )
		{
			news->AddItem( "", itemCount++, color );
			news->AddSubitem( LVS_TEXT, lines[it], NULL, NULL );
		};
	};
	news->RestoreScrollbarPos();
};

void NewsListview_Init( const std::string & param, CWidget * source )
{
	if( source->getType() != wid_Listview )
		return;
	if( ! cNewsDownload )
		cNewsDownload = new CHttp();
	CGuiSkin::RegisterUpdateCallback( & NewsListview_Update, param, source );
};

static bool NewsListview_Registered = CGuiSkin::RegisterVars("GUI")
		( & NewsListview_Init , "NewsListview_Init" );

class t_cNewsDownload_delete
{
	public:
	~t_cNewsDownload_delete()
	{
		if( cNewsDownload )
			delete cNewsDownload;
		cNewsDownload = NULL;
	};
};

t_cNewsDownload_delete cNewsDownload_delete();

