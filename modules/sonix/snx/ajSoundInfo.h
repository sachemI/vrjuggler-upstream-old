/* Generated by Together */

#ifndef SOUND_INFO_DATA
#define SOUND_INFO_DATA
#include <string>

/**
 * info struct that describes one sound entry.
 */
struct ajSoundInfo
{
   ajSoundInfo() : ambient( true ), isPlaying( false ), datasource( FILESYSTEM ), repeat( 1 ), repeat_countdown( 0 ), data(), filename()
   {
      position.makeIdent();
   }
   std::string alias;

   enum DataSource
   {
      FILESYSTEM, DATA_16_MONO, DATA_8_MONO
   };
   DataSource datasource; // which of the following resources to use...

   // source of the sound...
   std::string filename;
   std::string data; // TODO: i'll probably want to double buffer this...

   vjMatrix position;
   
   bool ambient;  // is the sound ambient (true) or positional (false)?
   
   // might not need (or use) these 3 following vars...
   bool isPlaying;
   int repeat;           // number of times to repeat (static)
   int repeat_countdown; // number of times left to repeat (changes during play)
};
#endif //SOUND_INFO_DATA
