#!/usr/bin/env python3
"""
Web-based GUI for the C++ Image Processor.
Starts a local HTTP server and opens a browser-based UI
that calls the C++ binary for all operations.
"""

import os
import sys
import json
import shutil
import subprocess
import tempfile
import base64
import webbrowser
from pathlib import Path
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import parse_qs, urlparse
import threading
import time

SCRIPT_DIR = Path(__file__).parent
CPP_BINARY = SCRIPT_DIR / "build" / "image_processor"
PORT = 8765

TMP_DIR = tempfile.mkdtemp(prefix="imgproc_cpp_")


class ImageHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        pass

    def do_GET(self):
        if self.path == "/" or self.path == "/index.html":
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(HTML_PAGE.encode())
        elif self.path.startswith("/tmp/"):
            fname = self.path.split("?")[0].split("/")[-1]
            fpath = os.path.join(TMP_DIR, fname)
            if os.path.exists(fpath):
                self.send_response(200)
                ext = Path(fpath).suffix.lower()
                ct = "image/png" if ext == ".png" else "image/jpeg"
                self.send_header("Content-Type", ct)
                self.send_header("Cache-Control", "no-cache")
                self.end_headers()
                with open(fpath, "rb") as f:
                    self.wfile.write(f.read())
            else:
                self.send_error(404)
        else:
            self.send_error(404)

    def do_POST(self):
        content_length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_length)

        if self.path == "/upload":
            try:
                data = json.loads(body)
                img_data = base64.b64decode(data["image"].split(",")[1])
                fname = f"original_{int(time.time())}.png"
                fpath = os.path.join(TMP_DIR, fname)
                with open(fpath, "wb") as f:
                    f.write(img_data)
                self._json_response({"ok": True, "file": fname})
            except Exception as e:
                self._json_response({"ok": False, "error": str(e)})

        elif self.path == "/apply":
            try:
                data = json.loads(body)
                input_file = os.path.join(TMP_DIR, data["input"])
                operation = data["operation"]
                params = data.get("params", {})
                output_name = f"step_{int(time.time() * 1000)}.png"
                output_file = os.path.join(TMP_DIR, output_name)

                cmd = [str(CPP_BINARY), input_file, operation, output_file]
                for k, v in params.items():
                    cmd.append(f"{k}={v}")

                result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
                if result.returncode == 0:
                    self._json_response({"ok": True, "file": output_name, "info": result.stdout.strip()})
                else:
                    self._json_response({"ok": False, "error": result.stderr.strip()})
            except Exception as e:
                self._json_response({"ok": False, "error": str(e)})

        elif self.path == "/apply_batch":
            try:
                data = json.loads(body)
                input_file = os.path.join(TMP_DIR, data["input"])
                operations = data["operations"]
                results = []

                for op_entry in operations:
                    op = op_entry["op"]
                    params = op_entry.get("params", {})
                    output_name = f"gallery_{op}_{int(time.time() * 1000)}.png"
                    output_file = os.path.join(TMP_DIR, output_name)

                    cmd = [str(CPP_BINARY), input_file, op, output_file]
                    for k, v in params.items():
                        cmd.append(f"{k}={v}")

                    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
                    if result.returncode == 0:
                        results.append({"op": op, "file": output_name, "ok": True})
                    else:
                        results.append({"op": op, "file": None, "ok": False, "error": result.stderr.strip()})

                self._json_response({"ok": True, "results": results})
            except Exception as e:
                self._json_response({"ok": False, "error": str(e)})
        else:
            self.send_error(404)

    def _json_response(self, data):
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())


