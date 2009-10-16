/***************************************************************************
 *   Copyright (C) 2005 - 2007 by                                          *
 *      Christian Muehlhaeuser, Last.fm Ltd <chris@last.fm>                *
 *      Erik Jaelevik, Last.fm Ltd <erik@last.fm>                          *
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
#include "CachedHttp.h"
#include "LastFmSettings.h"
#include "version.h"
#include "logger.h"

#include <QIcon>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QDateTime>


CrashReporter::CrashReporter( const QStringList& args )
{
    #ifndef WIN32
    setWindowIcon( QIcon( MooseUtils::dataPath( "icons/as.ico" ) ) );
    #endif

    ui.setupUi( this );

    ui.logoLabel->setPixmap( QPixmap( MooseUtils::dataPath( "app_55.png" ) ) );
    ui.progressBar->setRange( 0, 100 );
    ui.progressBar->setValue( 0 );
    ui.progressLabel->setPalette( Qt::gray );

  #ifdef Q_WS_MAC
    // macify++
    QFont f = ui.bottomLabel->font();
    f.setPointSize( 10 );
    ui.bottomLabel->setFont( f );
    f.setPointSize( 11 );
    ui.progressLabel->setFont( f );
    ui.progressLabel->setIndent( 3 );
  #else
    ui.vboxLayout->setSpacing( 16 );
    ui.progressBar->setTextVisible( false );
    ui.progressLabel->setIndent( 1 );
    ui.bottomLabel->setDisabled( true );
    ui.bottomLabel->setIndent( 1 );

    // adjust the spacer since we adjusted the spacing above
    for (int x = 0; x < ui.vboxLayout->count(); ++x)
        if (QSpacerItem* spacer = ui.vboxLayout->itemAt( x )->spacerItem())
        {
            spacer->changeSize( 6, 2, QSizePolicy::Minimum, QSizePolicy::Fixed );
            break;
        }
  #endif //Q_WS_MAC

    m_http = new CachedHttp( "oops.last.fm", 80, this );
    connect( m_http, SIGNAL(dataAvailable( QByteArray )), SLOT(onDone( QByteArray )), Qt::QueuedConnection);
    connect( m_http, SIGNAL(dataSendProgress( int, int )), SLOT(onProgress( int, int )) );
    connect( m_http, SIGNAL(errorOccured( int, QString )), SLOT(onFail( int, QString )) );

    m_dir = args.value( 1 );
    m_minidump = m_dir + '/' + args.value( 2 ) + ".dmp";
    m_product_name = args.value( 3 );
    m_username = The::currentUser().username();

    QTimer::singleShot( 0, this, SLOT(send()) );

    setFixedSize( sizeHint() );
}


static QByteArray contents( const QString& path )
{
    QFile f( path );
    f.open( QFile::ReadOnly );
    return f.readAll();
}


void
CrashReporter::send()
{
    QByteArray body;

    // add parameters
    typedef QPair<QByteArray, QByteArray> Pair;
    QList<Pair> pairs;
    pairs << Pair( "BuildID", LASTFM_CLIENT_VERSION )
          << Pair( "ProductName", m_product_name.toUtf8() )
          << Pair( "Version", LASTFM_CLIENT_VERSION )
          << Pair( "Vendor", "Last.fm Ltd." )
//        << Pair( "URL", "http://www.last.fm/" )
//        << Pair( "Email", "-" )
          << Pair( "UserID", m_username.toUtf8() )
          << Pair( "timestamp", QByteArray::number( QDateTime::currentDateTime().toTime_t() ) );

    foreach (Pair const pair, pairs)
    {
        body += "--mooseboundary\r\n";
        body += "Content-Disposition: form-data; name=\"" +
                           pair.first  + "\"\r\n\r\n" +
                           pair.second + "\r\n";
    }

    // add minidump file
    body += "--mooseboundary\r\n";
    body += "Content-Disposition: form-data; name=\"upload_file_minidump\"; filename=\""
          + QFileInfo( m_minidump ).fileName() + "\"\r\n";
    body += "Content-Type: application/octet-stream\r\n";
    body += "\r\n";
    body += contents( m_minidump );
    body += "\r\n";

    // add logfile
    body += "--mooseboundary\r\n";
    body += "Content-Disposition: form-data; name=\"upload_file_containerlog\"; filename=\"container.log\"\r\n";
    body += "Content-Type: application/x-gzip\r\n";
    body += "\r\n";
    body += qCompress( contents( MooseUtils::logPath( "Last.fm.log" ) ) );
    body += "\r\n";
    body += "--mooseboundary--\r\n";

//////
    QHttpRequestHeader header( "POST", "/report/add" );
    header.setContentType( "multipart/form-data; boundary=mooseboundary" );
    header.setValue( "HOST", m_http->host() );

    m_http->request( header, body );
}


void
CrashReporter::onProgress( int done, int total )
{
    if (total)
    {
        QString const msg = tr( "Uploaded %L1 of %L2 KB." ).arg( done / 1024 ).arg( total / 1024 );

        ui.progressBar->setMaximum( total );
        ui.progressBar->setValue( done );
        ui.progressLabel->setText( msg );
    }
}


void
CrashReporter::onDone( const QByteArray& data )
{
    ui.progressBar->setValue( ui.progressBar->maximum() );
    ui.button->setText( tr("Close") );

    QString const response = QString::fromUtf8( data );

    if ( m_http->error() != QHttp::NoError || !response.startsWith( "CrashID=" ) )
    {
        onFail( m_http->error(), m_http->errorString() );
        qDebug() << m_http->errorString() << response;
    }
    else {
        //QString const id = response.section( '=', 1, 1 ).remove( " ViewURL" );
        ui.progressLabel->setText( tr("Sent! <b>Many thanks.</b>") );
        ui.progressLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );
    }
}

void
CrashReporter::onFail( int error, const QString& errorString )
{
    ui.button->setText( tr("Close") );
    ui.progressLabel->setText( tr( "Failed to send crash info." ) );
    qDebug() << "Error: " << error << errorString;
}
