#include "iconview.h"

/*
* KGetIconViewItem
*
*/

KGetIconViewTransfer::KGetIconViewTransfer( KGetIconView * parent, Transfer * t )
    : QIconViewItem( parent ), transfer( t )
{
    setRenameEnabled( false );
    setDragEnabled( false );
    setDropEnabled( false );

}
 

/*
* KGetIconViewItem
*
*/

KGetIconView::KGetIconView( Scheduler * s, QWidget * parent, const char * name )
    : QIconView( parent, name ), ViewInterface( s )
{


}

