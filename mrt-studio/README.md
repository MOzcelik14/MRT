# MRT Studio

**MRT Studio** is a lightweight, modern, native Linux IDE for the MRT programming language.

Built from the ground up in standard C using **GTK4** and **GtkSourceView 5**, MRT Studio provides a fast, responsive, and distraction-free developer experience inspired by classic environments (Visual Basic, Visual C#), without the resource bloat of Electron or web technologies.

```text
┌──────────────────────────────────────────────┐
│ File Edit View Run Help        ▶ Run  ■ Stop│
├──────────────┬───────────────────────────────┤
│ Project      │ main.mrt                      │
│              │                               │
│ main.mrt     │ let x = 10                    │
│ utils.mrt    │ print(x)                      │
│              │                               │
├──────────────┴───────────────────────────────┤
│ Output / Errors                              │
│ > 10                                         │
└──────────────────────────────────────────────┘
```

---

## Screenshot

```text
+-----------------------------------------------------------------------------------+
|  [MRT Studio]  File  Edit  View  Run  Help                  [ ▶ Run ]  [ ■ Stop ] |
+------------------+----------------------------------------------------------------+
| Project          | main.mrt *                                                     |
|                  | 1 | // Welcome to MRT Studio                                   |
| > hello-project  | 2 | fn selamla(kisi) {                                         |
|   |-- main.mrt   | 3 |     print("Merhaba " + kisi)                               |
|   `-- utils.mrt  | 4 | }                                                          |
|                  | 5 |                                                            |
|                  | 6 | selamla("Murat")                                           |
+------------------+----------------------------------------------------------------+
| Output / Errors                                                               [X] |
| Running main.mrt...                                                               |
|                                                                                   |
| Merhaba Murat                                                                     |
|                                                                                   |
| Process finished with exit code 0                                                 |
+-----------------------------------------------------------------------------------+
```

---

## Features

- **Native Linux & GTK4**: Clean, modern interface fully integrated with system dark and light themes.
- **MRT Syntax Highlighting**: Dedicated GtkSourceView 5 language specification (`data/mrt.lang`) with distinct styling for keywords, built-ins, strings, numbers, operators, and comments.
- **Advanced Code Editing**:
  - Line numbering & current line highlighting
  - Bracket matching & auto-indentation
  - Tab width configuration (tabs or spaces)
  - Full Undo / Redo history (`Ctrl+Z`, `Ctrl+Shift+Z`)
  - Incremental Find & Replace (`Ctrl+F`, `Ctrl+H`)
- **Tabbed Document Interface**: Open and manage multiple `.mrt` files simultaneously in tabs with unsaved change tracking (`filename.mrt *`) and close confirmation dialogs.
- **Project Explorer**:
  - Open any directory as a project with a structured file tree.
  - Prioritizes `.mrt` source files.
  - "New Project" wizard to scaffold starter MRT projects (`main.mrt`).
- **Integrated MRT Runner**:
  - Run active `.mrt` script with `F5` or `▶ Run`.
  - Non-blocking asynchronous stdout/stderr streaming via `GSubprocess`.
  - Stop button (`■ Stop`) to forcefully terminate running processes.
  - Automatic `mrt` interpreter detection via `PATH`, local build directory, or custom user setting.
- **Clickable Error Navigation**:
  - Clicking on error locations like `main.mrt:8:12` in the output panel immediately opens the file and jumps cursor directly to line 8, column 12.
- **Configurable Settings**:
  - Font family and size
  - Tab width and spaces vs. tabs
  - Line numbers and word wrap toggles
  - Custom MRT interpreter executable path
  - Persistent configuration stored in `~/.config/mrt-studio/settings.ini`.

---

## Dependencies

- **Compiler**: GCC (11+) or Clang (13+)
- **Build tool**: GNU Make
- **Libraries**:
  - `gtk4` (>= 4.6)
  - `gtksourceview-5` (>= 5.0)
  - `glib-2.0` / `gio-2.0` (>= 2.70)

### Ubuntu / Debian Installation:
```bash
sudo apt update
sudo apt install -y libgtk-4-dev libgtksourceview-5-dev
```

*Note: MRT Studio also includes an automatic fallback for header bundles if dev packages are not installed globally.*

---

## Building

Clone or navigate to the `mrt-studio/` directory:

```bash
cd mrt-studio
```

Compile release binary:
```bash
make
```

Compile debug binary:
```bash
make debug
```

Clean build artifacts:
```bash
make clean
```

---

## Usage

### Launching the IDE
```bash
./mrt-studio
```

### Opening a File directly:
```bash
./mrt-studio main.mrt
```

### Opening a Project Folder:
```bash
./mrt-studio ~/Projects/my-mrt-project
```

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+N` | New File |
| `Ctrl+O` | Open File |
| `Ctrl+S` | Save File |
| `Ctrl+Shift+S` | Save As… |
| `Ctrl+F` | Find |
| `Ctrl+H` | Replace |
| `Ctrl+Z` | Undo |
| `Ctrl+Shift+Z` / `Ctrl+Y` | Redo |
| `F5` | Run active `.mrt` file |
| `Ctrl+Q` | Quit MRT Studio |

