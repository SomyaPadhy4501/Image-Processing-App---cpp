# Image Processing App - Complete Beginner's Guide

## 🎯 What Does This App Do? (Simple Version)

Imagine you have a photo. This app lets you:
- Make it black & white
- Blur it
- Brighten it
- Flip it
- Find edges in it
- And 20+ other effects

You can do this in 3 ways:
1. **Command line** (fastest for scripts)
2. **Menu interface** (interactive, explore operations)
3. **Web GUI** (visual, drag-drop images)

---

## 📁 How Is It Organized? (The Folders)

Think of it like a house with different rooms:

```
Image Processing App - cpp/
│
├── 📁 include/          ← BLUEPRINTS (what the code will do)
│   ├── model/          ← Data structures (how to store images)
│   ├── operations/     ← All 26 filters/effects
│   ├── controller/     ← Brain (orchestrates everything)
│   ├── view/           ← Menu interface
│   ├── factory/        ← Creates filters from names
│   ├── command/        ← Undo/redo system
│   └── builder/        ← Chain operations together
│
├── 📁 src/             ← IMPLEMENTATION (actual code)
│   ├── main.cpp        ← Entry point (where program starts)
│   ├── ImageModel.cpp  ← Image data implementation
│   └── [stubs]         ← Empty files (headers have the code)
│
├── 📁 lib/             ← EXTERNAL LIBRARIES
│   ├── stb_image.h     ← Load images (PNG, JPG, BMP)
│   └── stb_image_write.h ← Save images
│
├── 📁 tests/           ← VERIFICATION
│   ├── model_tests.cpp     ← Test image data
│   ├── operations_tests.cpp ← Test all 26 filters
│   └── pattern_tests.cpp   ← Test undo/redo, etc
│
├── 📁 build/           ← COMPILED PROGRAMS
│   ├── image_processor ← Main executable
│   └── run_tests       ← Test executable
│
├── CMakeLists.txt      ← How to build (using CMake)
├── Makefile            ← How to build (using Make)
├── gui.py              ← Web interface
└── HOW_TO_USE.md       ← User guide
```

---

## 🔄 How Does It Work? (The Flow)

### Step 1: You Run The Program

```bash
./build/image_processor photo.png grayscale result.png
```

What happens:
1. Program starts (`main.cpp`)
2. Reads arguments: input file, operation name, output file
3. Loads image using stb_image library
4. Creates the operation (using FilterFactory)
5. Applies it to the image
6. Saves the result

### Step 2: Loading an Image

**File:** `include/model/ImageModel.h` and `src/model/ImageModel.cpp`

```cpp
// This is what happens when you load "photo.png"
ImageModel::loadFromFile("photo.png")
```

What does it do?
- Reads the PNG file using stb_image
- Converts it to a flat array of pixels
- Each pixel is RGBA (Red, Green, Blue, Alpha/transparency)
- Stores width, height, and all pixels in memory

**Think of it like:** Converting a photo into a grid of colored dots

### Step 3: Understanding A Pixel

**File:** `include/model/Pixel.h`

A pixel is just 4 numbers:
```
R (Red):   0-255  (how much red)
G (Green): 0-255  (how much green)
B (Blue):  0-255  (how much blue)
A (Alpha): 0-255  (how transparent)
```

Example: Pure red = `Pixel(255, 0, 0, 255)` (fully red, fully opaque)

The Pixel class also has helper functions:
- Convert to grayscale (average the RGB values)
- Convert to HSL (Hue, Saturation, Lightness)
- Auto-clamp values (keep them 0-255)

### Step 4: Creating an Operation

**File:** `include/factory/FilterFactory.h`

This is like a factory that creates filters. When you say "I want grayscale":

```cpp
FilterFactory::create("grayscale", {})
```

It looks up "grayscale" in a list and returns a new `GrayscaleOperation` object.

If you want parameters:
```cpp
FilterFactory::create("brightness", {"delta", 50})
```

It returns a `BrightnessOperation` with delta=50.

**Think of it like:** A restaurant kitchen - you order "grayscale" and the chef knows exactly what to make.

### Step 5: Applying an Operation

**File:** `include/operations/AllOperations.h`

Each operation is a class like `GrayscaleOperation`:

```cpp
class GrayscaleOperation : public ImageOperation {
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        
        // Loop through each pixel in the source image
        for (int y = 0; y < src.getHeight(); y++) {
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);          // Get pixel at (x,y)
                int gray = p.toGray();                 // Convert to gray
                result->setPixel(x, y, Pixel(gray, gray, gray, p.a)); // Set gray pixel
            }
        }
        return result;
    }
};
```

**What this does step-by-step:**
1. Create a new empty image (same size as original)
2. For EACH pixel in the original:
   - Get the pixel's RGB values
   - Calculate grayscale (average of R, G, B)
   - Put that gray value in the new image
3. Return the new image

**Think of it like:** Photography - you're making a copy with grayscale applied.

### Step 6: Saving the Image

```cpp
result->saveToFile("output.png")
```

This uses stb_image_write to convert the pixel array back to a PNG file and save it.

---

