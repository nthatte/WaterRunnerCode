// Base class for incremental encoders
#ifndef ENCODER_H
#define ENCODER_H

#include <arduino.h>
#include <stdint.h>
#include "Filter.h"
#include "../trigint/Source/trigint.h"
class Encoder
{   
    protected:
        /*
        Angles stored in 16,384 units per rev
        Speed stored in above angle units/second
        */
        trigint_angle_t angleNew;
        trigint_angle_t angleOld;
        int32_t dAngle;
        uint32_t timeNew;
        uint32_t timeOld;
        int32_t speedNew;
        int32_t speedOld;
        Filter<int32_t> *speedFilter;
    
    public:
        Encoder(Filter<int32_t> *filter)
        :angleNew(0), angleOld(0), timeNew(0), timeOld(0),
            speedNew(0), speedOld(0), speedFilter(filter)
        {}

        /*
        Implementation of MeasureAngle() function will be different for each
        encoder counter chip. Hence it is a virtual function.
        */
        virtual trigint_angle_t MeasureAngle() = 0;

        /*
        Get a speed measurement by taking finite difference between new and last angle. 
        Speed can only be positive so direction should be set correctly
        Returns filtered result
        */
        int32_t MeasureSpeed()
        {
            angleOld = angleNew;
            timeOld = timeNew;
           
            timeNew = micros();
            MeasureAngle(); //Will update angleNew as well
            dAngle = static_cast<int16_t>(angleNew - angleOld);

            //handle case where angleNew is less than angleOld because of rollover
            dAngle = (dAngle < (-1*TRIGINT_ANGLES_PER_CYCLE/2)) ? 
                (dAngle + TRIGINT_ANGLES_PER_CYCLE) : dAngle;
            speedNew = dAngle*1e6/(timeNew - timeOld);
            
            return speedFilter->Update(speedNew);
            
        }

        trigint_angle_t GetAngle(){return angleNew;}
        int32_t GetSpeed(){return speedFilter->GetFilteredValue();}

};
#endif