---

## MRT Integration

When `▶ Run` (or `F5`) is pressed, MRT Studio:
1. Automatically saves any modifications in the active editor tab.
2. Locates the `mrt` interpreter (checks `settings.ini`, then `PATH`, then `../mrt`).
3. Launches the interpreter asynchronously using GLib's `GSubprocess`.
4. Streams outputs in real time to the bottom Output panel.
5. Highlights compiler and runtime errors with clickable links (`file.mrt:line:col`).

---

## Project Structure

```text
mrt-studio/
├── src/
│   ├── main.c           # CLI entry point
│   ├── application.c    # GtkApplication & accelerators
│   ├── application.h
│   ├── window.c         # Main IDE window, layouts, menus, actions
│   ├── window.h
│   ├── editor.c         # GtkSourceView tabs, buffers, find/replace
│   ├── editor.h
│   ├── project.c        # File tree explorer & project wizard
│   ├── project.h
│   ├── runner.c         # GSubprocess async execution & process control
│   ├── runner.h
│   ├── output.c         # Bottom console, styled logs, error link parser
│   ├── output.h
│   ├── settings.c       # Settings dialog & GKeyFile configuration
│   └── settings.h
├── data/
│   └── mrt.lang         # GtkSourceView 5 language specification for MRT
├── resources/
│   └── style.css        # CSS stylesheets for IDE theme
├── Makefile             # Build system
├── README.md            # Documentation
└── LICENSE              # MIT License
```

---

## Roadmap

### MRT Studio 0.1 (Current)
- [x] GTK4 + GtkSourceView 5 native implementation
- [x] Multi-tab editor with dirty tracking
- [x] MRT syntax highlighting (`mrt.lang`)
- [x] Project sidebar & New Project wizard
- [x] Integrated async runner (`▶ Run`, `■ Stop`)
- [x] Output console with clickable error locations
- [x] Preferences and settings persistence
- [x] Full standard keyboard shortcuts

### MRT Studio 0.2
- [ ] Code completion / Auto-complete popup
- [ ] Outline & symbol list (functions, variables)
- [ ] Jump to function navigation
- [ ] Project-wide search (Find in Files)
- [ ] Recent projects and files menu

### MRT Studio 0.3
- [ ] Integration with MRT Language Server Protocol (LSP)
- [ ] In-line syntax diagnostics & red wavy underlines
- [ ] Hover tooltips (types, signatures)
- [ ] Go-to-definition

### MRT Studio 0.4
- [ ] Visual GUI Designer (Visual Basic / Windows Forms style):
  ```text
  Toolbox
  ├── Window
  ├── Button
  ├── Label
  ├── Text Input
  └── Image
  ```
- [ ] Drag-and-drop form canvas generating MRT UI code

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