## 🎬 Real Example: Make an Image Grayscale

You type:
```bash
./build/image_processor photo.png grayscale result.png
```

**What happens under the hood:**

```
1. main.cpp reads the arguments
   ├─ inputPath = "photo.png"
   ├─ operation = "grayscale"
   └─ outputPath = "result.png"

2. Load the image
   └─ ImageModel::loadFromFile("photo.png")
      └─ Reads PNG file and converts to array of Pixel objects
      └─ Now we have: {R:100, G:150, B:200}, {R:50, G:60, B:70}, ...

3. Create the operation
   └─ FilterFactory::create("grayscale", {})
      └─ Returns a new GrayscaleOperation object

4. Apply the operation
   └─ grayscaleOp->apply(imageModel)
      └─ For each pixel:
         ├─ Get RGB (100, 150, 200)
         ├─ Calculate gray = (100+150+200)/3 = 150
         └─ Create new pixel (150, 150, 150) ← same RGB = gray
      └─ Returns new ImageModel with all gray pixels

5. Save the result
   └─ result->saveToFile("result.png")
      └─ Converts pixel array back to PNG

6. Done! result.png is now grayscale
```

---

## 🧩 The 26 Operations Explained

All 26 are implemented in `include/operations/AllOperations.h`. Here's what each category does:

### Color Operations (5)
- **Grayscale**: Average R, G, B to make gray
- **Sepia**: Multiply RGB by different amounts to get vintage look
- **Brightness**: Add a number to each pixel (delta)
- **Contrast**: Stretch values away from middle gray (128)
- **Invert**: Do `255 - value` for each RGB

### Channel Isolation (3)
- **Red**: Keep only Red, set Green and Blue to 0
- **Green**: Keep only Green, set Red and Blue to 0
- **Blue**: Keep only Blue, set Red and Green to 0

### Hue/Saturation (2)
- **Hue Shift**: Convert RGB→HSL, rotate the H value, convert back
- **Saturation**: Convert RGB→HSL, multiply the S value, convert back

### Blur Filters (3)
- **Gaussian Blur**: Create a "bell curve" kernel, apply to each pixel
  - Kernel is like a 5x5 grid of weights
  - Takes weighted average of surrounding pixels
- **Box Blur**: Simple average of surrounding pixels
- **Median Filter**: Take the middle value of surrounding pixels (good for noise)

### Edge Detection (3)
- **Sharpen**: Enhance edges by subtracting blurred version from original
- **Edge Detection (Sobel)**: Calculate how quickly color changes (edges are big changes)
- **Emboss**: Make pixels look raised/3D

### Geometric Transforms (4)
- **Rotate 90° CW**: Swap width/height, rearrange pixels
- **Rotate 90° CCW**: Swap width/height, rearrange differently
- **Flip Horizontal**: Mirror left-right
- **Flip Vertical**: Mirror top-bottom

### Advanced (4)
- **Histogram Equalization**: Stretch the brightness levels to use full range
- **Posterize**: Round RGB values to fewer levels (creates "poster" effect)
- **Dither**: Add patterns to simulate fewer colors
- **Vignette**: Darken the edges in a circle

---

## 🔄 The MVC Pattern (Architecture)

This app uses **MVC** = Model-View-Controller. Think of it like a restaurant:

### Model = Kitchen
- **What:** `ImageModel` - stores the actual image data
- **Job:** Hold and manipulate pixels
- **File:** `include/model/ImageModel.h`

### View = Dining Room / Menu
- **What:** `CLIView` - the interactive menu
- **Job:** Show options to user, get their choices
- **File:** `include/view/CLIView.h`

### Controller = Manager
- **What:** `ImageController` - coordinates everything
- **Job:** Tell model to load images, apply operations, manage history
- **File:** `include/controller/ImageController.h`

**Flow:**
```
User → View (menu)
    ↓
User picks "Apply Brightness"
    ↓
View tells Controller "Apply Brightness with delta=50"
    ↓
Controller tells Model to apply operation
    ↓
Model applies it to pixels
    ↓
Model returns result
    ↓
Controller updates View
    ↓
View shows result to user
```

---

## 📜 The Undo/Redo System

**Files:** `include/command/Command.h`

How it works:

### Without Undo/Redo:
```
Original Image → Apply Brightness → Result
(original lost!)
```

### With Undo/Redo:
```
Original Image ← save → Brightness Applied ← save → Contrast Applied
     ↑ (undo brings you back)
```

**Implementation:**
```cpp
class OperationCommand {
    ImageModel* original_;      // Save the original before applying
    std::unique_ptr<ImageModel> result_;  // Save the result
    
    void execute() {
        result_ = operation_->apply(*original_);  // Apply operation
    }
    
    void undo() {
        // Restore from snapshot
        return original_;
    }
};
```

**What's `CommandHistory`?**
- It's like a list of all your operations
- Maximum 50 commands saved
- `undo_stack`: List of operations you've done
- `redo_stack`: List of operations you've undone

---

## 🏗️ The Builder Pattern

**File:** `include/builder/TransformationBuilder.h`

This lets you chain operations:

