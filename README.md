# Image Analysis Toolkit

This repository contains two interactive OpenCV-based C++ tools for image analysis:

- **Magnifier**: A pixel-level zoom tool for exploring hidden image details.
- **Clone Detection**: A forensic analysis tool to detect duplicated regions within an image (e.g., image tampering or forgery).

---

## 📁 Contents

- `magnifier.cpp`: Zoom tool with live pixel magnification under cursor.
- `clone_detector.cpp`: Clone region detector with adjustable parameters.
- `CMakeLists.txt`: Build file for compiling with CMake.

---

## Dependencies

- [OpenCV](https://opencv.org/) (tested with OpenCV 4.x)
- C++17 or higher
- CMake 3.10+

---

## Build Instructions

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/image-analysis-toolkit.git
cd image-analysis-toolkit
```

### 2. Build with CMake

```bash
mkdir build
cd build
cmake ..
make
```

### 3. Run

```bash
./magnifier
# or
./clone_detector
```

> Make sure to update the image path in the code to point to your actual image.  

---

## Magnifier Tool

The **Magnifier** allows users to zoom into specific parts of an image and observe pixel-level details.

### How It Works

It crops a region around the mouse cursor and enlarges it using linear interpolation.

### Features

- Real-time zoom as you move the mouse
- Adjustable zoom from 1x to 10x using a slider
- Floating zoom window that follows the mouse

### Usage

1. Run the program: `./magnifier`, but initially adding the file name in CMakeLists.txt file as <file_name.cpp>
2. Hover over the image to see a zoomed view
3. Adjust the zoom level using the trackbar

---

## Clone Detection System

The **Clone Detector** identifies copied and pasted regions within the same image. It's useful for digital image forensics.

### Parameters

| Parameter                   | Description |
|----------------------------|-------------|
| **Minimal Similarity** (Quantization Level) | Controls how pixel intensities are grouped. Lower = sensitive, higher = robust. |
| **Minimal Detail** (Detail Threshold) | Filters flat areas using edge detection (Laplacian). Higher = stricter. |
| **Minimal Cluster Size** | Minimum number of similar block pairs to form a valid clone region. |
| **Block Size (2ⁿ)** | Patch size used for similarity matching. Smaller = fine detail, Larger = robustness. |
| **Maximal Image Size** | Image is resized if it exceeds this limit to reduce processing time. |

### How It Works

1. Image is divided into overlapping patches.
2. Low-detail patches are skipped.
3. Remaining patches are compressed and encoded into hash keys.
4. Matches are clustered and visualized if they show consistent displacement.

### Usage

1. Run the program: `./clone_detector`, but initially adding the file name in CMakeLists.txt file as <file_name.cpp>
2. Use the trackbars to tune sensitivity
3. Press `ESC` to exit

### Output

- Green and magenta rectangles show detected clone pairs.
- White lines connect matching blocks.

---

## Image Input

Both programs currently read an image from a hardcoded path.

To change the input image:
- Edit this line in both `.cpp` files:
```cpp
const string imagePath = "/your/path/to/image.jpg";
```

---

## 📌 Notes

- GUI-based OpenCV windows will pop up during runtime.
- Clone Detector is computationally intensive — reduce image size for performance.
- Works on Linux/macOS/Windows with proper OpenCV setup.

---
