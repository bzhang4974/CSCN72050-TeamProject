CSCN72050 - Milestone 2 Submission  
Group Members: Bowen Zhang, Henil Jariwala  
Date: 04/13/2025  

Contents:

    CSCN72050-TeamProject.sln
    PktDef/           (Header file, source file, static library project)
    PktDefTests/      (Unit test project using Microsoft CppUnitTest)
    MySocket/         (Header and source files for UDP/TCP socket abstraction)
    RobotController/  (Main application that sends DRIVE packets to Robot Simulator)
    .gitignore
    README.txt

How to Run the Project:

1. Open `CSCN72050-TeamProject.sln` in Visual Studio 2022.
2. Right-click the `RobotController` project → Select “Set as Startup Project”.
3. Build the solution.
4. Run `Robot__Simulator.exe` (ensure it uses the default UDP port 5000).
5. Run the RobotController project to send a DRIVE command.

To Run Unit Tests:

1. Right-click the `PktDefTests` project → Select “Set as Startup Project”.
2. Build the solution.
3. Open `Test Explorer` (View → Test Explorer).
4. Click “Run All Tests”.
