# Creating and Running Your Crank Container

## Overview

This guide highlights the process of creating a Crank container, a Docker container that packages a Storyboard application for deployment on your target device. For this example, we will utilize Weston as the Wayland compositor. Specifically, we will use a Weston container to manage the graphical stack, leveraging GPU acceleration and simplifying the development process.

However, you can choose to:
- **Build everything in one container**: This is another viable design choice.
- **Run the Crank container with a software render engine**: An alternative approach based on your needs.
- **Not use the Weston container**: Install the Wayland drivers directly on the base system (not recommended).

These are all design decisions that can be adapted based on your specific requirements and constraints.
In this repository you will find several projects for creating different images for different target devices. You can choose the one that best suits your needs. In some cases, you may need to make some changes to your system requirements

## PREREQUISITES
Before going through this guide, we advise you to have at least:
- Basic knowledge in Docker, sufficient to push, pull and run containers. 
- Basic knowledge in Storyboard, sufficient to understand its development concept.
- Basic knowledge in Command line, to execute basic instructions in shell.
-	Docker installed on the target device (installed by default in Torizon).
-	Torizon Core 6.x.y installed on the target device.
-	Display screen connected to the target device.
-	LAN connection to access internet from the target device
-	Docker installed on the host PC.
-	Storyboard installed on the host PC.
-	Visual Studio Code installed on the host PC.
-	Terra Term or similar installed on the host PC / Any Software to scann your network

