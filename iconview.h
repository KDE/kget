#include "viewinterface.h"

#include <qiconview.h>


class KGetIconView;

class KGetIconViewTransfer : public QIconViewItem
{
    public:
	KGetIconViewTransfer( KGetIconView * parent, Transfer * transfer );

    protected:
	

    private:
	Transfer * transfer;
};

class KGetIconView : public QIconView, public ViewInterface
{
    public:
	KGetIconView( Scheduler * s, QWidget * parent = 0, const char * name = 0 );

    
};
