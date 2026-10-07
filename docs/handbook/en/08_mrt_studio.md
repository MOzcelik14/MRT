# Chapter 8 — MRT Studio IDE

## Overview

**MRT Studio** is the official lightweight, native, and modern Integrated Development Environment (IDE) tailored for the MRT programming language.

- **Technology**: Pure C, GTK4, GtkSourceView 5, GLib/GIO.
- **Fast & Minimal**: Zero Electron, WebView, or JavaScript runtime dependencies. Instant startup with minimal memory overhead.
- **Bilingual**: Seamless English and Turkish localization.

---

## Features

### 1. Code Editor
- Syntax highlighting via GtkSourceView 5 (`data/mrt.lang`).
- Line numbering, active line highlight, bracket matching.
- Autocompletion for keywords, built-ins, and local identifiers.
- Code formatting integration (`mrt fmt`, `Shift+Alt+F`).

### 2. Explorer and Symbol Outline
- Sidebar project file browser (`Files`).
- Live symbol tree (`Outline`) showing declared tasks and variables.

### 3. Execution and Diagnostics
- One-click execution with `F5` or the Toolbar **Run** button.
- Clean terminal output with distinct stdout and stderr streams.
- Interactive error hyperlinks that jump directly to source lines and columns.
- **Problems** panel parsing `mrt check` diagnostics into a structured table.

### 4. Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+N` | New File |
| `Ctrl+O` | Open File |
| `Ctrl+S` | Save File |
| `Ctrl+Shift+S` | Save As |
| `Ctrl+F` | Find in File |
| `Ctrl+H` | Replace |
| `Ctrl+Shift+F` | Find in Files |
| `Ctrl+G` | Go to Line |
| `Ctrl+P` | Quick Open |
| `Ctrl+Shift+P` | Command Palette |
| `Shift+Alt+F` | Format Document |
| `F5` | Run Active File |
| `Ctrl+B` | Toggle Sidebar |
| `Ctrl+J` | Toggle Output Panel |
