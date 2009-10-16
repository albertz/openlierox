/***************************************************************************
 *   Copyright (C) 2005 - 2007 by                                          *
 *      Christian Muehlhaeuser, Last.fm Ltd <chris@last.fm>                *
 *      Erik J�evik, Last.fm Ltd <erik@last.fm>                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include "CrashReporter.h"

#include "MooseCommon.h"
#include "UnicornCommon.h"
#include "LastFmSettings.h"

#include <QTranslator>


int main( int argc, char* argv[] )
{
    // used by some Qt stuff, eg QSettings
    // leave first! As Settings object is created quickly
    QCoreApplication::setApplicationName( "Last.fm" );
    QCoreApplication::setOrganizationName( "Last.fm" );
    QCoreApplication::setOrganizationDomain( "last.fm" );

    QApplication app( argc, argv );

    if (!The::currentUser().crashReportingEnabled())
        return 0;

    // REFACTOR: move to lib
    // REFACTOR: just put in a file and include it here! no need for libbing it!
    QString langCode;
    QTranslator translatorApp;
    QTranslator translatorQt;

    #ifdef HIDE_RADIO
        langCode = "jp";
    #else
        // First check settings
        langCode = The::settings().appLanguage();
    #endif

    translatorApp.load( MooseUtils::dataPath( "i18n/lastfm_%1" ).arg( langCode ) );
    translatorQt.load( MooseUtils::dataPath( "i18n/qt_%1" ).arg( langCode ) );

    app.installTranslator( &translatorApp );
    app.installTranslator( &translatorQt );

    if ( app.arguments().size() != 4 )
        return 1;

    CrashReporter reporter( app.arguments() );
    reporter.show();

    return app.exec();
}
