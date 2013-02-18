/**
 * @function  shape_detection_v1.cpp
 * @brief		Code to recognize shapes.
 * @author 		Axolotl OpenCV team
 * @info		Stypi link: https://www.stypi.com/jmogpsai  
 */
 
//  to compile    g++ -o test cam_cvCapture.cpp `pkg-config opencv --cflags --libs`
//                ./test
 
 /****************************************** 
 Pseudocode:
 -----------
 
0. Create black image with white polygon
    * Do this outside in some paint program
    * Kyle wants to import image into color filter and then

1. Read image
	* Use imread function
	
2. Determine contours/vertices
	* Use Canny edge detection, Hough, and/or findContours
	* Use approxPolyDP to approximate found contours as lines
		* findContours has a lot of noise, but the other line generators should
		 	have their own noise reduction algorithms
		* Try code with and without this function to see how it affects things

3. Draw contours (Debugging)
	* Generates an image that shows the picture with the contours outlined

4. Figure out shape
	* Output number of vertices found
	* Label each found vertex on the screen with its coordinates

5. Output center coordinate
	* Label on the image
*********************************************/

//Included OpenCV libraries
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//Included standard libraries
#include <ios>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

//Object Declarations
Mat src; Mat src_gray; Mat contours;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345); 

//Function Declarations
Mat detect_edges(Mat bin_img); // step 2
void detect_vertices(); // step 2
void contour_draw( Mat source, Mat contours, RNG rng );
Mat find_centers(Mat frame_thresh); // step 5

Scalar gen_rand_color(RNG rng);
    // Please insert one shape at a time; should miraculously determine the center.


//Main Caller
int main( int argc, char** argv )
    // argc declares number of arguments input
    // argv lets you instert as many arguments as you want as a vector
        // argv[0] is the name of your program
        // argv[1] should be the filename
{

    // Load source image and force it to three-channel color
    src = imread( argv[1], 1 ); 
    if(!src.data) cerr << "ERROR: could not open image file" << endl;
    
    cout << "input =" << endl << src << endl << endl; /*Print multichannel image,
    	assuming MAT loads three-channel images as a multi-channel array*/
    
	// Detect edges (contours)
	
    contours = detect_edges( src );
    
    // Draw edges (contours)
    
    contour_draw( src, contours, rng );

    waitKey(0);
	return 0;
}

/**************************************
 * Functions                          *
 *************************************/
// @author: Kyle Miller ::: 13/01/25 @1300
// @i/o:    Input binary image
//          Output contour Mat

Mat detect_edges(Mat bin_img)
{
    Mat contours;
    vector<Vec2f> lines;
    
    Canny(bin_img,contours,125,350,3,1); //apply Canny filter
    HoughLines(contours,lines,1,3.14/180,80); //apply Hough lines algorithm
    
    //below from book
    vector<Vec2f>::const_iterator it= lines.begin(); 
    while (it!=lines.end()) 
    {
        float rho= (*it)[0];    // first element is distance rho 
        float theta= (*it)[1]; // second element is angle theta
        if (theta < 3.14/4. || theta > 3.*3.14/4.) // ~vertical line
        {
            Point pt1(rho/cos(theta),0);  // point of intersection of the line with first row 
            Point pt2((rho-bin_img.rows*sin(theta))/cos(theta),bin_img.rows); // point of intersection of the line with last row
            line( bin_img, pt1, pt2, cv::Scalar(255), 1);
        } //if 
        else 
        { 
            Point pt1(0,rho/sin(theta)); // point of intersection of the line with first column 
            Point pt2(bin_img.cols,(rho-bin_img.cols*cos(theta))/sin(theta)); // point of intersection of the line with last column
            line(bin_img, pt1, pt2, Scalar(255), 1);
        }//else
        ++it;
    }//while
    
	cout << "contours =" << endl << contours << endl << endl;

	// Create new window
    //namedWindow("Try Drawing Contours", CV_WINDOW_AUTOSIZE); // CV_WINDOW_AUTOSIZE adjusts window size to fit image    
    
    // Show in created window
    imshow( "Drawing Contours", contours );

	return contours;
}//detect_edges

Scalar gen_rand_color(RNG rng)
// @author: Darryl McGowan ::: 13/01/25 @1300
// @i/o:    Input RNG object
//          Output Scalar type object that represents a color
{
     // generate uniformly distributed random color
    Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
   
    return color;
}

void contour_draw( Mat source, Mat contours, RNG rng) // step 3; FULLY IMPLEMENTED
// @author: Darryl McGowan ::: 13/01/25 @1300
// @i/o:    Input Mat type source containing the 2D matrix of the image of interest.
//                Mat type contours containing at 2D matrix of point vectors
//          Output new window with detected contours drawn in it
{
    Mat contour_mat; // Mat object that will store contoured image

    contour_mat = Mat::zeros( source.size(), CV_8UC3 ); // Initialize matrix to zeros. same size as source img.
                                                        // Assuming 8 bit image with 3 channels 
    
    // Draw contours in new window
    for( int i = 0; i < contours.rows; i++ )
    {
        // copied arguments 5-8 from general_Contours_demo3.cpp (thickness, linetype, hierachy, maxlevel, offset)
        // http://docs.opencv.org/doc/tutorials/imgproc/shapedescriptors/find_contours/find_contours.html
        drawContours( contour_mat, contours, i, gen_rand_color( rng ), 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    // Create new window
    namedWindow("Found Contours", CV_WINDOW_AUTOSIZE); // CV_WINDOW_AUTOSIZE adjusts window size to fit image    
    
    // Show in created window
    imshow( "Contours", contour_mat );
    
}

void detect_vertices() // step 2
// @author: Sondra Miller ::: 13/01/25 @1350
// @i/o:    Input Mat type frame_thresh containing the contours of interest
//          Output Mat type frame_thresh that contains circled edges
// @brief:  
{
    //http://stackoverflow.com/questions/7263621/how-to-find-corners-on-a-image-using-opencv
    //http://stackoverflow.com/questions/8667818/opencv-c-obj-c-detecting-a-sheet-of-paper-square-detection/14368605#14368605
    //http://sudokugrab.blogspot.com/2009/07/how-does-it-all-work.html
}

/*Mat find_centers( Mat frame_thresh ) // step 5
// @author: Sondra Miller ::: 13/01/25 @1300
// @i/o:    Input Mat type frame_thresh containing the shape of interest.
//          Output Mat type frame_thresh that contains shape and text in upper corner
//              indicating the coordinates of the center, as well as a point
//              indicating the center.
// @brief:  
{
	int *shape_center_coordinates;
    asprintf( &shape_center_coordinates,"%d,%d", frame_thresh.center.x-320, center.y-240);
    putText(frame_thresh, shape_center_coordinates , center, CV_FONT_HERSHEY_SIMPLEX,
        0.5, Scalar(255, 255, 255), 1, 8, 0);
    namedWindow("Coordinate Centers", CV_WINDOW_AUTOSIZE);
}*/
