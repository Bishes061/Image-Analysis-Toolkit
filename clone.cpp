//FINAL CODE

#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace cv;
using namespace std;

int blockSlider = 2; // 2^2 = 4
int stepSlider = 4;
int detailSlider = 97; // Slider value (detailThreshold = detailSlider / 10.0)
int minDistSlider = 20;
int clusterSlider = 3;
int showQuantized = 0;

const int maxBlockSlider = 6; // up to 64 block size
const int maxDetail = 200;    // corresponds to 20.0
const int maxZoom = 10;
int zoomSlider = 1;
int zoomSize = 200;

Mat originalImage, annotatedImage, quantizedDisplay;

struct ClonePair {
    Point src;
    Point dst;
    Point displacement() const {
        return Point(dst.x - src.x, dst.y - src.y);
    }
};

string blockToKey(const Mat& block, Mat& smallOut) {
    Mat gray, small;
    cvtColor(block, gray, COLOR_BGR2GRAY);
    resize(gray, small, Size(4, 4));
    smallOut = small.clone();
    stringstream ss;
    for (int i = 0; i < small.rows; i++) {
        for (int j = 0; j < small.cols; j++) {
            ss << (int)(small.at<uchar>(i, j) / 16) << ",";
        }
    }
    return ss.str();
}

double computeDetail(const Mat& block) {
    Mat gray, lap;
    cvtColor(block, gray, COLOR_BGR2GRAY);
    Laplacian(gray, lap, CV_64F);
    Scalar mu, sigma;
    meanStdDev(lap, mu, sigma);
    return sigma[0];
}

