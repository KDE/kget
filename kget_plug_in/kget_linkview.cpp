#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoolbar.h>

#include "kget_linkview.h"

#define COL_NAME 0
#define COL_DESC 1
#define COL_MIME 2
#define COL_URL  3

LinkViewItem::LinkViewItem( QListView *parent, const LinkItem *lnk )
    : QListViewItem( parent ),
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

    KActionCollection *coll = actionCollection();

    (void ) new KAction( i18n("Download Selected Files"),
                         "khtml_kget",
                         CTRL+Key_D,
                         this, SLOT( slotStartLeech() ),
                         actionCollection(), "startDownload" );

    m_links.setAutoDelete( true );
    coll->action( "startDownload" )->plug( toolBar() );

    initView();
}

KGetLinkView::~KGetLinkView()
{
}

void KGetLinkView::initView()
{
    m_view = new KListView( this, "listview" );
    m_view->setSelectionMode( QListView::Extended );
    m_view->addColumn( i18n("File Name") );
    m_view->addColumn( i18n("Description") );
    m_view->addColumn( i18n("File Type") );
    m_view->addColumn( i18n("Location (URL)") );
    m_view->setShowSortIndicator( true );

    setCentralWidget( m_view );
}

void KGetLinkView::setLinks( QPtrList<LinkItem>& links )
{
    m_links = links; // now we 0wn them
    showLinks( m_links );
}

void KGetLinkView::showLinks( const QPtrList<LinkItem>& links )
{
    m_view->clear();

    QPtrListIterator<LinkItem> it( links );
    for ( ; it.current(); ++it )
        (void) new LinkViewItem( m_view, *it );

    m_view->sort();
}

void KGetLinkView::slotStartLeech()
{
    KURL::List urls;
    QListViewItemIterator it( m_view->firstChild() );
    for ( ; it.current(); ++it )
    {
        if ( it.current()->isSelected() )
            urls.append( static_cast<LinkViewItem*>( it.current() )->link->url );
    }

    if ( urls.isEmpty() )
        KMessageBox::sorry( this,
                            i18n("You did not select any files to download."),
                            i18n("No Files Selected") );
    else
        emit leechURLs( urls );
}

void KGetLinkView::setPageURL( const QString& url )
{
    setPlainCaption( i18n( "Links in: %1 - KGet" ).arg( url ) );
}

#include "kget_linkview.moc"
