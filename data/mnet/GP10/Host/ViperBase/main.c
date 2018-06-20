/*
 * main.c
 */

/* GKLib includes */
#include <gkadd.h>
#include <gkrgi.h>
#include <gkstorage.h>
#include <li.h>

#include "support.h"
#include "exchange.h"
#include "init.h"
#include "admin.h"

void EndpointsPrintTimer( PVOID );

/*
 *
 */
int main( int argc, char** argv )
{

   if( FALSE == MainInit( argc, argv ) )
   {
      StringPrint( "Initialization Error\n" );
      return FALSE;
   }

   StringPrint( "Initialization Successful\n\n" );

   /* TimerCreate( 30, EndpointsPrintTimer, NULL, TRUE ); */

   MainLoop();

   return TRUE;
}

/*
 *
 */
void EndpointsPrintTimer( PVOID pVoid )
{
   EndpointsPrint();
   return;
}
