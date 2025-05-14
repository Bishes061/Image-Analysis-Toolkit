#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

const string imagePath = "/Users/bishesh/Desktop/Intern/opencv-setup/scene.jpg";
const string mainWindow = "Image Viewer";
const string zoomWindow = "Zoomed View";
const string controlWindow = "Zoom Control";

Mat originalImage;
int zoomSlider = 1; // Default zoom 1x
const int maxZoom = 10; // Max zoom 10x
int zoomSize = 200; // Size of the zoomed view


void onMouse(int event, int x, int y, int, void*) {
    if (event != EVENT_MOUSEMOVE)
        return;

    int scale = zoomSlider;  // Integer zoom factor
    if (scale < 1) scale = 1;

    int cropSize = static_cast<int>(zoomSize / static_cast<double>(scale));
    int x1 = max(0, min(x - cropSize / 2, originalImage.cols - cropSize));
    int y1 = max(0, min(y - cropSize / 2, originalImage.rows - cropSize));

    Rect roi(x1, y1, cropSize, cropSize);
    Mat cropped = originalImage(roi);

    Mat zoomed;
    resize(cropped, zoomed, Size(zoomSize, zoomSize), 0, 0, INTER_LINEAR);

    moveWindow(zoomWindow, x + 20, y + 20);
    imshow(zoomWindow, zoomed);
}

void onZoomChange(int, void*) {}

int main() {
    originalImage = imread(imagePath);
    if (originalImage.empty()) {
        cerr << "Error loading image: " << imagePath << endl;
        return -1;
    }

    namedWindow(mainWindow);
    namedWindow(zoomWindow);
    resizeWindow(zoomWindow, zoomSize, zoomSize);
    setMouseCallback(mainWindow, onMouse);

    namedWindow(controlWindow);
    createTrackbar("Zoom (1x-10x)", controlWindow, &zoomSlider, maxZoom, onZoomChange);

    imshow(mainWindow, originalImage);
    waitKey(0);
    destroyAllWindows();
    return 0;
}




// #include <opencv2/opencv.hpp>
// #include <iostream>
// #include <string>

// using namespace cv;
// using namespace std;

// // Global variables
// Mat image;             // Original image
// Mat magnifiedWindow;   // Magnified window
// int zoomFactor = 2;    // Default zoom factor
// const int MAX_ZOOM = 10;
// const int MIN_ZOOM = 1;
// bool isRunning = true;
// const string controlWindowName = "Zoom Controls";

// // Mouse callback function for the original image
// void onMouse(int event, int x, int y, int flags, void* userdata)
// {
//     if (!isRunning || image.empty())
//         return;

//     // Only process if mouse is moving
//     if (event == EVENT_MOUSEMOVE)
//     {
//         // Create a region of interest (ROI) around the mouse cursor
//         int roiWidth = magnifiedWindow.cols / zoomFactor;
//         int roiHeight = magnifiedWindow.rows / zoomFactor;
        
//         // Calculate top-left corner of ROI
//         int roiX = max(0, min(x - roiWidth / 2, image.cols - roiWidth));
//         int roiY = max(0, min(y - roiHeight / 2, image.rows - roiHeight));
        
//         // Extract ROI from original image
//         Rect roi(roiX, roiY, roiWidth, roiHeight);
//         Mat roiImage = image(roi);
        
//         // Resize ROI to fit magnification window
//         resize(roiImage, magnifiedWindow, magnifiedWindow.size(), 0, 0, INTER_LINEAR);
        
//         // Draw a rectangle on the magnified image to show the original size
//         rectangle(magnifiedWindow, Point(0, 0), Point(magnifiedWindow.cols - 1, magnifiedWindow.rows - 1), 
//                   Scalar(0, 255, 0), 2);
        
//         // Display position information
//         string posText = "Position: (" + to_string(x) + ", " + to_string(y) + ")";
//         string zoomText = "Zoom: " + to_string(zoomFactor) + "x";
//         putText(magnifiedWindow, posText, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
//         putText(magnifiedWindow, zoomText, Point(10, 40), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
        
//         // Update the magnified window
//         imshow("Magnified View", magnifiedWindow);
//     }
// }

// // Callback function for the zoom slider
// void onZoomChange(int value, void* userdata)
// {
//     // Ensure zoom factor is between MIN_ZOOM and MAX_ZOOM
//     zoomFactor = max(MIN_ZOOM, min(MAX_ZOOM, value));
//     cout << "Zoom factor set to " << zoomFactor << "x" << endl;
// }

// int main(int argc, char** argv)
// {
//     string imagePath;
//     cout << "Please enter the path to an image: ";
//     getline(cin, imagePath);
    
//     // Load the image
//     image = imread(imagePath);
    
//     // Check if image was successfully loaded
//     if (image.empty())
//     {
//         cout << "Error: Could not load image at " << imagePath << endl;
//         return -1;
//     }
    
//     // Create windows
//     namedWindow("Original Image", WINDOW_AUTOSIZE);
//     namedWindow("Magnified View", WINDOW_AUTOSIZE);
//     namedWindow(controlWindowName, WINDOW_AUTOSIZE);
    
//     // Create trackbar for zoom control
//     createTrackbar("Zoom (1-10x)", controlWindowName, &zoomFactor, MAX_ZOOM, onZoomChange);
    
//     // Set initial zoom slider value
//     setTrackbarPos("Zoom (1-10x)", controlWindowName, zoomFactor);
    
//     // Calculate magnified window size - make it a reasonable size
//     int magnifiedWidth = min(640, image.cols);
//     int magnifiedHeight = min(480, image.rows);
//     magnifiedWindow = Mat::zeros(magnifiedHeight, magnifiedWidth, image.type());
    
//     // Set mouse callback for the original image window
//     setMouseCallback("Original Image", onMouse);
    
//     // Display the original image
//     imshow("Original Image", image);
    
//     // Create control panel background
//     Mat controlPanel = Mat::zeros(100, 400, CV_8UC3);
//     controlPanel.setTo(Scalar(50, 50, 50));  // Dark gray background
//     imshow(controlWindowName, controlPanel);
    
//     // Main loop
//     while (isRunning)
//     {
//         // Display the original image
//         imshow("Original Image", image);
        
//         // Wait for a key press
//         int key = waitKey(20);
        
//         // Process key presses
//         if (key == 27) // ESC key
//         {
//             isRunning = false;
//         }
//     }
    
//     // Clean up
//     destroyAllWindows();
    
//     return 0;
// }