/* Generated by Together */

#ifndef AJSOUNDIMPLEMENTATION_H
#define AJSOUNDIMPLEMENTATION_H
#include <string>
#include <map>
#include "ajSoundInfo.h"
#include "ajSoundAPIInfo.h"

class ajSoundImplementation
{
public:
   /**
    * @semantics default constructor 
    */
   ajSoundImplementation() : mSounds()
   {
      mListenerPos[0] = 0.0f;
      mListenerPos[1] = 0.0f;
      mListenerPos[2] = 0.0f;
   }

   /**
    * @semantics destructor 
    */
   virtual ~ajSoundImplementation()
   {
      // make sure the API has gracefully exited.
      this->shutdownAPI();
   }

   /**
    * copies current state of the system from one API to another.
    * @semantics copies sound state.  doesn't do binding here, you must do that separately
    */
   void copy( const ajSoundImplementation& si )
   {
      // copy over the current state
      mSounds = si.mSounds;
   }

public:

   /**
    * @input alias of the sound to trigger, and number of times to play
    * @preconditions alias does not have to be associated with a loaded sound.
    * @postconditions if it is, then the loaded sound is triggered.  if it isn't then nothing happens.
    * @semantics Triggers a sound
    */
   virtual void trigger( const std::string & alias, const unsigned int & repeat = -1 )
   {
      assert( this->isStarted() == true && "must call startAPI prior to this function" );
      
      // todo: capture this in a soundinfo::play func
      this->lookup( alias ).isPlaying = true;
      this->lookup( alias ).repeat = repeat;
      this->lookup( alias ).repeat_countdown = repeat;
   }

   /**
    * when sound is already playing then you call trigger,
    * does the sound restart from beginning?
    * (if a tree falls and no one is around to hear it, does it make sound?)
    */
   virtual void setRetriggerable( const std::string& alias, bool onOff )
   {
      // todo, maybe set a flag within ajSoundInfo?
   }

   /**
    * ambient or positional sound.
    * is the sound ambient - attached to the listener, doesn't change volume
    * when listener moves...
    * or is the sound positional - changes volume as listener nears or retreats..
    */
   void setAmbient( const std::string& alias, bool setting = false )
   {
   }

   /**
    * @semantics stop the sound
    * @input alias of the sound to be stopped
    */
   virtual void stop( const std::string& alias )
   {
      assert( this->isStarted() == true && "must call startAPI prior to this function" );
      
      this->lookup( alias ).isPlaying = false;
      this->lookup( alias ).repeat_countdown = 0;
   }

   /**
    * pause the sound, use unpause to return playback where you left off...
    */
   virtual void pause( const std::string& alias )
   {
      this->stop( alias );
   }

   /**
    * resume playback from a paused state.  does nothing if sound was not paused.
    */
   virtual void unpause( const std::string& alias )
   {
      this->play( alias, 1 );
   }

   /**
    * mute, sound continues to play, but you can't hear it...
    */
   virtual void mute( const std::string& alias )
   {
   }

   /**
    * unmute, let the muted-playing sound be heard again
    */
   virtual void unmute( const std::string& alias )
   {
   }

   /**
    * set sound's 3D position 
    */
   virtual void setPosition( const std::string& alias, float x, float y, float z )
   {
      assert( this->isStarted() == true && "must call startAPI prior to this function" );
      this->lookup( alias ).position[0] = x;
      this->lookup( alias ).position[1] = y;
      this->lookup( alias ).position[2] = z;
   }

   /**
    * get sound's 3D position
    * @input alias is a name that has been associate()d with some sound data
    * @output x,y,z are returned in OpenGL coordinates.
    */
   virtual void getPosition( const std::string& alias, float& x, float& y, float& z )
   {
      assert( this->isStarted() == true && "must call startAPI prior to this function" );
      
      x = this->lookup( alias ).position[0];
      y = this->lookup( alias ).position[1];
      z = this->lookup( alias ).position[2];
   }

