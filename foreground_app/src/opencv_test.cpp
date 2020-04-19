#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv_test.h"

using namespace std;
using namespace cv;

void hello_opencv()
{
    Mat image;
	
	image = Mat::zeros(480, 640, CV_8UC1);
    putText(image, "Hello OpenCV",
            Point(20, 50),
            FONT_HERSHEY_COMPLEX, 1, // font face and scale
            Scalar(255, 255, 255), // white
            1, LINE_AA); // line thickness and type
	
    imshow("Sample", image);
	
}


