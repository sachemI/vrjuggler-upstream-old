/****************** <VPR heading BEGIN do not edit this line> *****************
 *
 * VR Juggler Portable Runtime
 *
 * Original Authors:
 *   Allen Bierbaum, Patrick Hartling, Kevin Meinert, Carolina Cruz-Neira
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ****************** <VPR heading END do not edit this line> ******************/

/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
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
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include <vpr/vprConfig.h>

#include <iomanip>
#include <typeinfo>
#include <boost/concept_check.hpp>

#include <vpr/Util/Assert.h>
//#include <vpr/Thread/Thread.h>
#include <vpr/md/NSPR/NSPRHelpers.h>
#include <vpr/md/NSPR/Thread/ThreadNSPR.h>


namespace vpr
{

ThreadNSPR::staticWrapper ThreadNSPR::statics;    // Initialize the static data
PRUint32 ThreadNSPR::mTicksPerSec = PR_TicksPerSecond();

// Non-spawning constructor.  This will not start a thread.
ThreadNSPR::ThreadNSPR(VPRThreadPriority priority, VPRThreadScope scope,
                       VPRThreadState state, PRUint32 stackSize)
   : mThread(NULL)
   , mUserThreadFunctor(NULL)
   , mDeleteFunctor(false)
   , mStartFunctor(NULL)
   , mPriority(priority)
   , mScope(scope)
   , mState(state)
   , mStackSize(stackSize)
   , mCaughtException(false)
   , mException("No exception caught")
{
}

// Spawning constructor with arguments.  This will start a new thread that
// will execute the specified function.
ThreadNSPR::ThreadNSPR(thread_func_t func, void* arg,
                       VPRThreadPriority priority, VPRThreadScope scope,
                       VPRThreadState state, PRUint32 stackSize)
   : mThread(NULL)
   , mUserThreadFunctor(NULL)
   , mDeleteFunctor(false)
   , mStartFunctor(NULL)
   , mPriority(priority)
   , mScope(scope)
   , mState(state)
   , mStackSize(stackSize)
   , mCaughtException(false)
   , mException("No exception caught")
{
   mDeleteFunctor = true;
   setFunctor(new ThreadNonMemberFunctor(func, arg));
   start();
}

// Spawning constructor (functor version).  This will start a new thread that
// will execute the specified function.
ThreadNSPR::ThreadNSPR(BaseThreadFunctor* functorPtr,
                       VPRThreadPriority priority, VPRThreadScope scope,
                       VPRThreadState state, PRUint32 stackSize)
   : mThread(NULL)
   , mUserThreadFunctor(NULL)
   , mDeleteFunctor(false)
   , mStartFunctor(NULL)
   , mPriority(priority)
   , mScope(scope)
   , mState(state)
   , mStackSize(stackSize)
   , mCaughtException(false)
   , mException("No exception caught")
{
   setFunctor(functorPtr);
   start();
}

// Destructor.
ThreadNSPR::~ThreadNSPR()
{
   ThreadManager::instance()->lock();
   {
      unregisterThread();
   }
   ThreadManager::instance()->unlock();

   if ( NULL != mStartFunctor )
   {
      delete mStartFunctor;
      mStartFunctor = NULL;
   }

   if ( mDeleteFunctor )
   {
      delete mUserThreadFunctor;
      mUserThreadFunctor = NULL;
   }
}

void ThreadNSPR::setFunctor(BaseThreadFunctor* functorPtr)
{
   vprASSERT(mThread == NULL && "Thread already running");
   vprASSERT(functorPtr->isValid());

   mUserThreadFunctor = functorPtr;
}

// Creates a new thread that will execute mUserFunctorPtr.
vpr::ReturnStatus ThreadNSPR::start()
{
   vpr::ReturnStatus status;

   if ( NULL != mThread )
   {
      vprASSERT(false && "Thread already running");
      status.setCode(vpr::ReturnStatus::Fail);
   }
   else if ( NULL == mUserThreadFunctor )
   {
      vprASSERT(false && "No functor set");
      status.setCode(vpr::ReturnStatus::Fail);
   }
   else
   {
      PRThreadPriority nspr_prio;
      PRThreadScope nspr_scope;
      PRThreadState nspr_state;

      nspr_prio  = vprThreadPriorityToNSPR(mPriority);
      nspr_scope = vprThreadScopeToNSPR(mScope);
      nspr_state = vprThreadStateToNSPR(mState);

      vprASSERT(mUserThreadFunctor->isValid());

      // Store the member functor and create the functor for spawning to our
      // start routine.
      mStartFunctor = 
         new ThreadMemberFunctor<ThreadNSPR>(this, &ThreadNSPR::startThread,
                                             NULL);

      // Finally create the thread.
      // - On success --> The start method registers the actual thread info
      mThreadStartCompleted = false;      // Initialize registration flag (uses cond var for this)
      PRThread* ret_thread =
         PR_CreateThread(PR_USER_THREAD, vprThreadFunctorFunction,
                         (void*) mStartFunctor, nspr_prio, nspr_scope,
                         nspr_state, (PRUint32) mStackSize);

      // Inform the caller if the thread was not created successfully.
      if ( NULL == ret_thread )
      {
         ThreadManager::instance()->lock();
         {
            registerThread(false);
         }
         ThreadManager::instance()->unlock();

         NSPR_PrintError("vpr::ThreadNSPR::spawn() - Cannot create thread");
         status.setCode(vpr::ReturnStatus::Fail);
      }
      else
      {
         // start thread will register the thread, so let's wait for it
         // -- Wait for registration to complete
         mThreadStartCondVar.acquire();
         {
            // While not desired state (ie. register completed)
            while ( !mThreadStartCompleted )
            {
               mThreadStartCondVar.wait();
            }
         }
         mThreadStartCondVar.release();
         // ASSERT: Thread has completed registration
         vprASSERT(NULL != mThread && "Thread registration failed");
         vprASSERT(-1 != getTID() && "Thread id is invalid for successful thread");

         // Set the return code to success
         status.setCode(vpr::ReturnStatus::Succeed);
      }
   }

   return status;
}

int ThreadNSPR::join(void** status)  throw (UncaughtThreadException)
{
   boost::ignore_unused_variable_warning(status);
   if (mCaughtException)
   {
      throw mException;
   }
   return PR_JoinThread(mThread);
}

Thread* ThreadNSPR::self()
{
   vprASSERT((statics.mStaticsInitialized==1221) && "Trying to call vpr::ThreadNSPR::self before statics are initialized. Don't do that");

   Thread* my_thread;
   threadIdKey().getspecific((void**)&my_thread);

   return my_thread;
}

std::ostream& Thread::outStream(std::ostream& out)
{
   out.setf(std::ios::right);
   out << std::setw(7) << std::setfill('0')
#ifdef VPR_OS_Windows
       << _getpid()
#else
       << getpid()
#endif
       << "/";
   out.unsetf(std::ios::right);
   BaseThread::outStream(out);
   out << std::setfill(' ');
   return out;
}

/**
 * Helper method Called by the spawn routine to start the user thread function.
 *
 * @pre Called ONLY by a new thread
 * @post Do any thread registration necessary
 *       Call the user thread functor
 *
 * @param null_param
 */
void ThreadNSPR::startThread(void* nullParam)
{
   boost::ignore_unused_variable_warning(nullParam);

   // WE are a new thread... yeah!!!!
   // TELL EVERYONE THAT WE LIVE!!!!
   mThread = PR_GetCurrentThread();                   // Set the identity of the thread
   vprASSERT(NULL != mThread && "Invalid thread");    // We should not be able to have a NULL thread
   ThreadManager::instance()->lock();                 // Lock manager
   {
      threadIdKey().setspecific((void*)this);         // Store self in thread local data
      registerThread(true);                           // Finish thread initialization
   }
   ThreadManager::instance()->unlock();

   // Signal that registration is completed
   mThreadStartCondVar.acquire();
   {
      mThreadStartCompleted = true;
      mThreadStartCondVar.signal();
   }
   mThreadStartCondVar.release();

   try
   {
      // --- CALL USER FUNCTOR --- //
      (*mUserThreadFunctor)();
   }
   catch (vpr::Exception& ex)
   {
      vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
         << clrOutNORM(clrYELLOW, "WARNING:")
         << " Caught an unhandled exception of type " << typeid(ex).name()
         << " in thread:" << std::endl
         << ex.getExtendedDescription() << std::endl << vprDEBUG_FLUSH;
      vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
         << "Thread exiting due to uncaught exception\n" << vprDEBUG_FLUSH;

      mCaughtException = true;
      mException.setException(ex);
   }
   catch (std::exception& ex)
   {
      vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
         << clrOutNORM(clrYELLOW, "WARNING:")
         << " Caught an unhandled exception of type " << typeid(ex).name()
         << " in thread:" << std::endl
         << ex.what() << std::endl << vprDEBUG_FLUSH;
      vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
         << "Thread exiting due to uncaught exception\n" << vprDEBUG_FLUSH;

      mCaughtException = true;
      mException.setException(ex);
   }
}


// Set this thread's priority.
int ThreadNSPR::setPrio(VPRThreadPriority prio)
{
   int retval(0);

   if ( prio > 3 )
   {
      retval = -1;
   }
   else
   {
      PR_SetThreadPriority(mThread, vprThreadPriorityToNSPR(prio));
   }

   return retval;
}

// ===========================================================================
// Private methods follow.
// ===========================================================================


PRThreadPriority ThreadNSPR::vprThreadPriorityToNSPR(const VPRThreadPriority priority)
{
   PRThreadPriority nspr_prio;

   switch ( priority )
   {
      case VPR_PRIORITY_LOW:
         nspr_prio = PR_PRIORITY_LOW;
         break;
      case VPR_PRIORITY_NORMAL:
         nspr_prio = PR_PRIORITY_NORMAL;
         break;
      case VPR_PRIORITY_HIGH:
         nspr_prio = PR_PRIORITY_HIGH;
         break;
      case VPR_PRIORITY_URGENT:
         nspr_prio = PR_PRIORITY_URGENT;
         break;
   };

   return nspr_prio;
}

PRThreadScope ThreadNSPR::vprThreadScopeToNSPR(const VPRThreadScope scope)
{
   PRThreadScope nspr_scope;

   switch ( scope )
   {
      case VPR_LOCAL_THREAD:
         nspr_scope = PR_LOCAL_THREAD;
         break;
      case VPR_GLOBAL_THREAD:
         nspr_scope = PR_GLOBAL_THREAD;
         break;
   };

   return nspr_scope;
}

PRThreadState ThreadNSPR::vprThreadStateToNSPR(const VPRThreadState state)
{
   PRThreadState nspr_state;

   switch ( state )
   {
      case VPR_JOINABLE_THREAD:
         nspr_state = PR_JOINABLE_THREAD;
         break;
      case VPR_UNJOINABLE_THREAD:
         nspr_state = PR_UNJOINABLE_THREAD;
         break;
   };

   return nspr_state;
}

BaseThread::VPRThreadPriority ThreadNSPR::nsprThreadPriorityToVPR(const PRThreadPriority priority)
{
   VPRThreadPriority vpr_prio;

   switch ( priority )
   {
      case PR_PRIORITY_LOW:
         vpr_prio = VPR_PRIORITY_LOW;
         break;
      case PR_PRIORITY_NORMAL:
         vpr_prio = VPR_PRIORITY_NORMAL;
         break;
      case PR_PRIORITY_HIGH:
         vpr_prio = VPR_PRIORITY_HIGH;
         break;
      case PR_PRIORITY_URGENT:
         vpr_prio = VPR_PRIORITY_URGENT;
         break;
   };

   return vpr_prio;
}

BaseThread::VPRThreadScope ThreadNSPR::nsprThreadScopeToVPR(const PRThreadScope scope)
{
   VPRThreadScope vpr_scope;

   switch ( scope )
   {
      case PR_LOCAL_THREAD:
         vpr_scope = VPR_LOCAL_THREAD;
         break;
      case PR_GLOBAL_THREAD:
         vpr_scope = VPR_GLOBAL_THREAD;
         break;
   };

   return vpr_scope;
}

BaseThread::VPRThreadState ThreadNSPR::nsprThreadStateToVPR(const PRThreadState state)
{
   VPRThreadState vpr_state;

   switch ( state )
   {
      case PR_JOINABLE_THREAD:
         vpr_state = VPR_JOINABLE_THREAD;
         break;
      case PR_UNJOINABLE_THREAD:
         vpr_state = VPR_UNJOINABLE_THREAD;
         break;
   };

   return vpr_state;
}

} // End of vpr namespace
