#ifndef _VPR_STAT_COLLECTOR_H_
#define _VPR_STAT_COLLECTOR_H_

#include <vpr/vprConfig.h>

#include <vpr/Util/DateTime.h>
#include <vpr/Util/Interval.h>

#include <deque>
#include <utility>

namespace vpr
{

/** Statistics collection class
*
* STA - Short Term Average (a time limited average)
*/
template<class TYPE>
class StatCollector
{
public:
   /** Constructor
   * @param staMaxTime  The max age for samples in the sta computation
   */
   StatCollector(vpr::Interval staMaxTime=vpr::Interval(5, vpr::Interval::Sec) )
   {
      mCurTotal = 0;
      mSampleCount = 0;

      mMaxSTA = 0.0;
      mPrevSample1 = 0;
      mPrevSample2 = 0;

      mSTAMaxTime = staMaxTime;
   }

   void addSample(const TYPE sample);

   TYPE getTotal() const
   { return mCurTotal; }
   vpr::Uint32 getNumSamples() const
   { return mSampleCount; }

   /** Return Mean (value/second) */
   double getMean();
   double getInstAverage();
   double getSTA();
   double getMaxSTA() const
   {  return mMaxSTA; }

private:
   TYPE        mCurTotal;     // Running total of the data
   vpr::Uint32 mSampleCount;  // Number of samples taken

   vpr::Interval  mSTAMaxTime;   // Max age of a value used in short term average (STA)
   double         mMaxSTA;

   vpr::DateTime mInitialSampleTime;   // Time of first sample
   vpr::Interval mPrevSampleTime1, mPrevSampleTime2;      // Time of last 2 samples (mPrevST1 < mPrevSt2)
   TYPE mPrevSample1, mPrevSample2;                      // Previous samples

   std::deque< std::pair<TYPE,vpr::Interval> >  mSampleBuffer;    // Buffer of samples used to calc STA
};


template <class TYPE>
void StatCollector<TYPE>::addSample(const TYPE sample)
{
   mCurTotal += sample;
   mSampleCount += 1;

   vpr::Interval cur_time;
   cur_time.setNow();                     // Set current time

   // Set Init time
   if(mSampleCount == 1)              // First send -- INIT TIMES
   {
      mInitialSampleTime.setNow();    // Initialize first read time
   }

   // Update recent sample values for INST average
   mPrevSampleTime1 = mPrevSampleTime2;      // Time of last 2 samples (mPrevST1 < mPrevSt2)
   mPrevSampleTime2 = cur_time;
   mPrevSample1 = mPrevSample2;
   mPrevSample2 = sample;

   // Add value to sample buffer
   mSampleBuffer.push_front( std::pair<TYPE,vpr::Interval>(sample, cur_time) );
}

template <class TYPE>
double StatCollector<TYPE>::getMean()
{
   if(0 == mCurTotal)
      return 0.0f;

   vpr::DateTime cur_date_time, diff_date_time;
   double mean_result, diff_secs;

   cur_date_time.setNow();
   diff_date_time = cur_date_time - mInitialSampleTime;
   diff_secs = diff_date_time.getSecondsf();

   mean_result = double(mCurTotal)/diff_secs;

   return mean_result;
}

template <class TYPE>
double StatCollector<TYPE>::getInstAverage()
{
   //
   //     |
   //     |                  |
   //     |                  |
   // mPrevTime1        mPrevTime2             cur_time
   vpr::Interval cur_time, diff_time;
   double diff_sec;                       // Num secs different in send times
   cur_time.setNow();                     // Set current time
   double inst_average(0.0);

   // Compute -- INST BANDWIDTH
   diff_time = cur_time - mPrevSampleTime1;     // Get time to compute the average over
   diff_sec = diff_time.secf();
   if(diff_sec > 0)
   {
     inst_average = double(mPrevSample1 + mPrevSample1)/diff_sec;
   }
   if(diff_time > mSTAMaxTime)   // Haven't had sample in quite a while, so clamp to zero
   {
      inst_average = 0.0;
   }

   return inst_average;
}

template <class TYPE>
double StatCollector<TYPE>::getSTA()
{
   vpr::Interval cur_time, diff_time;
   double diff_sec;                       // Num secs different in send times
   cur_time.setNow();                     // Set current time

   // --- CULL OFF old values --- //
   bool got_cull_point(false);
   std::deque< std::pair<TYPE,vpr::Interval> >::iterator cull_from;
   cull_from = mSampleBuffer.end();

   // Go until I am pointing at an element that is within range
   // OR I am at the head of the queue
   while((!got_cull_point) && (cull_from != mSampleBuffer.begin()))
   {
      --cull_from;   // Go to next one to check
      diff_time = cur_time - (*cull_from).second;
      if(diff_time < mSTAMaxTime)
         got_cull_point = true;
   }

   // Cull the values
   // - If at beginning, then just clear the whole thing
   // - Else clear from cull pt to end
   if(cull_from == mSampleBuffer.begin())
   {
      mSampleBuffer.clear();
   }
   else
   {
      ++cull_from;         // Increment to get to the first value to cull
      if(cull_from != mSampleBuffer.end())
      {
         mSampleBuffer.erase(cull_from, mSampleBuffer.end());     // Erase the values
      }
   }

   // Compute -- STA BANDWIDTH
   double sta_value(0.0f);

   if(mSampleBuffer.size() > 0)
   {
      double sum(0.0f);
      vpr::Interval first_sample_time(mSampleBuffer[0].second);

      std::deque< std::pair<TYPE,vpr::Interval> >::iterator cur_sample(mSampleBuffer.begin());

      for( ; cur_sample != mSampleBuffer.end(); ++cur_sample)       // Sum the total bandwidth up
      {  sum += (*cur_sample).first; }

      diff_time = cur_time - first_sample_time;
      diff_sec = diff_time.secf();
      sta_value = sum/diff_sec;

      if(sta_value > mMaxSTA)                        // Check for new max
           mMaxSTA = sta_value;
   }

   return sta_value;
}




}; // namespace vpr
#endif
