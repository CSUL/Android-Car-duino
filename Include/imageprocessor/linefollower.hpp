#include "line.hpp"
#include "roadlinebuilder.hpp"
#include "lightnormalizer.hpp"

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


        float outMax=100;
        float outMin=-100;
        float Error;
        float ITerm;
        float lastInput=0;
        float dInput;
        float SetPoint;
        float Input;
        float Kp=0.1;
        float Ki=0.5;
        float Kd=0.2;
        float Output;
        int degrees ;

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
                SetPoint = targetRoadDistance;
                Input = roadLine.getMeanStartDistance(10);
                Error=Input-SetPoint;
                ITerm=(Ki * Error);

                if(ITerm > outMax) {ITerm= outMax;}
                else if(ITerm < outMin){ITerm = outMin;}

                dInput=(Input - lastInput);
                Output = (Kp * Error) + (Kp*ITerm)- (Kp*Kd * dInput);

                if(Output > outMax) { Output = outMax;}
                else if(Output < outMin) {Output= outMin;}

                lastInput= Input;
                if(ITerm > outMax) {ITerm= outMax;}
                else if(ITerm < outMin) {ITerm= outMin;}

                lastInput=Input;
                degrees=int(Output*0.25);
                std::cout << "setpoint: " <<SetPoint << std::endl;
                std::cout << "Input: " << Input << std::endl;
                std::cout << "Output: " << Output<< std::endl;
                std::cout << "Degrees: " << degrees<<std::endl;

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
