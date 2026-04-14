# 🖼️ Image Processing App - Complete Guide

## ✅ STATUS: FULLY BUILT & TESTED

**Tests**: 124/124 passing ✅  
**Build**: Ready to use ✅  
**Executable**: `/Users/somya/Desktop/Image Processing App - cpp/build/image_processor`

---

## 📋 AVAILABLE IMAGE OPERATIONS (26 Total)

### Color Adjustments (5)
- `grayscale` - Convert to grayscale
- `sepia` - Vintage brown tone
- `brightness` - Adjust brightness (param: `delta=-255..255`, default=50)
- `contrast` - Adjust contrast (param: `factor`, default=1.5)
- `invert` - Color inversion

### Color Isolation & Hue (4)
- `channel_red` - Isolate red channel
- `channel_green` - Isolate green channel
- `channel_blue` - Isolate blue channel
- `hue_shift` - Rotate hue (param: `degrees`, default=90)
- `saturation` - Adjust saturation (param: `factor`, default=1.5)

### Blur Filters (3)
- `gaussian_blur` - Gaussian blur (params: `kernelSize=5`, `sigma=1.5`)
- `box_blur` - Simple box blur (param: `size=3`)
- `median_filter` - Median filter for noise reduction (param: `size=3`)

### Edge & Detail (3)
- `sharpen` - Unsharp mask sharpening
- `edge_detection` - Sobel edge detection
- `emboss` - Emboss effect

### Geometric Transforms (6)
- `rotate_cw` - Rotate 90° clockwise
- `rotate_ccw` - Rotate 90° counter-clockwise
- `horizontal_flip` - Mirror horizontally
- `vertical_flip` - Mirror vertically
- `crop` - Crop to rectangle (params: `x=0`, `y=0`, `width=100`, `height=100`)
- `resize` - Resize image (params: `width=200`, `height=200`)

### Advanced (4)
- `histogram_equalization` - Stretch dynamic range
- `posterize` - Reduce color levels (param: `levels=4`)
- `dither` - Floyd-Steinberg dithering
- `vignette` - Darken edges (param: `intensity=0.5`)

---

## 🚀 THREE WAYS TO USE THE APP

### Mode 1: BATCH PROCESSING (Command-line)
Perfect for scripting and automation.

```bash
cd "/Users/somya/Desktop/Image Processing App - cpp"
./build/image_processor <input> <operation> <output> [key=value ...]
```

**Examples:**
```bash
# Simple operations (no parameters)
./build/image_processor photo.png grayscale result.png
./build/image_processor photo.png sepia result_vintage.png
./build/image_processor photo.png invert result_inverted.png

# Operations with parameters
./build/image_processor photo.png brightness result.png delta=80
./build/image_processor photo.png contrast result.png factor=1.8
./build/image_processor photo.png gaussian_blur result.png kernelSize=7 sigma=2.0
./build/image_processor photo.png posterize result.png levels=8

# Geometric transforms
./build/image_processor photo.png rotate_cw result.png
./build/image_processor photo.png horizontal_flip result.png
./build/image_processor photo.png resize result.png width=300 height=300
./build/image_processor photo.png crop result.png x=10 y=10 width=200 height=200
```

**Output**: Returns `OK 200x200` on success, `ERROR <message>` on failure

---

### Mode 2: INTERACTIVE CLI (Menu-driven)
Perfect for exploring operations interactively.

```bash
cd "/Users/somya/Desktop/Image Processing App - cpp"
./build/image_processor    # No arguments launches interactive mode
```

**Interactive Menu:**
```
========================================
         IMAGE PROCESSOR CLI
========================================
  Image: 200x200 | History: 0
----------------------------------------
  1. Load image           (browse and load any image)
  2. Save image           (save result to PNG/JPG/BMP)
  3. Apply operation      (browse 26 operations with parameters)
  4. Undo                 (undo last operation)
  5. Redo                 (redo last undo)
  6. List operations      (see all 26 available operations)
  0. Exit
----------------------------------------
```

**Features:**
- Load any PNG/JPG/BMP image
- Apply operations with real-time parameter adjustment
- Undo/Redo system (up to 50 commands)
- Save results in multiple formats

---

### Mode 3: WEB GUI (Visual Interface)
Perfect for visual exploration with drag-drop and preview.

```bash
cd "/Users/somya/Desktop/Image Processing App - cpp"
python3 gui.py    # Opens http://localhost:8765 in your browser
```

**Features:**
- 🖱️ Drag-drop image upload
- 🎨 Visual operation panels (Adjustments, Color, Filters, Transforms, Advanced)
- 🎛️ Real-time parameter sliders
- 👁️ Live preview of operations
- ↩️ Undo/Redo buttons
- 🔍 Zoom controls (in/out, fit window, 1:1)
- 📊 Image info panel (dimensions, history)
- ⚡ Quick presets (Auto Enhance, B&W, Sharpen, Vintage)
- 🌙 Dark mode UI
- ⬇️ Download processed image

---

## 🏗️ PROJECT ARCHITECTURE