HTML_PAGE = r'''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Image Processor — C++ Engine</title>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
:root {
  --bg: #111317; --bg2: #1a1d23; --bg3: #22262e; --bg4: #2a2f3a;
  --bg5: #333844; --border: #2e3340; --border2: #3d4455;
  --text: #e2e6f0; --text2: #9aa0b4; --text3: #5c6378;
  --accent: #4f8ef7; --accent2: #6aa3ff; --accent3: #3a70d4;
  --green: #3ecf8e; --red: #e05252; --yellow: #f0c040;
  --purple: #9b72f7; --orange: #f57c3e;
}
body { font-family: -apple-system, BlinkMacSystemFont, 'Inter', 'Segoe UI', sans-serif;
       background: var(--bg); color: var(--text); height: 100vh; overflow: hidden; display: flex; flex-direction: column; }

/* ─── TOP BAR ─── */
.topbar { display: flex; align-items: center; gap: 2px; padding: 6px 10px;
          background: var(--bg2); border-bottom: 1px solid var(--border); flex-shrink: 0; }
.topbar-title { font-size: 13px; font-weight: 700; color: var(--accent2); margin-right: 10px; letter-spacing: -0.3px; }
.topbar-title span { color: var(--text2); font-weight: 400; }
.tb-btn { background: var(--bg4); border: 1px solid var(--border2); color: var(--text);
          font-size: 12px; font-weight: 500; padding: 5px 11px; cursor: pointer;
          border-radius: 5px; transition: all 0.15s; }
.tb-btn:hover { background: var(--bg5); border-color: var(--accent3); }
.tb-btn:active { background: var(--accent3); }
.tb-btn:disabled { opacity: 0.35; cursor: default; pointer-events: none; }
.tb-btn.primary { background: var(--accent3); border-color: var(--accent); color: white; }
.tb-btn.primary:hover { background: var(--accent); }
.tb-sep { width: 1px; height: 20px; background: var(--border2); margin: 0 4px; flex-shrink: 0; }
.tb-spacer { flex: 1; }
.mode-tabs { display: flex; gap: 2px; background: var(--bg3); border: 1px solid var(--border); border-radius: 7px; padding: 2px; }
.mode-tab { padding: 5px 14px; border-radius: 5px; font-size: 12px; font-weight: 600; cursor: pointer;
            color: var(--text2); transition: all 0.15s; }
.mode-tab.active { background: var(--accent3); color: white; }
.mode-tab:hover:not(.active) { color: var(--text); }

/* ─── MAIN LAYOUT ─── */
.main { display: flex; flex: 1; overflow: hidden; }

/* ─── SIDEBAR ─── */
.sidebar { width: 195px; min-width: 195px; background: var(--bg2);
           border-right: 1px solid var(--border); overflow-y: auto; flex-shrink: 0; }
.sidebar::-webkit-scrollbar { width: 5px; }
.sidebar::-webkit-scrollbar-thumb { background: var(--bg5); border-radius: 3px; }
.sec-hdr { font-size: 9px; font-weight: 800; letter-spacing: 1px; color: var(--text3);
           padding: 10px 12px 5px; text-transform: uppercase; }
.op-item { display: flex; align-items: center; gap: 8px; padding: 7px 12px;
           cursor: pointer; font-size: 12px; color: var(--text2); border-radius: 0;
           transition: background 0.1s; user-select: none; }
.op-item:hover { background: var(--bg3); color: var(--text); }
.op-item.active { background: linear-gradient(90deg, rgba(79,142,247,0.15), transparent);
                  color: var(--accent2); border-left: 2px solid var(--accent); }
.op-dot { width: 7px; height: 7px; border-radius: 50%; flex-shrink: 0; }
.op-dot.color { background: var(--yellow); }
.op-dot.adjust { background: var(--accent2); }
.op-dot.filter { background: var(--purple); }
.op-dot.transform { background: var(--green); }
.op-dot.advanced { background: var(--orange); }

/* ─── CENTER PANEL ─── */
.center { flex: 1; display: flex; flex-direction: column; overflow: hidden; }

/* EDITOR MODE */
#editorMode { display: flex; flex-direction: column; height: 100%; }
.canvas-wrap { flex: 1; position: relative; overflow: hidden; background: #0a0b0e;
               background-image: repeating-conic-gradient(var(--bg2) 0% 25%, transparent 0% 50%) 0 0 / 20px 20px; }
.canvas-wrap img { position: absolute; top: 50%; left: 50%; transform-origin: center;
                   max-width: none; image-rendering: auto; }
.drop-hint { position: absolute; inset: 0; display: flex; flex-direction: column;
             align-items: center; justify-content: center; gap: 12px; }
.drop-hint .icon { font-size: 52px; filter: grayscale(1); opacity: 0.4; }
.drop-hint .msg { font-size: 17px; color: var(--text3); }
.drop-hint .sub { font-size: 12px; color: var(--text3); }
.drop-hint .upload-btn { background: var(--accent3); color: white; border: none;
                          padding: 10px 24px; border-radius: 7px; font-size: 13px;
                          font-weight: 600; cursor: pointer; margin-top: 4px; }
.drop-hint .upload-btn:hover { background: var(--accent); }
.drop-zone-active { outline: 2px dashed var(--accent) !important; }

/* Before/After */
.ba-container { position: absolute; inset: 0; overflow: hidden; display: none; }
.ba-container.active { display: block; }
.ba-img { position: absolute; top: 0; left: 0; width: 100%; height: 100%; }
.ba-img img { position: absolute; top: 50%; left: 50%; max-width: none; transform-origin: center; }
.ba-before { clip-path: inset(0 50% 0 0); }
.ba-divider { position: absolute; top: 0; bottom: 0; width: 2px; background: white;
              left: 50%; cursor: ew-resize; z-index: 10;
              box-shadow: 0 0 0 1px rgba(0,0,0,0.5); }
.ba-handle { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);
             width: 36px; height: 36px; background: white; border-radius: 50%;
             box-shadow: 0 2px 8px rgba(0,0,0,0.5); display: flex; align-items: center;
             justify-content: center; font-size: 14px; cursor: ew-resize; user-select: none; }
.ba-label { position: absolute; top: 12px; font-size: 11px; font-weight: 700;
            padding: 3px 10px; border-radius: 4px; background: rgba(0,0,0,0.6); }
.ba-label.left { left: 12px; }
.ba-label.right { right: 12px; }

/* Spinner */
.spinner { position: absolute; inset: 0; display: none; align-items: center;
           justify-content: center; background: rgba(0,0,0,0.45); z-index: 20; }
.spinner.show { display: flex; }
.spin-box { background: var(--bg3); border: 1px solid var(--border2); border-radius: 10px;
            padding: 16px 28px; display: flex; flex-direction: column; align-items: center; gap: 10px; }
.spin-ring { width: 28px; height: 28px; border: 3px solid var(--border2);
             border-top-color: var(--accent); border-radius: 50%; animation: spin 0.8s linear infinite; }
@keyframes spin { to { transform: rotate(360deg); } }
.spin-text { font-size: 12px; color: var(--text2); }

/* GALLERY MODE */
#galleryMode { display: none; flex-direction: column; height: 100%; }
#galleryMode.active { display: flex; }
.gallery-header { padding: 12px 16px; background: var(--bg2); border-bottom: 1px solid var(--border);
                  display: flex; align-items: center; gap: 10px; flex-shrink: 0; }
.gallery-title { font-size: 13px; font-weight: 600; color: var(--text); }
.gallery-grid { flex: 1; overflow-y: auto; padding: 16px; display: grid;
                grid-template-columns: repeat(auto-fill, minmax(200px, 1fr)); gap: 12px; }
.gallery-grid::-webkit-scrollbar { width: 6px; }
.gallery-grid::-webkit-scrollbar-thumb { background: var(--bg5); border-radius: 3px; }
.gallery-card { background: var(--bg2); border: 1px solid var(--border); border-radius: 10px;
                overflow: hidden; transition: border-color 0.2s, transform 0.15s; cursor: pointer; }
.gallery-card:hover { border-color: var(--accent3); transform: translateY(-2px); }
.gallery-card-img { width: 100%; aspect-ratio: 1; object-fit: contain; background: #0a0b0e;
                    background-image: repeating-conic-gradient(var(--bg2) 0% 25%, transparent 0% 50%) 0 0 / 16px 16px;
                    display: block; }
.gallery-card-info { padding: 8px 10px; }
.gallery-card-name { font-size: 11px; font-weight: 600; color: var(--text); margin-bottom: 2px; }
.gallery-card-use { font-size: 10px; color: var(--accent2); cursor: pointer;
                    background: none; border: none; padding: 0; }
.gallery-card-use:hover { text-decoration: underline; }
.gallery-loading { grid-column: 1/-1; display: flex; flex-direction: column;
                   align-items: center; justify-content: center; padding: 60px; gap: 14px; }

/* ─── RIGHT PANEL ─── */
.right-panel { width: 240px; min-width: 240px; background: var(--bg2);
               border-left: 1px solid var(--border); overflow-y: auto; flex-shrink: 0; }
.right-panel::-webkit-scrollbar { width: 5px; }
.right-panel::-webkit-scrollbar-thumb { background: var(--bg5); border-radius: 3px; }
.rp-section { padding: 10px 14px; }
.rp-label { font-size: 9px; font-weight: 800; letter-spacing: 1px; color: var(--text3);
            text-transform: uppercase; margin-bottom: 8px; }
.info-grid { display: grid; grid-template-columns: auto 1fr; gap: 4px 10px; font-size: 11px; }
.info-key { color: var(--text3); }
.info-val { color: var(--text); font-weight: 500; }
.rp-divider { height: 1px; background: var(--border); margin: 0; }

/* History list */
.history-list { display: flex; flex-direction: column; gap: 3px; max-height: 160px; overflow-y: auto; }
.history-list::-webkit-scrollbar { width: 4px; }
.history-list::-webkit-scrollbar-thumb { background: var(--bg5); }
.hist-item { font-size: 11px; color: var(--text2); padding: 4px 8px; border-radius: 4px;
             background: var(--bg3); display: flex; align-items: center; gap: 6px; }
.hist-dot { width: 6px; height: 6px; border-radius: 50%; background: var(--accent); flex-shrink: 0; }
.hist-dot.redo { background: var(--text3); }
.hist-empty { font-size: 11px; color: var(--text3); }

/* Quick actions */
.quick-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 6px; }
.q-btn { background: var(--bg4); border: 1px solid var(--border2); color: var(--text);
         font-size: 11px; font-weight: 600; padding: 8px 6px; cursor: pointer;
         border-radius: 6px; transition: all 0.15s; text-align: center; }
.q-btn:hover { background: var(--bg5); border-color: var(--accent3); }
.reset-btn { width: 100%; margin-top: 10px; background: rgba(224, 82, 82, 0.15);
             border: 1px solid var(--red); color: var(--red); font-size: 12px;
             font-weight: 600; padding: 9px; cursor: pointer; border-radius: 6px; }
.reset-btn:hover { background: rgba(224, 82, 82, 0.25); }

/* Param slider section */
.param-block { padding: 8px 14px; border-top: 1px solid var(--border); }
.param-row { margin-bottom: 14px; }
.param-head { display: flex; justify-content: space-between; margin-bottom: 5px; }
.param-name { font-size: 11px; color: var(--text2); }
.param-val { font-size: 12px; font-weight: 700; color: var(--accent2); min-width: 36px; text-align: right; }
input[type=range] { width: 100%; accent-color: var(--accent); height: 4px; cursor: pointer; }
.apply-btn { width: 100%; margin-top: 4px; background: var(--accent3); border: none;
             color: white; font-size: 12px; font-weight: 700; padding: 10px;
             cursor: pointer; border-radius: 6px; }
.apply-btn:hover { background: var(--accent); }

/* ─── STATUS BAR ─── */
.statusbar { display: flex; align-items: center; gap: 12px; padding: 4px 14px;
             background: var(--bg2); border-top: 1px solid var(--border);
             font-size: 11px; color: var(--text3); flex-shrink: 0; }
.status-indicator { width: 6px; height: 6px; border-radius: 50%; background: var(--green); flex-shrink: 0; }
.status-indicator.busy { background: var(--yellow); animation: pulse 1s infinite; }
@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.3} }
.status-text { color: var(--text2); flex: 1; }
.zoom-text { color: var(--text3); }
</style>
</head>
<body>
<!-- TOP BAR -->
<div class="topbar">
  <div class="topbar-title">ImageProc <span>C++ Engine</span></div>
  <div class="tb-sep"></div>
  <button class="tb-btn" onclick="openFile()">📂 Open</button>
  <button class="tb-btn" id="saveBtn" onclick="saveFile()" disabled>💾 Save</button>
  <div class="tb-sep"></div>
  <button class="tb-btn" id="undoBtn" onclick="undo()" disabled>↩ Undo</button>
  <button class="tb-btn" id="redoBtn" onclick="redo()" disabled>↪ Redo</button>
  <div class="tb-sep"></div>
  <button class="tb-btn" id="baBtn" onclick="toggleBA()" disabled>⇔ Before/After</button>
  <div class="tb-sep"></div>
  <button class="tb-btn" onclick="zoomIn()">＋</button>
  <button class="tb-btn" onclick="zoomOut()">－</button>
  <button class="tb-btn" onclick="fitWindow()">⊞ Fit</button>
  <div class="tb-spacer"></div>
  <div class="mode-tabs">
    <div class="mode-tab active" id="tabEditor" onclick="switchMode('editor')">Editor</div>
    <div class="mode-tab" id="tabGallery" onclick="switchMode('gallery')">Gallery</div>
  </div>
</div>

<!-- MAIN -->
<div class="main">
  <!-- SIDEBAR -->
  <div class="sidebar" id="sidebar"></div>

  <!-- CENTER -->
  <div class="center">
    <!-- EDITOR MODE -->
    <div id="editorMode">
      <div class="canvas-wrap" id="canvasWrap">
        <div class="drop-hint" id="dropHint">
          <div class="icon">🖼</div>
          <div class="msg">Drop an image to get started</div>
          <div class="sub">Supports PNG, JPG, BMP</div>
          <button class="upload-btn" onclick="openFile()">Browse Files</button>
        </div>
        <!-- Before/After -->
        <div class="ba-container" id="baContainer">
          <div class="ba-img ba-after" id="baAfterDiv"><img id="baAfterImg"/></div>
          <div class="ba-img ba-before" id="baBeforeDiv" style="clip-path: inset(0 50% 0 0)"><img id="baBeforeImg"/></div>
          <div class="ba-divider" id="baDivider">
            <div class="ba-handle">⇔</div>
          </div>
          <div class="ba-label left">ORIGINAL</div>
          <div class="ba-label right">PROCESSED</div>
        </div>
        <img id="mainImg" style="display:none; transform: translate(-50%,-50%)"/>
        <div class="spinner" id="spinner">
          <div class="spin-box">
            <div class="spin-ring"></div>
            <div class="spin-text" id="spinText">Processing…</div>
          </div>
        </div>
      </div>
    </div>

    <!-- GALLERY MODE -->
    <div id="galleryMode">
      <div class="gallery-header">
        <div class="gallery-title">All Operations Gallery</div>
        <button class="tb-btn primary" id="runGalleryBtn" onclick="runGallery()" disabled>▶ Run All</button>
        <span style="font-size:11px;color:var(--text3);margin-left:4px" id="galleryStatus"></span>
      </div>
      <div class="gallery-grid" id="galleryGrid">
        <div class="gallery-loading">
          <div style="font-size:36px;opacity:0.3">🖼</div>
          <div style="font-size:13px;color:var(--text3)">Open an image and click Run All to see all 26 operations at once</div>
        </div>
      </div>
    </div>
  </div>

  <!-- RIGHT PANEL -->
  <div class="right-panel" id="rightPanel">
    <div class="rp-section">
      <div class="rp-label">Image Info</div>
      <div class="info-grid" id="imageInfo">
        <span class="info-key">Status</span><span class="info-val" style="color:var(--text3)">No image</span>
      </div>
    </div>
    <div class="rp-divider"></div>
    <div class="rp-section">
      <div class="rp-label">History</div>
      <div class="history-list" id="histList"><div class="hist-empty">No operations yet</div></div>
    </div>
    <div class="rp-divider"></div>
    <div class="rp-section">
      <div class="rp-label">Quick Presets</div>
      <div class="quick-grid">
        <div class="q-btn" onclick="applyOp('histogram_equalization',{})">Auto Enhance</div>
        <div class="q-btn" onclick="applyOp('grayscale',{})">B &amp; W</div>
        <div class="q-btn" onclick="applyOp('sharpen',{})">Sharpen</div>
        <div class="q-btn" onclick="applyVintage()">Vintage</div>
        <div class="q-btn" onclick="applyOp('invert',{})">Invert</div>
        <div class="q-btn" onclick="applyOp('edge_detection',{})">Edges</div>
      </div>
      <button class="reset-btn" onclick="resetOriginal()">↩ Reset to Original</button>
    </div>
    <div id="paramSection" style="display:none"></div>
  </div>
</div>

<!-- STATUS BAR -->
<div class="statusbar">
  <div class="status-indicator" id="statusDot"></div>
  <div class="status-text" id="statusText">Ready — C++ engine</div>
  <div class="zoom-text" id="zoomText">100%</div>
</div>

<input type="file" id="fileInput" accept="image/*" style="display:none">

<script>
const OPS = {
  "Adjustments": {
    color: "adjust",
    items: [
      {label:"Brightness",    op:"brightness",   params:{delta:{min:-255,max:255,step:1,def:0,type:"int"}}},
      {label:"Contrast",      op:"contrast",     params:{factor:{min:0,max:3,step:0.05,def:1.5,type:"float"}}},
      {label:"Hue Shift",     op:"hue_shift",    params:{degrees:{min:-180,max:180,step:1,def:0,type:"int"}}},
      {label:"Saturation",    op:"saturation",   params:{factor:{min:0,max:3,step:0.05,def:1.0,type:"float"}}},
    ]
  },
  "Color": {
    color: "color",
    items: [
      {label:"Grayscale",     op:"grayscale"},
      {label:"Sepia",         op:"sepia"},
      {label:"Invert",        op:"invert"},
      {label:"Red Channel",   op:"channel_red"},
      {label:"Green Channel", op:"channel_green"},
      {label:"Blue Channel",  op:"channel_blue"},
    ]
  },
  "Filters": {
    color: "filter",
    items: [
      {label:"Gaussian Blur", op:"gaussian_blur", params:{kernelSize:{min:3,max:21,step:2,def:5,type:"int"}}},
      {label:"Box Blur",      op:"box_blur",      params:{size:{min:3,max:21,step:2,def:3,type:"int"}}},
      {label:"Median Filter", op:"median_filter", params:{size:{min:3,max:11,step:2,def:3,type:"int"}}},
      {label:"Sharpen",       op:"sharpen"},
      {label:"Edge Detect",   op:"edge_detection"},
      {label:"Emboss",        op:"emboss"},
    ]
  },
  "Transform": {
    color: "transform",
    items: [
      {label:"Rotate 90° CW",  op:"rotate_cw"},
      {label:"Rotate 90° CCW", op:"rotate_ccw"},
      {label:"Flip Horizontal",op:"horizontal_flip"},
      {label:"Flip Vertical",  op:"vertical_flip"},
      {label:"Crop",           op:"crop", params:{x:{min:0,max:1000,step:1,def:0,type:"int"}, y:{min:0,max:1000,step:1,def:0,type:"int"}, width:{min:10,max:2000,step:1,def:200,type:"int"}, height:{min:10,max:2000,step:1,def:200,type:"int"}}},
      {label:"Resize",         op:"resize", params:{width:{min:10,max:4000,step:1,def:400,type:"int"}, height:{min:10,max:4000,step:1,def:400,type:"int"}}},
    ]
  },
  "Advanced": {
    color: "advanced",
    items: [
      {label:"Histogram Eq.",  op:"histogram_equalization"},
      {label:"Posterize",      op:"posterize", params:{levels:{min:2,max:32,step:1,def:4,type:"int"}}},
      {label:"Dither",         op:"dither"},
      {label:"Vignette",       op:"vignette",  params:{intensity:{min:0,max:2,step:0.05,def:0.5,type:"float"}}},
    ]
  }
};

// State
let currentFile = null, originalFile = null;
let undoStack = [], redoStack = [];
let zoom = 1, imgW = 0, imgH = 0;
let baActive = false, baDragging = false, baPos = 0.5;
let selectedOp = null;
let currentMode = 'editor';
let historyLog = [];

// Build sidebar
const sb = document.getElementById('sidebar');
for (const [sec, group] of Object.entries(OPS)) {
  const hdr = document.createElement('div');
  hdr.className = 'sec-hdr';
  hdr.textContent = sec;
  sb.appendChild(hdr);
  for (const item of group.items) {
    const el = document.createElement('div');
    el.className = 'op-item';
    el.dataset.op = item.op;
    el.innerHTML = `<div class="op-dot ${group.color}"></div>${item.label}`;
    el.onclick = () => handleOpClick(item, el);
    sb.appendChild(el);
  }
}

function handleOpClick(item, el) {
  document.querySelectorAll('.op-item').forEach(e => e.classList.remove('active'));
  el.classList.add('active');
  selectedOp = item;

  if (!item.params) {
    // No params — apply immediately
    document.getElementById('paramSection').style.display = 'none';
    applyOp(item.op, {});
  } else {
    // Show param panel in right sidebar
    showParamPanel(item);
  }
}

function showParamPanel(item) {
  const ps = document.getElementById('paramSection');
  ps.style.display = 'block';
  const paramVals = {};
  for (const [k, c] of Object.entries(item.params)) paramVals[k] = c.def;

  let html = `<div class="rp-divider"></div><div class="param-block"><div class="rp-label">${item.label} — Parameters</div>`;
  for (const [key, cfg] of Object.entries(item.params)) {
    html += `
      <div class="param-row">
        <div class="param-head">
          <span class="param-name">${key.replace(/_/g,' ')}</span>
          <span class="param-val" id="pv_${key}">${cfg.def}</span>
        </div>
        <input type="range" id="ps_${key}" min="${cfg.min}" max="${cfg.max}" step="${cfg.step}" value="${cfg.def}">
      </div>`;
  }
  html += `<button class="apply-btn" onclick="applyFromPanel()">Apply</button></div>`;
  ps.innerHTML = html;

  for (const [key, cfg] of Object.entries(item.params)) {
    const sl = document.getElementById('ps_' + key);
    const vl = document.getElementById('pv_' + key);
    sl.oninput = () => {
      const v = cfg.type === 'int' ? parseInt(sl.value) : parseFloat(sl.value);
      vl.textContent = cfg.type === 'float' ? v.toFixed(2) : v;
    };
  }
}

function applyFromPanel() {
  if (!selectedOp || !selectedOp.params) return;
  const params = {};
  for (const [key, cfg] of Object.entries(selectedOp.params)) {
    const sl = document.getElementById('ps_' + key);
    params[key] = cfg.type === 'int' ? parseInt(sl.value) : parseFloat(sl.value);
  }
  applyOp(selectedOp.op, params);
}

// File handling
document.getElementById('fileInput').onchange = e => {
  if (e.target.files[0]) uploadFile(e.target.files[0]);
};
function openFile() { document.getElementById('fileInput').click(); }

const cw = document.getElementById('canvasWrap');
cw.addEventListener('dragover', e => { e.preventDefault(); cw.classList.add('drop-zone-active'); });
cw.addEventListener('dragleave', () => cw.classList.remove('drop-zone-active'));
cw.addEventListener('drop', e => {
  e.preventDefault(); cw.classList.remove('drop-zone-active');
  if (e.dataTransfer.files[0]) uploadFile(e.dataTransfer.files[0]);
});

async function uploadFile(file) {
  setStatus('Loading image…', true);
  const reader = new FileReader();
  reader.onload = async () => {
    try {
      const res = await fetch('/upload', {
        method: 'POST', headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({image: reader.result})
      });
      const data = await res.json();
      if (data.ok) {
        originalFile = data.file;
        currentFile = data.file;
        undoStack = []; redoStack = [];
        historyLog = [];
        showImage(data.file);
        setStatus('Loaded: ' + file.name, false);
        document.getElementById('runGalleryBtn').disabled = false;
      } else { alert('Upload failed: ' + data.error); setStatus('Error', false); }
    } catch(e) { alert('Network error: ' + e); setStatus('Error', false); }
  };
  reader.readAsDataURL(file);
}

function showImage(fname) {
  const img = document.getElementById('mainImg');
  img.onload = () => {
    imgW = img.naturalWidth; imgH = img.naturalHeight;
    fitWindow();
    updateInfo();
    updateUndoRedo();
    document.getElementById('dropHint').style.display = 'none';
    document.getElementById('saveBtn').disabled = false;
    document.getElementById('baBtn').disabled = false;
    if (baActive) updateBA();
  };
  img.src = '/tmp/' + fname + '?t=' + Date.now();
  img.style.display = 'block';
  img.style.transform = `translate(-50%, -50%) scale(${zoom})`;
}

async function applyOp(op, params) {
  if (!currentFile) { alert('Open an image first.'); return; }
  // Ensure odd values for kernel sizes
  for (const k of ['kernelSize','size']) {
    if (params[k] !== undefined) { let v=parseInt(params[k]); if(v%2===0)v++; params[k]=v; }
  }

  const prevFile = currentFile;
  setStatus('Applying ' + op + '…', true);
  document.getElementById('spinner').classList.add('show');
  document.getElementById('spinText').textContent = 'Applying ' + op + '…';

  try {
    const res = await fetch('/apply', {
      method: 'POST', headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({input: currentFile, operation: op, params})
    });
    const data = await res.json();
    if (data.ok) {
      undoStack.push(prevFile);
      redoStack = [];
      currentFile = data.file;
      historyLog.push({op, params, direction:'apply'});
      showImage(data.file);
      setStatus('✓ ' + op, false);
      if (baActive) {
        document.getElementById('baAfterImg').src = '/tmp/' + data.file + '?t=' + Date.now();
        syncBASize();
      }
    } else {
      setStatus('Error: ' + data.error, false);
      alert('Error applying ' + op + ': ' + data.error);
    }
  } catch(e) { setStatus('Network error', false); }
  document.getElementById('spinner').classList.remove('show');
}

async function applyVintage() {
  await applyOp('sepia', {});
  await applyOp('vignette', {intensity: 0.7});
  await applyOp('brightness', {delta: -15});
}

function undo() {
  if (!undoStack.length) return;
  redoStack.push(currentFile);
  currentFile = undoStack.pop();
  showImage(currentFile);
  setStatus('Undone', false);
  updateUndoRedo();
}
function redo() {
  if (!redoStack.length) return;
  undoStack.push(currentFile);
  currentFile = redoStack.pop();
  showImage(currentFile);
  setStatus('Redone', false);
  updateUndoRedo();
}
function resetOriginal() {
  if (!originalFile) return;
  undoStack.push(currentFile);
  redoStack = [];
  currentFile = originalFile;
  showImage(currentFile);
  setStatus('Reset to original', false);
  updateUndoRedo();
}
function saveFile() {
  if (!currentFile) return;
  const a = document.createElement('a');
  a.href = '/tmp/' + currentFile + '?t=' + Date.now();
  a.download = 'processed.png';
  a.click();
}

// ─── Before/After ───
function toggleBA() {
  if (!currentFile || !originalFile) return;
  baActive = !baActive;
  const btn = document.getElementById('baBtn');
  const container = document.getElementById('baContainer');
  btn.textContent = baActive ? '✕ Close B/A' : '⇔ Before/After';
  btn.style.background = baActive ? 'var(--accent3)' : '';

  if (baActive) {
    container.classList.add('active');
    const ts = '?t=' + Date.now();
    document.getElementById('baBeforeImg').src = '/tmp/' + originalFile + ts;
    document.getElementById('baAfterImg').src  = '/tmp/' + currentFile + ts;
    baPos = 0.5;
    syncBASize();
  } else {
    container.classList.remove('active');
  }
}

function syncBASize() {
  const img = document.getElementById('mainImg');
  const w = img.naturalWidth * zoom;
  const h = img.naturalHeight * zoom;
  const area = document.getElementById('canvasWrap');
  const ox = area.clientWidth/2;
  const oy = area.clientHeight/2;
  const cx = `translate(${ox - w/2}px, ${oy - h/2}px) scale(${zoom})`;
  const transform = `translate(-50%, -50%) scale(${zoom})`;

  for (const id of ['baBeforeImg','baAfterImg']) {
    const el = document.getElementById(id);
    el.style.transform = transform;
    el.style.width = img.naturalWidth + 'px';
    el.style.height = img.naturalHeight + 'px';
  }

  const divEl = document.getElementById('baDivider');
  const pctX = baPos * 100;
  divEl.style.left = pctX + '%';
  document.getElementById('baBeforeDiv').style.clipPath = `inset(0 ${100 - pctX}% 0 0)`;
}

// B/A drag
const divider = document.getElementById('baDivider');
divider.addEventListener('mousedown', e => { e.preventDefault(); baDragging = true; });
document.addEventListener('mousemove', e => {
  if (!baDragging) return;
  const area = document.getElementById('canvasWrap');
  const rect = area.getBoundingClientRect();
  baPos = Math.max(0.02, Math.min(0.98, (e.clientX - rect.left) / rect.width));
  const pctX = baPos * 100;
  document.getElementById('baDivider').style.left = pctX + '%';
  document.getElementById('baBeforeDiv').style.clipPath = `inset(0 ${100 - pctX}% 0 0)`;
});
document.addEventListener('mouseup', () => { baDragging = false; });

// ─── Gallery Mode ───
function switchMode(mode) {
  currentMode = mode;
  document.getElementById('tabEditor').classList.toggle('active', mode === 'editor');
  document.getElementById('tabGallery').classList.toggle('active', mode === 'gallery');
  document.getElementById('editorMode').style.display = mode === 'editor' ? 'block' : 'none';
  document.getElementById('galleryMode').style.display = mode === 'gallery' ? 'flex' : 'none';
  document.getElementById('galleryMode').classList.toggle('active', mode === 'gallery');
}

async function runGallery() {
  if (!currentFile) { alert('Open an image first.'); return; }
  const btn = document.getElementById('runGalleryBtn');
  btn.disabled = true;
  btn.textContent = '⏳ Running…';

  const allOps = [];
  for (const [sec, group] of Object.entries(OPS)) {
    for (const item of group.items) {
      const params = {};
      if (item.params) {
        for (const [k,c] of Object.entries(item.params)) params[k] = c.def;
      }
      allOps.push({op: item.op, label: item.label, params});
    }
  }

  const grid = document.getElementById('galleryGrid');
  grid.innerHTML = `<div class="gallery-loading"><div class="spin-ring" style="width:32px;height:32px"></div><div style="font-size:13px;color:var(--text2)">Running all 26 operations…</div></div>`;

  document.getElementById('galleryStatus').textContent = 'Running…';

  try {
    const res = await fetch('/apply_batch', {
      method: 'POST', headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({input: currentFile, operations: allOps.map(a=>({op:a.op, params:a.params}))})
    });
    const data = await res.json();

    grid.innerHTML = '';
    for (let i = 0; i < data.results.length; i++) {
      const r = data.results[i];
      const label = allOps[i].label;
      const card = document.createElement('div');
      card.className = 'gallery-card';
      if (r.ok) {
        card.innerHTML = `
          <img class="gallery-card-img" src="/tmp/${r.file}?t=${Date.now()}" loading="lazy">
          <div class="gallery-card-info">
            <div class="gallery-card-name">${label}</div>
            <button class="gallery-card-use" onclick="useFromGallery('${r.file}','${label}')">Use this result →</button>
          </div>`;
      } else {
        card.innerHTML = `
          <div class="gallery-card-img" style="display:flex;align-items:center;justify-content:center;font-size:11px;color:var(--red);padding:10px">${r.error || 'Failed'}</div>
          <div class="gallery-card-info"><div class="gallery-card-name">${label}</div></div>`;
      }
      grid.appendChild(card);
    }
    document.getElementById('galleryStatus').textContent = `${data.results.filter(r=>r.ok).length} / ${data.results.length} done`;
  } catch(e) {
    grid.innerHTML = `<div class="gallery-loading"><div style="color:var(--red)">Network error: ${e}</div></div>`;
  }

  btn.disabled = false;
  btn.textContent = '▶ Run All';
}

function useFromGallery(fname, label) {
  undoStack.push(currentFile);
  redoStack = [];
  currentFile = fname;
  historyLog.push({op: label});
  switchMode('editor');
  showImage(fname);
  setStatus('Applied: ' + label, false);
  updateUndoRedo();
}

// ─── Zoom ───
function zoomIn()  { zoom = Math.min(20, zoom * 1.2); applyZoom(); }
function zoomOut() { zoom = Math.max(0.05, zoom / 1.2); applyZoom(); }
function fitWindow() {
  const img = document.getElementById('mainImg');
  if (!img.naturalWidth) return;
  const area = document.getElementById('canvasWrap');
  const sx = area.clientWidth / img.naturalWidth;
  const sy = area.clientHeight / img.naturalHeight;
  zoom = Math.min(sx, sy) * 0.9;
  applyZoom();
}
function applyZoom() {
  const img = document.getElementById('mainImg');
  img.style.transform = `translate(-50%, -50%) scale(${zoom})`;
  document.getElementById('zoomText').textContent = Math.round(zoom * 100) + '%';
  if (baActive) syncBASize();
}

// Scroll to zoom
document.getElementById('canvasWrap').addEventListener('wheel', e => {
  e.preventDefault();
  if (e.deltaY < 0) zoomIn(); else zoomOut();
}, {passive: false});

// ─── UI State ───
function updateInfo() {
  const el = document.getElementById('imageInfo');
  el.innerHTML = `
    <span class="info-key">Size</span><span class="info-val">${imgW} × ${imgH}</span>
    <span class="info-key">Pixels</span><span class="info-val">${(imgW*imgH).toLocaleString()}</span>
    <span class="info-key">Engine</span><span class="info-val">C++17</span>
    <span class="info-key">Format</span><span class="info-val">RGBA PNG</span>
  `;
}

function updateUndoRedo() {
  document.getElementById('undoBtn').disabled = !undoStack.length;
  document.getElementById('redoBtn').disabled = !redoStack.length;

  const histEl = document.getElementById('histList');
  if (!undoStack.length && !redoStack.length) {
    histEl.innerHTML = '<div class="hist-empty">No operations yet</div>';
    return;
  }
  let html = '';
  undoStack.slice(-6).reverse().forEach((f, i) => {
    const entry = historyLog[undoStack.length - 1 - i];
    html += `<div class="hist-item"><div class="hist-dot"></div>${entry ? entry.op : 'step'}</div>`;
  });
  redoStack.slice(-3).forEach((f, i) => {
    html += `<div class="hist-item"><div class="hist-dot redo"></div><span style="opacity:.5">↪ redo</span></div>`;
  });
  histEl.innerHTML = html || '<div class="hist-empty">Empty</div>';
}

function setStatus(text, busy) {
  document.getElementById('statusText').textContent = text;
  document.getElementById('statusDot').className = 'status-indicator' + (busy ? ' busy' : '');
}

// Keyboard
document.addEventListener('keydown', e => {
  if ((e.metaKey||e.ctrlKey) && e.key==='o') { e.preventDefault(); openFile(); }
  if ((e.metaKey||e.ctrlKey) && e.key==='s') { e.preventDefault(); saveFile(); }
  if ((e.metaKey||e.ctrlKey) && e.key==='z') { e.preventDefault(); undo(); }
  if ((e.metaKey||e.ctrlKey) && e.key==='y') { e.preventDefault(); redo(); }
  if ((e.metaKey||e.ctrlKey) && e.key==='b') { e.preventDefault(); toggleBA(); }
});
</script>
</body>
</html>
'''


def main():
    if not CPP_BINARY.exists():
        print(f"ERROR: C++ binary not found at {CPP_BINARY}")
        print("Run 'make app' first.")
        sys.exit(1)

    print("╔══════════════════════════════════════════╗")
    print(f"║   Image Processor — C++ Engine GUI       ║")
    print(f"║   Open http://localhost:{PORT} in browser   ║")
    print("╚══════════════════════════════════════════╝")

    server = HTTPServer(("localhost", PORT), ImageHandler)

    def open_browser():
        time.sleep(0.5)
        webbrowser.open(f"http://localhost:{PORT}")
    threading.Thread(target=open_browser, daemon=True).start()

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down…")
    finally:
        shutil.rmtree(TMP_DIR, ignore_errors=True)
        server.server_close()


if __name__ == "__main__":
    main()