   /**
    * set the position of the listener
    */
   virtual void setListenerPosition( const vjMatrix& mat )
   {
      assert( this->isStarted() == true && "must call startAPI prior to this function" );
      
      mListenerPos[0] = x;
      mListenerPos[1] = y;
      mListenerPos[2] = z;
   }

   /**
    * get the position of the listener
    */
   virtual void getListenerPosition( vjMatrix& mat ) const
   {
      assert( this->isStarted() == true && "must call startAPI prior to this function" );
      
      x = mListenerPos[0];
      y = mListenerPos[1];
      z = mListenerPos[2];
   }

   /**
    * start the sound API, creating any contexts or other configurations at startup
    * @postconditions sound API is ready to go.
    * @semantics this function should be called before using the other functions in the class.
    */
   virtual void startAPI() = 0;

   /**
    * query whether the API has been started or not
    * @semantics return true if api has been started, false otherwise.
    */
   virtual bool isStarted() const = 0;   
   
   /**
    * kill the sound API, deallocating any sounds, etc...
    * @postconditions sound API is ready to go.
    * @semantics this function could be called any time, the function could be called multiple times, so it should be smart.
    */
   virtual void shutdownAPI() {}

   /*
    * configure/reconfigure the sound API global settings
    */
   virtual void configure( const ajSoundAPIInfo& sai )
   {
      mSoundAPIInfo = sai;
   }

   /**
     * configure/reconfigure a sound
     * configure: associate a name (alias) to the description if not already done
     * reconfigure: change properties of the sound to the descriptino provided.
     * @preconditions provide an alias and a SoundInfo which describes the sound
     * @postconditions alias will point to loaded sound data
     * @semantics associate an alias to sound data.  later this alias can be used to operate on this sound data.
     */
   virtual void configure( const std::string& alias, const ajSoundInfo& description )
   {
      mSounds[alias] = description;
      if (this->isStarted())
      {
         this->bind( alias );
      }
   }

   /**
    * remove a configured sound, any future reference to the alias will not
    * cause an error, but will not result in a rendered sound
    */
   virtual void remove(const std::string alias)
   {
      if (this->isStarted())
      {
         this->unbind( alias );
      }
      mSounds.erase( alias );
   }

   /**
    * @semantics call once per sound frame (doesn't have to be same as your graphics frame)
    * @input time elapsed since last frame
    */
   virtual void step( const float& timeElapsed )
   {
      assert( this->isStarted() == true && "must call startAPI prior to this function" );
      
   }   

   /**
    * clear all associate()tions.
    * @semantics any existing aliases will be stubbed. sounds will be unbound
    */
   virtual void clear()
   {
      this->unbindAll();
   }   

   /**
    * bind: load (or reload) all associate()d sounds
    * @postconditions all sound associations are buffered by the sound API
    */
   virtual void bindAll() = 0;

   /**
    * unbind: unload/deallocate all associate()d sounds.
    * @postconditions all sound associations are unbuffered by the sound API
    */
   virtual void unbindAll() = 0;

   /**
    * load/allocate the sound data this alias refers to the sound API
    * @postconditions the sound API has the sound buffered.
    */
   virtual void bind( const std::string& alias ) = 0;

   /**
    * unload/deallocate the sound data this alias refers from the sound API
    * @postconditions the sound API no longer has the sound buffered.
    */
   virtual void unbind( const std::string& alias ) = 0;

   ajSoundInfo& lookup( const std::string& alias )
   {
      return mSounds[alias];
   }

protected:
   ajSoundAPIInfo mSoundAPIInfo;
   std::map<std::string, ajSoundInfo> mSounds;

private:
   /*
    * position of the observer/listener
    */
   float mListenerPos[3];

   /** This class uses a std::map of sound infos for alias lookup
    * @link aggregation
    * @supplierCardinality 0..*
    * @clientCardinality 1*/
   //ajSoundInfo lnkSoundInfo_not_used_see_one_above;
};
#endif //AJSOUNDIMPLEMENTATION_H
