
#include "../MultiSegKio.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <QTimer>
#include <kio/filejob.h>
#include <kio/job.h>

#include <iostream>

static KCmdLineOptions options[] = { { "+file", "url", 0 } , KCmdLineLastOption };

int main( int argc, char ** argv )
{
   KAboutData about( "kiotest", "Test of KIO::MultiSegmentCopyJob",
                        "0.01", "Test",
                        KAboutData::License_GPL, 0 );
   about.addAuthor( "Manolo Valdes", 0, "nolis71cu@gmail.com" );
   KCmdLineArgs::addCmdLineOptions(options);
   KCmdLineArgs::init( argc, argv, &about );
   KApplication app;
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );
   if ( args->count() < 2 )
   {
      std::cout << "error to few arguments" << std::endl;
      return 1;
   }
   KIO::MultiSegfile_copy( args->url(0), args->url(1), -1, false, 5);
   app.processEvents();
   app.exec();
}