```cpp
TransformationBuilder builder;
builder.addGrayscale()
       .addBrightness(50)
       .addSharpen()
       .build()
       .apply(image);
```

This is like:
1. Convert to grayscale
2. Make it brighter
3. Sharpen it
4. Apply all three in sequence

---

## 🏭 The Factory Pattern

**File:** `include/factory/FilterFactory.h`

Instead of writing:
```cpp
if (name == "grayscale") return std::make_unique<GrayscaleOperation>();
if (name == "sepia") return std::make_unique<SepiaOperation>();
...
```

The factory centralizes this. It's like a catalog:

```
"grayscale"    → GrayscaleOperation
"sepia"        → SepiaOperation
"brightness"   → BrightnessOperation(...params...)
"gaussian_blur" → GaussianBlurOperation(...params...)
```

---

## 🧪 How Testing Works

**Files:**
- `tests/model_tests.cpp` - Test pixel and image operations
- `tests/operations_tests.cpp` - Test all 26 filters
- `tests/pattern_tests.cpp` - Test undo/redo, factory, etc.

Each test:
1. Creates test data
2. Runs code
3. Checks result

Example:
```cpp
TEST(GrayscaleTest, SimpleConversion) {
    // Create a test image: red pixel
    ImageModel img(1, 1);
    img.setPixel(0, 0, Pixel(100, 150, 200, 255));
    
    // Apply grayscale
    GrayscaleOperation op;
    auto result = op.apply(img);
    
    // Check: all RGB should be same (gray)
    Pixel p = result->getPixel(0, 0);
    EXPECT_EQ(p.r, p.g);
    EXPECT_EQ(p.g, p.b);
}
```

Run all tests:
```bash
./build/run_tests --gtest_color=yes
```

---

## 🎨 How the Web GUI Works

**File:** `gui.py`

It's a Python web server:

```
Browser (HTML/JS)
      ↓↑
  Python Server (gui.py)
      ↓↑
C++ Executable (image_processor)
```

**Flow:**
1. You open http://localhost:8765
2. Python serves HTML page
3. You upload image (JavaScript sends to Python)
4. Python saves image to temp folder
5. You click "Grayscale"
6. JavaScript calls Python /apply endpoint
7. Python runs: `./image_processor input.png grayscale output.png`
8. C++ does the work, saves result
9. Python sends result back to browser
10. Browser shows image

**Key endpoints:**
- `POST /upload` - Save uploaded image
- `POST /apply` - Run one operation
- `POST /apply_batch` - Run all 26 operations
- `GET /tmp/<filename>` - Download processed image

---

## 📊 Three Ways to Use It

### Way 1: Command Line (Batch Mode)
```bash
./build/image_processor input.png grayscale output.png
./build/image_processor input.png brightness output.png delta=50
```
- Fastest
- Good for scripts
- No interactive menu

### Way 2: Interactive Menu
```bash
./build/image_processor
# Shows menu:
# 1. Load image
# 2. Save image
# 3. Apply operation
# 4. Undo
# 5. Redo
# 6. List operations
```
- Explore operations
- Use undo/redo
- See image dimensions

### Way 3: Web GUI
```bash
python3 gui.py
# Opens http://localhost:8765
```
- Visual (drag-drop)
- Before/After comparison
- See all 26 operations at once (Gallery mode)
- Parameter sliders

---

## 🎓 How to Trace Through Code

Want to understand "What happens when I apply Gaussian Blur"?

1. **Start at entry point**: `src/main.cpp`
   - Parse arguments: `gaussian_blur`, `kernelSize=5`

2. **Load image**: `ImageModel::loadFromFile()`
   - Opens PNG file using `stb_image.h`
   - Stores pixels in a vector

3. **Create operation**: `FilterFactory::create("gaussian_blur", {kernelSize: 5})`
   - Looks up "gaussian_blur"
   - Returns `new GaussianBlurOperation(5)`

4. **Apply operation**: `operation->apply(image)`
   - In `AllOperations.h`, find `GaussianBlurOperation::apply()`
   - Creates Gaussian kernel (5x5 grid of numbers)
   - For each pixel, calculates weighted average of surrounding pixels
   - Returns new image

5. **Save result**: `result->saveToFile("output.png")`
   - Converts pixel vector back to PNG format
   - Uses `stb_image_write.h`

---

## 💡 Key Takeaways

1. **Images are just pixel grids**: Each pixel is 4 numbers (R,G,B,A)
2. **Operations loop through pixels**: Each operation modifies pixels in some way
3. **Smart patterns make code clean**: MVC, Factory, Builder, Command patterns
4. **Testing verifies everything works**: 124 tests check all functionality
5. **Multiple interfaces, same engine**: CLI, Menu, Web all use the same C++ code
6. **Undo/Redo works by saving snapshots**: Before each operation, save the image

---

## 🚀 Quick Start

```bash
# 1. Build it
make app

# 2. Try batch mode
./build/image_processor photo.png grayscale result.png

# 3. Try interactive mode
./build/image_processor

# 4. Try web GUI
python3 gui.py

# 5. Run tests
./build/run_tests
```

That's it! You now understand the whole project. 🎉