double euclideanDistance(Point a, Point b) {
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

vector<vector<ClonePair>> clusterClones(const vector<ClonePair>& pairs, int minClusterSize, double directionTolerance = 5.0) {
    vector<vector<ClonePair>> clusters;
    vector<bool> used(pairs.size(), false);

    for (size_t i = 0; i < pairs.size(); i++) {
        if (used[i]) continue;
        vector<ClonePair> cluster = { pairs[i] };
        used[i] = true;
        Point refDisp = pairs[i].displacement();

        for (size_t j = i + 1; j < pairs.size(); j++) {
            if (used[j]) continue;
            Point d = pairs[j].displacement();
            if (abs(d.x - refDisp.x) <= directionTolerance && abs(d.y - refDisp.y) <= directionTolerance) {
                cluster.push_back(pairs[j]);
                used[j] = true;
            }
        }

        if (cluster.size() >= minClusterSize) {
            clusters.push_back(cluster);
        }
    }
    return clusters;
}

void detectClones() {
    int blockSize = (1 << blockSlider);
    double detailThreshold = detailSlider / 10.0;
    int stepSize = stepSlider;
    int minDistance = minDistSlider;
    int minClusterSize = clusterSlider;

    unordered_map<string, Point> blockMap;
    vector<ClonePair> candidatePairs;

    annotatedImage = originalImage.clone();
    quantizedDisplay = originalImage.clone();

    for (int y = 0; y <= originalImage.rows - blockSize; y += stepSize) {
        for (int x = 0; x <= originalImage.cols - blockSize; x += stepSize) {
            Rect roi(x, y, blockSize, blockSize);
            Mat block = originalImage(roi);

            double detail = computeDetail(block);
            if (detail < detailThreshold)
                continue;

            Mat compressed;
            string key = blockToKey(block, compressed);

            if (blockMap.find(key) != blockMap.end()) {
                Point orig = blockMap[key];
                Point curr = Point(x, y);
                if (euclideanDistance(orig, curr) < minDistance)
                    continue;

                candidatePairs.push_back({ orig, curr });
            } else {
                blockMap[key] = Point(x, y);
            }

            Mat avgBlock;
            resize(compressed, avgBlock, Size(blockSize, blockSize), 0, 0, INTER_NEAREST);
            cvtColor(avgBlock, avgBlock, COLOR_GRAY2BGR);
            avgBlock.copyTo(quantizedDisplay(roi));
        }
    }

    auto clusters = clusterClones(candidatePairs, minClusterSize);

    for (const auto& cluster : clusters) {
        for (const auto& pair : cluster) {
            Rect srcRect(pair.src.x, pair.src.y, blockSize, blockSize);
            Rect dstRect(pair.dst.x, pair.dst.y, blockSize, blockSize);
            rectangle(annotatedImage, srcRect, Scalar(0, 255, 0), 2);
            rectangle(annotatedImage, dstRect, Scalar(255, 0, 255), 2);
            line(annotatedImage,
                Point(pair.src.x + blockSize / 2, pair.src.y + blockSize / 2),
                Point(pair.dst.x + blockSize / 2, pair.dst.y + blockSize / 2),
                Scalar(255, 255, 255), 1);
        }
    }
}

void onSliderChange(int, void*) {
    detectClones();
    if (showQuantized == 1) {
        imshow("Clone Detector", quantizedDisplay);
    } else {
        imshow("Clone Detector", annotatedImage);
    }
}

void onMouse(int event, int x, int y, int, void*) {
    if (event != EVENT_MOUSEMOVE)
        return;

    int scale = zoomSlider;
    if (scale < 1) scale = 1;

    int cropSize = static_cast<int>(zoomSize / static_cast<double>(scale));
    int x1 = max(0, min(x - cropSize / 2, originalImage.cols - cropSize));
    int y1 = max(0, min(y - cropSize / 2, originalImage.rows - cropSize));

    Rect roi(x1, y1, cropSize, cropSize);
    Mat cropped = originalImage(roi);

    Mat zoomed;
    resize(cropped, zoomed, Size(zoomSize, zoomSize), 0, 0, INTER_LINEAR);

    moveWindow("Zoom View", x + 20, y + 20);
    imshow("Zoom View", zoomed);
}

int main() {
    originalImage = imread("/Users/bishesh/Desktop/Intern/opencv-setup/combined.png");
    if (originalImage.empty()) {
        cerr << "Could not open image" << endl;
        return -1;
    }

    namedWindow("Clone Detector", WINDOW_AUTOSIZE);
    namedWindow("Zoom View", WINDOW_NORMAL);
    resizeWindow("Zoom View", zoomSize, zoomSize);
    setMouseCallback("Clone Detector", onMouse);

    createTrackbar("Show Quantized", "Clone Detector", &showQuantized, 1, onSliderChange);
    createTrackbar("Block Size (2^n)", "Clone Detector", &blockSlider, maxBlockSlider, onSliderChange);
    createTrackbar("Step Size", "Clone Detector", &stepSlider, 20, onSliderChange);
    createTrackbar("Detail Threshold", "Clone Detector", &detailSlider, maxDetail, onSliderChange);
    createTrackbar("Min Distance", "Clone Detector", &minDistSlider, 100, onSliderChange);
    createTrackbar("Cluster Size", "Clone Detector", &clusterSlider, 10, onSliderChange);
    createTrackbar("Zoom (1x-10x)", "Clone Detector", &zoomSlider, maxZoom);

    detectClones();
    imshow("Clone Detector", annotatedImage);

    waitKey(0);
    return 0;
}



// #include <opencv2/opencv.hpp>
// #include <unordered_map>
// #include <vector>
// #include <iostream>
// #include <sstream>
// #include <cmath>

// using namespace cv;
// using namespace std;

// int blockSlider = 2; // 2^2 = 4
// int stepSize = 4;
// double detailThreshold = 9.7;
// int minDistance = 20;
// int minClusterSize = 3;
// int quantizationLevel = 16; // Controls similarity threshold
// int showQuantized = 0;

// Mat img, result, quantizedDisplay;

// struct ClonePair {
//     Point src;
//     Point dst;
//     Point displacement() const {
//         return Point(dst.x - src.x, dst.y - src.y);
//     }
// };

// string blockToKey(const Mat& block, Mat& smallOut, int quantLevel) {
//     Mat gray, small;
//     cvtColor(block, gray, COLOR_BGR2GRAY);
//     resize(gray, small, Size(4, 4));
//     smallOut = small.clone();
//     stringstream ss;
//     for (int i = 0; i < small.rows; i++) {
//         for (int j = 0; j < small.cols; j++) {
//             ss << (int)(small.at<uchar>(i, j) / quantLevel) << ",";
//         }
//     }
//     return ss.str();
// }

// double computeDetail(const Mat& block) {
//     Mat gray, lap;
//     cvtColor(block, gray, COLOR_BGR2GRAY);
//     Laplacian(gray, lap, CV_64F);
//     Scalar mu, sigma;
//     meanStdDev(lap, mu, sigma);
//     return sigma[0];
// }

// double euclideanDistance(Point a, Point b) {
//     return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
// }

// vector<vector<ClonePair>> clusterClones(const vector<ClonePair>& pairs, int minClusterSize, double directionTolerance = 5.0) {
//     vector<vector<ClonePair>> clusters;
//     vector<bool> used(pairs.size(), false);

//     for (size_t i = 0; i < pairs.size(); i++) {
//         if (used[i]) continue;
//         vector<ClonePair> cluster = { pairs[i] };
//         used[i] = true;
//         Point refDisp = pairs[i].displacement();

//         for (size_t j = i + 1; j < pairs.size(); j++) {
//             if (used[j]) continue;
//             Point d = pairs[j].displacement();
//             if (abs(d.x - refDisp.x) <= directionTolerance && abs(d.y - refDisp.y) <= directionTolerance) {
//                 cluster.push_back(pairs[j]);
//                 used[j] = true;
//             }
//         }

//         if (cluster.size() >= minClusterSize) {
//             clusters.push_back(cluster);
//         }
//     }
//     return clusters;
// }

// void detectClones() {
//     if (img.empty()) return;

//     int blockSize = 1 << (blockSlider + 2);
//     result = img.clone();
//     quantizedDisplay = img.clone();
//     unordered_map<string, Point> blockMap;
//     vector<ClonePair> candidatePairs;

//     for (int y = 0; y <= img.rows - blockSize; y += stepSize) {
//         for (int x = 0; x <= img.cols - blockSize; x += stepSize) {
//             Rect roi(x, y, blockSize, blockSize);
//             Mat block = img(roi);

//             double detail = computeDetail(block);
//             if (detail < detailThreshold)
//                 continue;

//             Mat compressed;
//             string key = blockToKey(block, compressed, quantizationLevel);

//             if (blockMap.find(key) != blockMap.end()) {
//                 Point orig = blockMap[key];
//                 Point curr = Point(x, y);
//                 if (euclideanDistance(orig, curr) < minDistance)
//                     continue;

//                 candidatePairs.push_back({orig, curr});
//             } else {
//                 blockMap[key] = Point(x, y);
//             }

//             Mat avgBlock;
//             resize(compressed, avgBlock, Size(blockSize, blockSize), 0, 0, INTER_NEAREST);
//             cvtColor(avgBlock, avgBlock, COLOR_GRAY2BGR);
//             avgBlock.copyTo(quantizedDisplay(roi));
//         }
//     }

//     auto clusters = clusterClones(candidatePairs, minClusterSize);

//     for (const auto& cluster : clusters) {
//         for (const auto& pair : cluster) {
//             Rect srcRect(pair.src.x, pair.src.y, blockSize, blockSize);
//             Rect dstRect(pair.dst.x, pair.dst.y, blockSize, blockSize);
//             rectangle(result, srcRect, Scalar(0, 255, 0), 2);
//             rectangle(result, dstRect, Scalar(255, 0, 255), 2);
//             line(result,
//                  Point(pair.src.x + blockSize/2, pair.src.y + blockSize/2),
//                  Point(pair.dst.x + blockSize/2, pair.dst.y + blockSize/2),
//                  Scalar(255, 255, 255), 1);
//         }
//     }

//     string label = format("BlockSize: %d | Detail Threshold: %.1f | Cluster: %d | Quantization Level: %d", 
//                            blockSize, detailThreshold, minClusterSize, quantizationLevel);
//     putText(showQuantized ? quantizedDisplay : result, label, Point(10, 25), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);

//     imshow("Clone Detector", showQuantized ? quantizedDisplay : result);
// }

// void onSliderChange(int, void*) {
//     detectClones();
// }

// int main() {
//     img = imread("/Users/bishesh/Desktop/Intern/opencv-setup/combined.png");
//     if (img.empty()) {
//         cout << "Could not open image" << endl;
//         return -1;
//     }

//     namedWindow("Clone Detector", WINDOW_AUTOSIZE);

//     createTrackbar("Show Quantized", "Clone Detector", &showQuantized, 1, onSliderChange);
//     createTrackbar("Block Size", "Clone Detector", &blockSlider, 4, onSliderChange);
//     createTrackbar("Detail Threshold", "Clone Detector", 0, 100, [](int v, void*) {
//         detailThreshold = v / 10.0;
//         detectClones();
//     });
//     setTrackbarPos("Detail Threshold", "Clone Detector", (int)(detailThreshold * 10));

//     createTrackbar("Min Cluster Size", "Clone Detector", &minClusterSize, 10, onSliderChange);
//     createTrackbar("Quantization Level", "Clone Detector", &quantizationLevel, 64, onSliderChange);

//     detectClones();
//     waitKey(0);
//     return 0;
// }





// #include <opencv2/opencv.hpp>
// #include <unordered_map>
// #include <iostream>
// #include <sstream>

// using namespace cv;
// using namespace std;

// string blockToKey(const Mat& block) {
//     stringstream ss;
//     for (int i = 0; i < block.rows; i++) {
//         for (int j = 0; j < block.cols; j++) {
//             Vec3b pixel = block.at<Vec3b>(i, j);
//             ss << (int)pixel[0] << "," << (int)pixel[1] << "," << (int)pixel[2] << ";";
//         }
//     }
//     return ss.str();
// }

// int main() {
//     // Load image
//     Mat img = imread("/Users/bishesh/Desktop/Intern/opencv-setup/scene.jpg");
//     if (img.empty()) {
//         cout << "Could not open image" << endl;
//         return -1;
//     }

//     // Parameters
//     int blockSize = 16;  // You can set this to 8, 16, etc.
//     unordered_map<string, Point> blockMap;
//     Mat result = img.clone();

//     for (int y = 0; y < img.rows - blockSize; y++) {
//         for (int x = 0; x < img.cols - blockSize; x++) {
//             Rect roi(x, y, blockSize, blockSize);
//             Mat block = img(roi);

//             string key = blockToKey(block);
//             if (blockMap.find(key) != blockMap.end()) {
//                 // Found a duplicate block
//                 Point orig = blockMap[key];
//                 rectangle(result, roi, Scalar(255, 0, 255), 2);  // Current clone block
//                 rectangle(result, Rect(orig.x, orig.y, blockSize, blockSize), Scalar(0, 0, 255), 2);  // Original
//                 line(result, Point(x + blockSize/2, y + blockSize/2),
//                             Point(orig.x + blockSize/2, orig.y + blockSize/2), Scalar(255, 255, 255), 1);
//             } else {
//                 blockMap[key] = Point(x, y);
//             }
//         }
//     }

//     // Show results
//     imshow("Detected Clones", result);
//     waitKey(0);
//     return 0;
// }


// COMPRESSION
// #include <opencv2/opencv.hpp>
// #include <unordered_map>
// #include <iostream>
// #include <sstream>

// using namespace cv;
// using namespace std;

// // Generate a fuzzy key using grayscale + downsample + quantization
// string blockToKey(const Mat& block) {
//     Mat gray, small;
//     cvtColor(block, gray, COLOR_BGR2GRAY);          // Convert to grayscale
//     resize(gray, small, Size(4, 4));                 // Downsample to 4x4

//     stringstream ss;
//     for (int i = 0; i < small.rows; i++) {
//         for (int j = 0; j < small.cols; j++) {
//             // Quantize pixel value to reduce sensitivity (e.g., 0–255 → 0–15)
//             ss << (int)(small.at<uchar>(i, j) / 16) << ",";
//         }
//     }
//     return ss.str();
// }

// int main() {
//     // Load image
//     Mat img = imread("/Users/bishesh/Desktop/Intern/opencv-setup/scene.jpg");
//     if (img.empty()) {
//         cout << "Could not open image" << endl;
//         return -1;
//     }

//     // Parameters
//     int blockSize = 16;  // Adjust block size to tune sensitivity
//     unordered_map<string, Point> blockMap;
//     Mat result = img.clone();

//     for (int y = 0; y < img.rows - blockSize; y += 4) {  // Step size can be < blockSize for overlap
//         for (int x = 0; x < img.cols - blockSize; x += 4) {
//             Rect roi(x, y, blockSize, blockSize);
//             Mat block = img(roi);

//             string key = blockToKey(block);

//             if (blockMap.find(key) != blockMap.end()) {
//                 // Found a duplicate (fuzzy matched)
//                 Point orig = blockMap[key];
//                 rectangle(result, roi, Scalar(255, 0, 255), 2);  // Current clone
//                 rectangle(result, Rect(orig.x, orig.y, blockSize, blockSize), Scalar(0, 0, 255), 2);  // Original
//                 line(result, Point(x + blockSize/2, y + blockSize/2),
//                             Point(orig.x + blockSize/2, orig.y + blockSize/2), Scalar(255, 255, 255), 1);
//             } else {
//                 blockMap[key] = Point(x, y);
//             }
//         }
//     }

//     // Show results
//     imshow("Detected Clones (Fuzzy)", result);
//     waitKey(0);
//     return 0;
// }



// FILTERING
// #include <opencv2/opencv.hpp>
// #include <unordered_map>
// #include <iostream>
// #include <sstream>

// using namespace cv;
// using namespace std;

// // Parameters
// int blockSize = 16;         // Size of each block
// int stepSize = 4;           // How much to slide the window
// int detailThreshold = 18; // Minimum high-frequency energy to consider "interesting"

// // Generate a fuzzy key using grayscale + downsample + quantization
// string blockToKey(const Mat& block) {
//     Mat gray, small;
//     cvtColor(block, gray, COLOR_BGR2GRAY);
//     resize(gray, small, Size(4, 4));  // Downsample

//     stringstream ss;
//     for (int i = 0; i < small.rows; i++) {
//         for (int j = 0; j < small.cols; j++) {
//             ss << (int)(small.at<uchar>(i, j) / 16) << ",";
//         }
//     }
//     return ss.str();
// }

// // Compute detail level using Laplacian (high-pass energy)
// double computeDetail(const Mat& block) {
//     Mat gray, lap;
//     cvtColor(block, gray, COLOR_BGR2GRAY);
//     Laplacian(gray, lap, CV_64F);  // High-frequency emphasis
//     Scalar mu, sigma;
//     meanStdDev(lap, mu, sigma);    // Use stddev as measure of detail
//     return sigma[0];               // Higher stddev → more edge detail
// }

// int main() {
//     // Load image
//     Mat img = imread("/Users/bishesh/Desktop/Intern/opencv-setup/scene.jpg");
//     if (img.empty()) {
//         cout << "Could not open image" << endl;
//         return -1;
//     }

//     unordered_map<string, Point> blockMap;
//     Mat result = img.clone();

//     for (int y = 0; y <= img.rows - blockSize; y += stepSize) {
//         for (int x = 0; x <= img.cols - blockSize; x += stepSize) {
//             Rect roi(x, y, blockSize, blockSize);
//             Mat block = img(roi);

//             // Skip boring blocks (low texture/detail)
//             double detail = computeDetail(block);
//             if (detail < detailThreshold)
//                 continue;

//             // Generate fuzzy key
//             string key = blockToKey(block);

//             if (blockMap.find(key) != blockMap.end()) {
//                 // Found a match
//                 Point orig = blockMap[key];
//                 rectangle(result, roi, Scalar(255, 0, 255), 2);
//                 rectangle(result, Rect(orig.x, orig.y, blockSize, blockSize), Scalar(0, 0, 255), 2);
//                 line(result, Point(x + blockSize/2, y + blockSize/2),
//                             Point(orig.x + blockSize/2, orig.y + blockSize/2), Scalar(255, 255, 255), 1);
//             } else {
//                 blockMap[key] = Point(x, y);
//             }
//         }
//     }

//     // Show results
//     imshow("Detected Clones (Filtered)", result);
//     waitKey(0);
//     return 0;
// }


//CLUSTERING
// #include <opencv2/opencv.hpp>
// #include <unordered_map>
// #include <vector>
// #include <iostream>
// #include <sstream>
// #include <cmath>

// using namespace cv;
// using namespace std;

// int blockSize = 16;
// int stepSize = 4;
// double detailThreshold = 11.0;
// int minDistance = 20; // Minimum distance between source and destination
// int minClusterSize = 3; // Minimum number of similar clone pairs

// struct ClonePair {
//     Point src;
//     Point dst;
//     Point displacement() const {
//         return Point(dst.x - src.x, dst.y - src.y);
//     }
// };

// string blockToKey(const Mat& block) {
//     Mat gray, small;
//     cvtColor(block, gray, COLOR_BGR2GRAY);
//     resize(gray, small, Size(4, 4));
//     stringstream ss;
//     for (int i = 0; i < small.rows; i++) {
//         for (int j = 0; j < small.cols; j++) {
//             ss << (int)(small.at<uchar>(i, j) / 16) << ",";
//         }
//     }
//     return ss.str();
// }

// double computeDetail(const Mat& block) {
//     Mat gray, lap;
//     cvtColor(block, gray, COLOR_BGR2GRAY);
//     Laplacian(gray, lap, CV_64F);
//     Scalar mu, sigma;
//     meanStdDev(lap, mu, sigma);
//     return sigma[0];
// }

// double euclideanDistance(Point a, Point b) {
//     return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
// }

// // Group displacement vectors into clusters
// vector<vector<ClonePair>> clusterClones(const vector<ClonePair>& pairs, int minClusterSize, double directionTolerance = 5.0) {
//     vector<vector<ClonePair>> clusters;
//     vector<bool> used(pairs.size(), false);

//     for (size_t i = 0; i < pairs.size(); i++) {
//         if (used[i]) continue;
//         vector<ClonePair> cluster = { pairs[i] };
//         used[i] = true;
//         Point refDisp = pairs[i].displacement();

//         for (size_t j = i + 1; j < pairs.size(); j++) {
//             if (used[j]) continue;
//             Point d = pairs[j].displacement();
//             if (abs(d.x - refDisp.x) <= directionTolerance && abs(d.y - refDisp.y) <= directionTolerance) {
//                 cluster.push_back(pairs[j]);
//                 used[j] = true;
//             }
//         }

//         if (cluster.size() >= minClusterSize) {
//             clusters.push_back(cluster);
//         }
//     }
//     return clusters;
// }

// int main() {
//     Mat img = imread("/Users/bishesh/Desktop/Intern/opencv-setup/scene.jpg");
//     if (img.empty()) {
//         cout << "Could not open image" << endl;
//         return -1;
//     }

//     unordered_map<string, Point> blockMap;
//     vector<ClonePair> candidatePairs;
//     Mat result = img.clone();

//     for (int y = 0; y <= img.rows - blockSize; y += stepSize) {
//         for (int x = 0; x <= img.cols - blockSize; x += stepSize) {
//             Rect roi(x, y, blockSize, blockSize);
//             Mat block = img(roi);

//             double detail = computeDetail(block);
//             if (detail < detailThreshold)
//                 continue;

//             string key = blockToKey(block);

//             if (blockMap.find(key) != blockMap.end()) {
//                 Point orig = blockMap[key];
//                 Point curr = Point(x, y);
//                 if (euclideanDistance(orig, curr) < minDistance)
//                     continue; // Ignore near-duplicates

//                 candidatePairs.push_back({orig, curr});
//             } else {
//                 blockMap[key] = Point(x, y);
//             }
//         }
//     }

//     // Cluster and filter clone candidates
//     auto clusters = clusterClones(candidatePairs, minClusterSize);

//     // Draw results
//     for (const auto& cluster : clusters) {
//         for (const auto& pair : cluster) {
//             Rect srcRect(pair.src.x, pair.src.y, blockSize, blockSize);
//             Rect dstRect(pair.dst.x, pair.dst.y, blockSize, blockSize);
//             rectangle(result, srcRect, Scalar(0, 255, 0), 2);
//             rectangle(result, dstRect, Scalar(255, 0, 255), 2);
//             line(result,
//                  Point(pair.src.x + blockSize/2, pair.src.y + blockSize/2),
//                  Point(pair.dst.x + blockSize/2, pair.dst.y + blockSize/2),
//                  Scalar(255, 255, 255), 1);
//         }
//     }

//     cout << "Total clusters: " << clusters.size() << endl;
//     imshow("Detected Clones (Filtered)", result);
//     waitKey(0);
//     return 0;
// }


// QUANTIZED IMAGE SHOW
// #include <opencv2/opencv.hpp>
// #include <unordered_map>
// #include <vector>
// #include <iostream>
// #include <sstream>
// #include <cmath>

// using namespace cv;
// using namespace std;

// int blockSize = 16;
// int stepSize = 4;
// double detailThreshold = 9.7;
// int minDistance = 20;
// int minClusterSize = 3;
// int showQuantized = 0; // Checkbox state (0 = normal, 1 = quantized)

// Mat result, quantizedDisplay;

// struct ClonePair {
//     Point src;
//     Point dst;
//     Point displacement() const {
//         return Point(dst.x - src.x, dst.y - src.y);
//     }
// };

// string blockToKey(const Mat& block, Mat& smallOut) {
//     Mat gray, small;
//     cvtColor(block, gray, COLOR_BGR2GRAY);
//     resize(gray, small, Size(4, 4));
//     smallOut = small.clone();
//     stringstream ss;
//     for (int i = 0; i < small.rows; i++) {
//         for (int j = 0; j < small.cols; j++) {
//             ss << (int)(small.at<uchar>(i, j) / 16) << ",";
//         }
//     }
//     return ss.str();
// }

// double computeDetail(const Mat& block) {
//     Mat gray, lap;
//     cvtColor(block, gray, COLOR_BGR2GRAY);
//     Laplacian(gray, lap, CV_64F);
//     Scalar mu, sigma;
//     meanStdDev(lap, mu, sigma);
//     return sigma[0];
// }

// double euclideanDistance(Point a, Point b) {
//     return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
// }

// vector<vector<ClonePair>> clusterClones(const vector<ClonePair>& pairs, int minClusterSize, double directionTolerance = 5.0) {
//     vector<vector<ClonePair>> clusters;
//     vector<bool> used(pairs.size(), false);

//     for (size_t i = 0; i < pairs.size(); i++) {
//         if (used[i]) continue;
//         vector<ClonePair> cluster = { pairs[i] };
//         used[i] = true;
//         Point refDisp = pairs[i].displacement();

//         for (size_t j = i + 1; j < pairs.size(); j++) {
//             if (used[j]) continue;
//             Point d = pairs[j].displacement();
//             if (abs(d.x - refDisp.x) <= directionTolerance && abs(d.y - refDisp.y) <= directionTolerance) {
//                 cluster.push_back(pairs[j]);
//                 used[j] = true;
//             }
//         }

//         if (cluster.size() >= minClusterSize) {
//             clusters.push_back(cluster);
//         }
//     }
//     return clusters;
// }

// void onCheckboxChange(int, void*) {
//     if (showQuantized == 1) {
//         imshow("Clone Detector", quantizedDisplay);
//     } else {
//         imshow("Clone Detector", result);
//     }
// }

// int main() {
//     Mat img = imread("/Users/bishesh/Desktop/Intern/opencv-setup/scene.jpg");
//     if (img.empty()) {
//         cout << "Could not open image" << endl;
//         return -1;
//     }

//     unordered_map<string, Point> blockMap;
//     vector<ClonePair> candidatePairs;
//     result = img.clone();
//     quantizedDisplay = img.clone();

//     for (int y = 0; y <= img.rows - blockSize; y += stepSize) {
//         for (int x = 0; x <= img.cols - blockSize; x += stepSize) {
//             Rect roi(x, y, blockSize, blockSize);
//             Mat block = img(roi);

//             double detail = computeDetail(block);
//             if (detail < detailThreshold)
//                 continue;

//             Mat compressed;
//             string key = blockToKey(block, compressed);

//             if (blockMap.find(key) != blockMap.end()) {
//                 Point orig = blockMap[key];
//                 Point curr = Point(x, y);
//                 if (euclideanDistance(orig, curr) < minDistance)
//                     continue;

//                 candidatePairs.push_back({orig, curr});
//             } else {
//                 blockMap[key] = Point(x, y);
//             }

//             // Also update quantized display (use average of compressed block as gray value)
//             Mat avgBlock;
//             resize(compressed, avgBlock, Size(blockSize, blockSize), 0, 0, INTER_NEAREST);
//             cvtColor(avgBlock, avgBlock, COLOR_GRAY2BGR);
//             avgBlock.copyTo(quantizedDisplay(roi));
//         }
//     }

//     auto clusters = clusterClones(candidatePairs, minClusterSize);

//     for (const auto& cluster : clusters) {
//         for (const auto& pair : cluster) {
//             Rect srcRect(pair.src.x, pair.src.y, blockSize, blockSize);
//             Rect dstRect(pair.dst.x, pair.dst.y, blockSize, blockSize);
//             rectangle(result, srcRect, Scalar(0, 255, 0), 2);
//             rectangle(result, dstRect, Scalar(255, 0, 255), 2);
//             line(result,
//                  Point(pair.src.x + blockSize/2, pair.src.y + blockSize/2),
//                  Point(pair.dst.x + blockSize/2, pair.dst.y + blockSize/2),
//                  Scalar(255, 255, 255), 1);
//         }
//     }

//     // GUI Setup
//     namedWindow("Clone Detector", WINDOW_AUTOSIZE);
//     createTrackbar("Show Quantized", "Clone Detector", &showQuantized, 1, onCheckboxChange);

//     // Show initial result
//     imshow("Clone Detector", result);
//     waitKey(0);
//     return 0;
// }