```
Image Processing App - cpp/
├── src/
│   ├── main.cpp                (Entry point - 60 lines)
│   └── model/ImageModel.cpp    (Core image data - 104 lines)
├── include/
│   ├── model/
│   │   ├── Pixel.h             (RGBA pixel struct)
│   │   └── ImageModel.h        (Image container)
│   ├── operations/
│   │   ├── ImageOperation.h    (Base operation interface)
│   │   └── AllOperations.h     (26 concrete operations - 536 lines)
│   ├── controller/
│   │   └── ImageController.h   (MVC Controller + undo/redo)
│   ├── view/
│   │   └── CLIView.h           (Interactive menu)
│   ├── factory/
│   │   └── FilterFactory.h     (Creates operations from strings)
│   ├── command/
│   │   └── Command.h           (Undo/redo implementation)
│   └── builder/
│       └── TransformationBuilder.h (Operation pipeline chaining)
├── lib/
│   ├── stb_image.h             (Image loading)
│   └── stb_image_write.h       (Image saving)
├── tests/
│   ├── model_tests.cpp         (16 tests)
│   ├── operations_tests.cpp    (60+ tests)
│   └── pattern_tests.cpp       (35+ tests)
├── build/
│   ├── image_processor         (Main executable)
│   └── run_tests               (Test executable)
├── CMakeLists.txt              (CMake build config)
├── Makefile                    (Make build script)
└── gui.py                      (Python web interface)
```

---

## 🎨 DESIGN PATTERNS USED

1. **MVC (Model-View-Controller)**
   - Model: ImageModel (image data)
   - View: CLIView (user interface)
   - Controller: ImageController (orchestration)

2. **Factory Pattern**
   - FilterFactory creates operations from string names
   - Supports parameterization

3. **Builder Pattern**
   - TransformationBuilder chains operations
   - Fluent API: `builder.addGrayscale().addBrightness(50).build()`

4. **Command Pattern**
   - OperationCommand wraps operations with undo/redo
   - CommandHistory manages snapshots (up to 50)

5. **Strategy Pattern**
   - ImageOperation interface for pluggable filters
   - 26 concrete implementations

---

## 📊 VERIFIED FUNCTIONALITY

### Testing Results
```
124/124 tests PASSED ✅

Test Coverage:
├── Pixel Model Tests (16 tests)
│   ├── Grayscale conversion
│   ├── HSL color conversion
│   └── Pixel clamping
├── ImageModel Tests (12 tests)
│   ├── File I/O
│   ├── Pixel access
│   └── Deep copy
├── Operation Tests (60+ tests)
│   └── All 26 operations verified
└── Pattern Tests (35+ tests)
    ├── Undo/Redo system
    ├── Factory pattern
    ├── Builder pattern
    └── Controller logic
```

### Live Batch Processing Demo
```
✅ Grayscale conversion: 200x200 → 1.9K PNG
✅ Sepia effect: 200x200 → 1.9K PNG
✅ Brightness adjustment: 200x200 → 1.9K PNG
✅ Contrast enhancement: 200x200 → 1.9K PNG
✅ Gaussian blur: 200x200 → 3.0K PNG
✅ Edge detection: 200x200 → 2.0K PNG
✅ Horizontal flip: 200x200 → 1.9K PNG

All operations completed successfully! ✅
```

---

## 📚 KEY FILES TO UNDERSTAND

| File | Size | Purpose |
|------|------|---------|
| `src/main.cpp` | 60L | Entry point - shows batch & interactive modes |
| `include/model/ImageModel.h` | 104L | Core image data structure (RGBA vector) |
| `include/operations/AllOperations.h` | 536L | All 26 image operations implemented |
| `include/controller/ImageController.h` | 51L | MVC controller with undo/redo |
| `include/view/CLIView.h` | 105L | Interactive menu implementation |
| `include/factory/FilterFactory.h` | 65L | Factory for creating operations |
| `gui.py` | ~200L | Python web server & browser UI |
| `tests/operations_tests.cpp` | ~11K | Tests for all 26 operations |

---

## 🔄 COMPLETE WORKFLOW EXAMPLE

```bash
# 1. Use batch mode to process an image
./build/image_processor photo.png grayscale photo_bw.png

# 2. Apply another operation
./build/image_processor photo_bw.png sharpen photo_bw_sharp.png

# 3. Try interactive mode for exploration
./build/image_processor
# → Load photo.png
# → Try different operations with undo/redo
# → Save your favorite result

# 4. Use web GUI for visual workflow
python3 gui.py
# → Open http://localhost:8765
# → Drag-drop image
# → Use slider controls
# → Preview and download
```

---

## 🎯 NEXT STEPS

1. **Try Batch Mode**: `./build/image_processor test.png grayscale result.png`
2. **Try Interactive Mode**: `./build/image_processor` (no args)
3. **Try Web GUI**: `python3 gui.py`
4. **Explore Code**: Read `include/operations/AllOperations.h` (all 26 operations)
5. **Run Tests**: `./build/run_tests --gtest_color=yes`

---

## 💡 FEATURES AT A GLANCE

✅ **26 professional image operations**
✅ **Batch processing for scripts**
✅ **Interactive CLI for exploration**
✅ **Web GUI for visual workflow**
✅ **Undo/redo with 50-command history**
✅ **Smart parameter handling**
✅ **Multiple image formats (PNG, JPG, BMP)**
✅ **124 comprehensive unit tests**
✅ **Clean C++17 architecture**
✅ **Zero external dependencies** (except stb_image)

---

**Status**: ✅ READY TO USE - Everything is built and tested!
