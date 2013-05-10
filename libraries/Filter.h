/*
Filter.h

Filter interface to smooth measurements via
algorithms such as Running Avg, Butterworth Filter
*/

#ifndef FILTER_H
#define FILTER_H

template<typename T>
class Filter
{
    public:
        /*
        updates filtered measurement given new value 
        and returns filtered measurement
        */
        virtual T Update(const T value) = 0;

        //returns current filtered measurement
        virtual T GetFilteredValue() = 0;
};
#endif
