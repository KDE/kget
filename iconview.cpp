#include "iconview.h"

/*
* KGetIconViewItem
*
*/

KGetIconViewItem::KGetIconViewItem( KGetIconView * parent, const char * name )
    : QIconViewItem( parent, name )
{


}
 

/*
* KGetIconViewItem
*
*/

KGetIconView::KGetIconView( Scheduler * s, QWidget * parent, const char * name )
    : QIconView( parent, name ), ViewInterface( s )
{


}

