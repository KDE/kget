#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include <qfileinfo.h>

#include "safedelete.h"

bool SafeDelete::deleteFile( const KURL& url )
{
    if ( url.isLocalFile() )
    {
        QFileInfo info( url.path() );
        if ( info.isDir() )
        {
            KMessageBox::information(0L,
                                     i18n("Not deleting\n%1\nas it is a "
                                          "directory.").arg( url.prettyURL() ),
                                     i18n("Not Deleted"));
            return false;
        }

        KIO::NetAccess::del( url, 0L );
        return true;
    }

    else
        KMessageBox::information( 0L,
                                  i18n("Not deleting\n%1\nas it is not a local"
                                       " file.").arg( url.prettyURL() ),
                                  i18n("Not Deleted") );

    return false;
}
