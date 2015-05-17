#pragma once
#include "sensordata.hpp"
#include "parking.hpp"
#include "imageprocessor/imageprocessor.hpp"

namespace Autodrive
{
    command lastCommand;

    bool speedChanged() 
    {
        return lastCommand.changedSpeed;
    }

    bool angleChanged()
    {
        return lastCommand.changedAngle;
    }

    double getSpeed()
    {
        return lastCommand.speed;
    }

    double getAngle()
    {
        return lastCommand.angle;
    }

    enum carstatus
    {
        DETECTING_GAP,PARKING,SEARCHING_FOR_LANES,FOLLOWING_LANES,UNKNOWN,DEBUG,DISTANCE_MEASURED_DEBUG
    };  
    
    carstatus initialStatus = SEARCHING_FOR_LANES;
    carstatus status = initialStatus;

    void setInitialStatus(carstatus newStatus)
    {
        initialStatus = newStatus;
        status = newStatus;
    }

    void resetStatus()
    {
       status = initialStatus;
    }
    
    int carRatio = 1;
    
    void setCarRatio(int ratio)
    {
        carRatio = ratio;
    }

    void drive()
    {
        /* Reset command */
        lastCommand = command();

        switch (status)
        {
            case Autodrive::SEARCHING_FOR_LANES:
                if (Autodrive::imageProcessor::init_processing(Autodrive::SensorData::image))
                {
                    lastCommand.setSpeed(0.28);
                    status = FOLLOWING_LANES;
                }
                break;
                
            case Autodrive::FOLLOWING_LANES:
                lastCommand = Autodrive::imageProcessor::continue_processing(*Autodrive::SensorData::image);
                break;
                
            // debug only! will be merged with lane following   
            case Autodrive::DETECTING_GAP:
                lastCommand = Parking::SetParkingProcedure();
                if(Parking::parkingProcedure == Parking::PERPENDICULAR_STANDARD){ // select parking procedure
                    status = PARKING;
                }else{
                    lastCommand.setSpeed(Maneuver::normalSpeed); 
                }
                break;
            // -----------
            
            case Autodrive::PARKING:
                //std::cout << "PARKING" << std::endl;
                lastCommand = Parking::Park();
                break; 
                
            case Autodrive::UNKNOWN:
                break;
                
            case Autodrive::DEBUG:
                lastCommand = Parking::Debug();
                break;
                
            case Autodrive::DISTANCE_MEASURED_DEBUG:
                lastCommand = Parking::DebugGapLength();
                break;
                
            default:
                break;
        }
    }
}
