/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _TORUS_APP
#define _TORUS_APP

#include <vrj/vrjConfig.h>

#include <vrj/Draw/OGL/GlApp.h>

#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/DigitalInterface.h>


namespace vrjTest
{

//--------------------------------------------------
// Torus test OpenGL application class
//
// This application simply renders a torus in the corner of a C2.
//---------------------------------------------------
class torusApp : public vrj::GlApp
{
public:
   torusApp(vrj::Kernel* kern) : vrj::GlApp(kern), mTorusRotation(0.0f)
   {;}

   virtual ~torusApp (void) {
      /* Do nothing. */ ;
   }

   // Execute any initialization needed before the API is started.  Put device
   // initialization here.
   virtual void init()
   {
      mWand.init("VJWand");            // Handle to the wand device
      mAppExit.init("VJAppExit");      // App exit key
   }

   // Execute any initialization needed <b>after</b> API is started
   //  but before the drawManager starts the drawing loops.
   virtual void apiInit()
   {;}

   // Called immediately upon opening a new OpenGL context.  This is called
   // once for every display window that is opened.  Put OpenGL resource
   // allocation here.
   virtual void contextInit()
   {
      initGLState();
   }

   // Function called after tracker update but before start of drawing.  Do
   // calculations and state modifications here.
   virtual void preFrame()
   {
      //static long count=0;
      //count++;
      
      // Did we ask for an app exit
      //vprDEBUG(vprDBG_ALL, 0) << "VJAppExit: " << mAppExit->getData() << std::endl << vprDEBUG_FLUSH;
      if(mAppExit->getData() )
      {
         vprDEBUG(vprDBG_ALL, 0) << "APP EXIT KEY PRESSED: Stopping kernel and exiting.\n" << vprDEBUG_FLUSH;
         mKernel->stop();     // trigger a kernel stop
      }
   }

   virtual void bufferPreDraw();

   // Function to draw the scene.  Put OpenGL draw functions here.
   //
   // PRE: OpenGL state has correct transformation and buffer selected
   // POST: The current scene has been drawn
   virtual void draw();

   // Function called after drawing has been triggered but BEFORE it completes
   virtual void intraFrame()
   {;}

   // Function called before updating trackers but after the frame is drawn.
   // Do calculations here.
   virtual void postFrame()
   {;}

   //: Execute any final cleanup needed for the application
   virtual void exit()
   {
      vprDEBUG(vprDBG_ALL, 0) << "torusApp::exit: Exit called. Cleaning up application.\n" << vprDEBUG_FLUSH;;
   }


private:
   void initGLState();

private:
   gadget::PositionInterface    mWand;
   float                        mTorusRotation;
   gadget::DigitalInterface     mAppExit;
};

}; // namespace vrjTest

#endif
