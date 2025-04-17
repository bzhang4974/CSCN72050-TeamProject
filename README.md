# CSCN72050 - Milestone 3 Submission  
**Group Members**: Bowen Zhang, Henil Jariwala  
**Date**: 04/17/2025  

Contents:

CSCN72050-TeamProject.sln
    PktDef/           (Header file, source file, static library project)
    PktDefTests/      (Unit test project using Microsoft CppUnitTest)
    MySocket/         (Header and source files for UDP/TCP socket abstraction)
    RobotController/  (Crow-based web server serving GUI and command handling)
    static/           (Frontend files: index.html, style.css, script.js)
    Dockerfile        (For optional Linux container deployment)
    .gitignore
    README.txt

How to Run the Project:

1. Build the image:
    ```bash
    docker build -t robot_webserver .
2. Run the container:
    ```bash
    docker run -p 18080:18080 robot_webserver
3. Open your browser and visit `http://localhost:18080`.
4. In the GUI:
   - Enter the Robot IP (e.g., `127.0.0.1`) and port (e.g., `5000`)
   - Select **UDP** or **TCP**
   - Click **Connect**
   - Send commands (forward, backward, left, right, sleep)
   - Click **Get Telemetry** to receive data



To Run Unit Tests:

1. Right-click the `PktDefTests` project → Select “Set as Startup Project”.
2. Build the solution.
3. Open `Test Explorer` (View → Test Explorer).
4. Click “Run All Tests”.
