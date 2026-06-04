# BetaGo
AI algorithm to solve Go

## Getting Started

This workspace uses CMake for the BetaGo C++ project.

### Build

1. Open the workspace in VS Code.
2. Run the task `CMake: build` from the terminal or Command Palette.

### Run

- Use the VS Code debug configuration `Launch BetaGo`.
- The executable output is generated in `build/`.

### Frontend / Backend Integration

1. Build the project with `CMake: build`.
2. Run the backend server:
   - `python backend/server.py`
3. Open the browser at `http://127.0.0.1:8000`.
4. The browser frontend now uses the existing C++ backend logic as the API for board state and moves.
