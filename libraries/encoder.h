// Base class for incremental encoders
#ifndef ENCODER_H
#define ENCODER_H

#include <arduino.h>
#include <stdint.h>
#include "Filter.h"
#include "../trigint/Source/trigint.h"
#include "TypeDefs.h"

class Encoder
{  
    /**
     * Angles stored in 16,384 units per rev
     * Speed stored in above angle units/(1024 microseconds)
     */

    protected:
        trigint_angle_t angleNew;
        trigint_angle_t angleOld;
        int32_t dAngle;
        uint32_t timeNew;
        uint32_t timeOld;

    private:
        int_vel_t speedNew;
        int_vel_t speedOld;
        Filter<int_vel_t> *speedFilter;
    
    public:
        /**
         *  Constructor
         */
        Encoder(Filter<int_vel_t> *filter)
        :angleNew(0), angleOld(0), timeNew(0), timeOld(0),
            speedNew(0), speedOld(0), speedFilter(filter)
        {}

        /**
         * Samples encoder and returns angle in units of 16384 angle units/rev
         * Implementation of function will be different for each
         * encoder counter chip. Hence it is a virtual function.
         */
        virtual trigint_angle_t MeasureAngle() = 0;

        /**
         * Get a speed measurement by sampling current angle (calls MeasureAngle()))
         * and performing finite difference between new and last angle. 
         * Speed can only be positive so direction should be set correctly
         * Returns filtered result
         */
        int_vel_t MeasureSpeed()
        {
            angleOld = angleNew;
            timeOld = timeNew;
           
            timeNew = micros();
            MeasureAngle(); //Will update angleNew as well
            dAngle = static_cast<int16_t>(angleNew - angleOld);

            //handle case where angleNew is less than angleOld because of rollover
            dAngle = (dAngle < (-1*(TRIGINT_ANGLES_PER_CYCLE>>1))) ? 
                (dAngle + TRIGINT_ANGLES_PER_CYCLE) : dAngle;
            speedNew = ((dAngle << 10)/(timeNew - timeOld));
            return speedFilter->Update(speedNew);
            
        }

        /**
         * GetAngle() and GetSpeed() do not sample encoder again.
         * They simply return the last measured angle and speed respectively
         */
        inline trigint_angle_t GetAngle(){return angleNew;}
        inline int_vel_t GetSpeed(){return speedFilter->GetFilteredValue();}

};
#endif
