/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 */

#include <stdlib.h>   //for NULL
#include <unistd.h>   //for sleep
#include <iostream.h> // for cout
#include <vjSound/vjSoundFactory.h>
#include "fileIO.h"

vjSoundEngine* gSoundEngine = NULL;
vjSound* longPan = NULL, *fastPan = NULL, *shortTriggeredvjSound = NULL;

void usage(char** argv)
{
   cout<<"Usage:\n";
      cout<<"      "<<argv[0]<<" sl  (for plib's SL library)\n";
      cout<<"      "<<argv[0]<<" aw  (for audioworks library)\n"<<flush;
      cout<<"      "<<argv[0]<<" nosound  (to stub out all sound functionality)\n"<<flush;
}

bool initvjSoundEngine( const std::string& arg, vjSoundEngine* &engine )
{
   std::string soundConfigFile;
   if (arg == "sl") // some hacky logic.
   {
      soundConfigFile = "sound.dat";
   }
   
   else if (arg == "aw")
   {
      soundConfigFile = "sound.adf";
   }
   
   else if (arg == "nosound")
   {
      soundConfigFile = "sound.dat";
   }   
   else
   {
      engine = NULL;
      return false; //user didn't specify an option
   }
   
   std::string soundConfigFileWithPath;
   if (fileIO::fileExistsResolvePath( soundConfigFile, soundConfigFileWithPath ))
   {
      engine = vjSoundFactory::newEngine( arg, soundConfigFileWithPath.c_str() );
      
      if ( engine == NULL)
      {
         cout<<"NULL engine\n"<<flush;
         return false;
      }      
   }
      
   else
   {
      cout<<"Couldn't find "<<soundConfigFile<<"\n"<<flush;
      engine = NULL;
      return false;
   }    
   
   cout<<"Good engine\n"<<flush;
   return true; //user specified an option
}

void main(int argc, char** argv)
{
   std::string arg;
   
   if (argc > 1)
   {
      arg = argv[1];
   }   
   
   // get the sound engine...
   bool success = initvjSoundEngine( arg, gSoundEngine );
   if (!success)
   {
      ::usage(argv);
      cout<<"\n"<<flush;
      cout<<"!!! WARNING !!!: Option \""<<arg.c_str()<<"\" not recognized, Defaulting to the \"nosound\" option\n"<<flush;
      cout<<"\n"<<flush;
      arg = "nosound"; //default arg
      success = initvjSoundEngine( arg, gSoundEngine );
      sleep( 1 );
   }   
   
   assert( gSoundEngine != NULL );
   assert( success == true );
   
   longPan = gSoundEngine->getHandle( "longPan" );
   longPan->print();
   
   fastPan = gSoundEngine->getHandle( "fastPan" );
   fastPan->print();
   
   shortTriggeredvjSound = gSoundEngine->getHandle( "shortTriggeredvjSound" );
   shortTriggeredvjSound->print();

   // Find players
   //pendulumplayer = awFindPlyr("pendulumplayer");

   // Find objects
   //you = awFindObs("you");
 
   /*gSoundEngine->setPosition( 10, 0, 0 );
   longPan->trigger();
   gSoundEngine->update();
   usleep( 1 );
   
   gSoundEngine->setPosition( 0, 5, 0 );
   fastPan->trigger();
   gSoundEngine->update();
   usleep( 1 );
   
   gSoundEngine->setPosition( 0, 5, 5 );
   shortTriggeredvjSound->trigger();
   gSoundEngine->update();
   usleep( 1 );
   
   gSoundEngine->setPosition( 0, 0, 20 );
   fastPan->trigger();
   gSoundEngine->update();
   sleep( 1 );
   
   gSoundEngine->setPosition( 0, 0, 10 );
   longPan->trigger();
   gSoundEngine->update();
   sleep( 1 );
   
   gSoundEngine->setPosition( 0, 0, 5 );
   shortTriggeredvjSound->trigger();
   gSoundEngine->update();
   sleep( 1 );
   
   gSoundEngine->setPosition( -5, -5, -4 );
   longPan->trigger();
   gSoundEngine->update();
   sleep( 1 );
   
   gSoundEngine->setPosition( 30, 0, 0 );
   fastPan->trigger();
   gSoundEngine->update();
   sleep( 1 );
   */

   gSoundEngine->setPosition( 0, 0, 0 );
  
   // NOTES:  AW messes up when you call trigger, and setPosition every frame.
   //         if that happens, then you'll only hear sound when it's at 0,0,0
   //         this shouldn't be a problem though, since in a real app
   //         sounds are triggered infrequently compared to the number 
   //         of frames occuring.
   
   // fastPan positional test.
   // real fast panning
   cout<<"Fast panning\n"<<flush;
   for (int yy = 0; yy < 5; ++yy)
   {
      fastPan->trigger();
      for ( int x = 0; x < 100; ++x )  
      {
         cout<<"Position: "<<x - 50<<"\n"<<flush;
         //longPan->stop();
         //gSoundEngine->update();
         fastPan->setPosition( x - 50, 0, 0 );
         //gSoundEngine->update();
         gSoundEngine->update();
         usleep( 25000 );
      }
   }
   sleep( 1 );

   // positional test, for when longPan has looppoints.
   // nice long panning.
   cout<<"Slow panning left to right, back to front\n"<<flush;
   for ( int x = 0; x < 5; ++x)
   {
      longPan->trigger(); // just to test out triggering, 
                         // and to have a predictable beginning.
   
      // pan back ...
      for ( int z = 0; z < 100; ++z )  
      {
         longPan->setPosition( z - 50, 0, x * 10 - 50 );
         gSoundEngine->update();
         usleep( 50000 );
      }
      
      // ... and forth
      for ( z = 100; z > 0; --z )  
      {
         longPan->setPosition( z - 50, 0, x * 10 - 50 );
         gSoundEngine->update();
         usleep( 50000 );
      }
   }
  
   sleep( 1 );
   
   // or just trigger a sound, no panning...
   cout<<"Triggering a short sound....\n"<<flush;
   shortTriggeredvjSound->trigger();
   gSoundEngine->update();
   
   sleep( 3 );
  
   gSoundEngine->update();
   longPan->stop();
   fastPan->stop();
   shortTriggeredvjSound->stop();
   gSoundEngine->update();
  
   usleep( 30000 );
   
   cout<<"vjSound should have stopped....\n"<<flush;
   cout<<"Done...\n"<<flush;
   gSoundEngine->kill();
   
   
   delete gSoundEngine;
   delete longPan;
   
   
   usleep( 1 );
}
