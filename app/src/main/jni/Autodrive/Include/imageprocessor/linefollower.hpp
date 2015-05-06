#include "line.hpp"
#include "roadlinebuilder.hpp"
#include "lightnormalizer.hpp"
#include <unistd.h>
#include <chrono>

namespace Autodrive
{
    class linefollower
    {
        RoadLine roadLine;
        int roadsize = 40;
    public:
        float targetRoadDistance = 0;
        std::unique_ptr<roadlinebuilder> roadBuilder = nullptr;

        void draw(cv::Mat* colorCopy, int centerX)
        {
            roadLine.draw(colorCopy);
           
            /* DRAW RECTANGLE FOR POSSIBLE FIRST HITS*/
            POINT upperLeft = roadBuilder->last_start + POINT(-Settings::leftIterationLength, Settings::firstFragmentMaxDist);
            POINT lowerRight = roadBuilder->last_start + POINT(Settings::rightIterationLength, 0);
            cv::rectangle(*colorCopy,upperLeft , lowerRight,cv::Scalar(255,0,255));
            
            //linef(roadBuilder->last_start, roadBuilder->last_start + POINT(8, -20)).draw(colorCopy, cv::Scalar(0, 255, 255), 1);

            /* DRAW VERTICAL LINE DISPLAYING DISTANCE TO ROAD AND TARGETED DISTANCE TO ROAD*/
            POINT offsetX = POINT(targetRoadDistance,0);
            POINT bottom_center = POINT(centerX, colorCopy->size().height);
            linef(bottom_center + offsetX, POINT(centerX, 0) + offsetX).draw(*colorCopy);
            offsetX.x = roadLine.getMeanStartDistance(5);
            if (int(offsetX.x) != 0)
                linef(bottom_center + offsetX, POINT(centerX, 0) + offsetX).draw(*colorCopy, cv::Scalar(255, 255, 0));
        }

        linefollower(const cv::Mat& cannied, POINT laneStartPoint, int center_x)
        {
            roadBuilder = make_unique<roadlinebuilder>(laneStartPoint, center_x);

            roadLine = roadBuilder->build(cannied, roadsize);

            targetRoadDistance = roadLine.getMeanStartDistance(5);
        }


        // Prerequicite for wheter a road is found or not
        bool isFound()
        {
            return roadLine.points.size() > 5 && fabs(roadLine.getMeanAngle() - Direction::FORWARD) < Mathf::PI_2;
        }

        float distanceDeviation()
        {
            if(!isFound())
                return targetRoadDistance;
            float startDistance = roadLine.getMeanStartDistance(5);
            return (startDistance - targetRoadDistance) * 1.1f;

        }


        float outMax=25;
        float outMin=-25;
        float error;
        float integral = 0;
        float previous_error=0;
        float derivate;
        float kp = Settings::kp;
        float ki = Settings::ki;
        float kd = Settings::kd;
        float Output;
        int degrees ;
        std::chrono::high_resolution_clock::time_point lastTime;
        bool first;

        template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

        optional<int> getPreferedAngle()
        {
            if (isFound())
            {
                /* Start by setting the target angle to the mean road angle*/
                //int degrees = Mathf::toDegrees(roadLine.getMeanAngle(4)) - 90;
                //degrees = int((degrees / 48.f) * 25);
                //degrees *= -1;
                // If the current distance is larger than, target distance, turn more right, vice versa
        
                //if(ITerm > outMax) {ITerm= outMax;}
                //else if(ITerm < outMin){ITerm = outMin;}

                auto newTime = std::chrono::high_resolution_clock::now();


                /*double timeDelta;

                if(first){
                    timeDelta = 1;
                    first = false;
                } else
                    timeDelta = std::chrono::duration_cast<std::chrono::nanoseconds >(newTime - lastTime).count() / 1000000000;
*/

                lastTime = newTime;

                error =  roadLine.getMeanStartDistance(5) - targetRoadDistance;
                integral = integral + error;
                derivate = (error - previous_error);
                Output = (kp * error) + (ki*integral) + (kd * derivate);
                previous_error = error;
        
                //if(Output > outMax) { Output = outMax;}
                //else if(Output < outMin) {Output= outMin;}

                //if(ITerm > outMax) {ITerm= outMax;}
                //else if(ITerm < outMin) {ITerm= outMin;}
    
                degrees=int(Output)*0.25;
                //std::cout << "setpoint: " <<SetPoint << std::endl;
                //std::cout << "Input: " << Input << std::endl;
                std::cout << "Output: " << Output<< std::endl;
                std::cout << "Degrees: " << degrees<<std::endl;

                usleep(1000);

                degrees = std::min(degrees, 25);
                degrees = std::max(degrees, -25);
                return degrees;
            }

            return nullptr;
        }

        void update(cv::Mat& cannied)
        {
            roadLine = roadBuilder->build(cannied, roadsize);
        }

    };

}

