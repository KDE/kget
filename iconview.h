#include "viewinterface.h"

#include <qiconview.h>


class KGetIconView;

class KGetIconViewItem : public QIconViewItem
{
    public:
	KGetIconViewItem( KGetIconView * parent, const char * name = 0 );

    private:
	

};

class KGetIconView : public QIconView, public ViewInterface
{
    public:
	KGetIconView( Scheduler * s, QWidget * parent = 0, const char * name = 0 );

    
};
