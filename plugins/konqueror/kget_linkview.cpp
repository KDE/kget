#include "kget_linkview.h"

#include <dcopclient.h>
#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstdaction.h>
#include <ktoolbar.h>
//Added by qt3to4:
#include <Q3PtrList>

#define COL_NAME 0
#define COL_DESC 1
#define COL_MIME 2
#define COL_URL  3

LinkViewItem::LinkViewItem( Q3ListView *parent, const LinkItem *lnk )
    : Q3ListViewItem( parent ),
      link( lnk )
{
    QString file = link->url.fileName();
    if ( file.isEmpty() )
        file = link->url.host();

    setPixmap( COL_NAME, SmallIcon( link->icon ) );
    setText( COL_NAME, file );

    setText( COL_DESC, link->text );
    setText( COL_MIME, link->mimeType );
    setText( COL_URL,  link->url.prettyURL() );
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KGetLinkView::KGetLinkView( QWidget *parent, const char *name )
    : KMainWindow( parent, name )
{
    setPlainCaption( i18n( "KGet" ) );

    KAction* actionDownload = new KAction( i18n("Download Selected Files"),
                                           "khtml_kget", "CTRL+Key_D",
                                           this, SLOT( slotStartLeech() ),
                                           actionCollection(), "startDownload" );

    KAction* actionSelectAll = KStdAction::selectAll( this, SLOT( slotSelectAll() ),
                                                      actionCollection() );

    m_links.setAutoDelete( true );
    actionDownload->plug( toolBar() );
    toolBar()->insertLineSeparator();
    actionSelectAll->plug( toolBar() );

    m_view = new KListView( this, "listview" );
    m_view->setSelectionMode( Q3ListView::Extended );
    m_view->addColumn( i18n("File Name") );
    m_view->addColumn( i18n("Description") );
    m_view->addColumn( i18n("File Type") );
    m_view->addColumn( i18n("Location (URL)") );
    m_view->setShowSortIndicator( true );

    setCentralWidget( m_view );

    // setting a fixed (not floating) toolbar
    toolBar()->setMovingEnabled(false);
    // setting Text next to Icons
    toolBar()->setIconText( KToolBar::IconTextRight );
}

KGetLinkView::~KGetLinkView()
{
}

void KGetLinkView::setLinks( Q3PtrList<LinkItem>& links )
{
    m_links = links; // now we 0wn them
    showLinks( m_links );
}

void KGetLinkView::showLinks( const Q3PtrList<LinkItem>& links )
{
    m_view->clear();

    Q3PtrListIterator<LinkItem> it( links );
    for ( ; it.current(); ++it )
        (void) new LinkViewItem( m_view, *it );

    m_view->sort();
}

void KGetLinkView::slotStartLeech()
{
    QByteArray * data;
    QDataStream stream( data, QIODevice::WriteOnly );
    bool itemSelected = false;

    QStringList urls;

    Q3ListViewItemIterator it( m_view->firstChild() );
    for ( ; it.current(); ++it )
    {
        if ( it.current()->isSelected() )
        {
            QString url = static_cast<LinkViewItem*>( it.current() )->link->url.url();

            stream << url;
            urls.append( url );
            itemSelected = true;
        }
    }

    if ( !itemSelected )
        KMessageBox::sorry( this,
                            i18n("You did not select any files to download."),
                            i18n("No Files Selected") );
    else
    {
        DCOPClient* p_dcopServer = new DCOPClient();
        p_dcopServer->attach();

        if (!p_dcopServer->isApplicationRegistered("kget"))
        {
            KProcess* proc = new KProcess();
            *proc << "kget" << urls;
            proc->start( KProcess::DontCare );
        }
        else
        {
            stream << QString();
            bool ok = DCOPClient::mainClient()->send( "kget", "KGet-Interface",
                                                      "addTransfers(KURL::List, QString)",
                                                      *data );

            kdDebug() << "*** startDownload: " << ok << endl;
        }

        p_dcopServer->detach();
        delete p_dcopServer;
    }
}

void KGetLinkView::setPageURL( const QString& url )
{
    setPlainCaption( i18n( "Links in: %1 - KGet" ).arg( url ) );
}

void KGetLinkView::slotSelectAll()
{
    m_view->selectAll( true );
}

#include "kget_linkview.moc"
