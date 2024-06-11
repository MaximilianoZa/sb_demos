# Creating and Running Your Crank Container

## OVERVIEW

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

## SET UP
This setup was used for building and testing. While it is not necessary to replicate it exactly, it serves as a useful guide.

Software:
- Storyboard v8.1
- Windows v10
- Terra Term v4.106
- Docker Desktop v4.25.2
- Docker Engine v24.0.6
- Visual Studio Code v1.87.0

Embedded software:
- Linux verdin-imx8mp-07174518 5.4.193-5.7.0+git.f78299297185 #1-TorizonCore SMP PREEMPT Mon Jul 11 14:42:03 UTC 2022 aarch64 aarch64 aarch64 GNU/Linux
- Docker v19.03.14-ce

Hardware:
- Toradex Verdin: <https://www.toradex.com/de/computer-on-modules/verdin-arm-family>
- Gechic Touch Monitor T151A: <https://www.gechic.com/de/touch-monitor>

## TARGET DEVICE PREPARATION: PULLING TORIZON CONTAINER TO ACCESS THE WAYLAND COMPOSITOR

Torizon OS is a container-based embedded system. Which means that the applications run in isolated environments called containers.
To run GPU accelerated applications on Linux in general you need some components in the userspace and some other components in the kernel space. 
The Crank application, when using GPU acceleration, needs access to the OpenGL graphics API. For that we need to run first the Torizon base container with the Graphical libraries.
Different devices require different configurations, this is way Torizon provides different base containers for different platforms.
a couple of examples of this:

- Command to run Wayland based container on a IMX8 in Torizon OS 6:
`docker run -e ACCEPT_FSL_EULA=1 -d --rm --name=weston --net=host --cap-add CAP_SYS_TTY_CONFIG  -v /dev:/dev -v /tmp:/tmp -v /run/udev/:/run/udev/ --device-cgroup-rule='c 4:* rmw' --device-cgroup-rule='c 13:* rmw' --device-cgroup-rule='c 199:* rmw' --device-cgroup-rule='c 226:* rmw' torizon/weston-vivante:$CT_TAG_WESTON_VIVANTE --developer --tty=/dev/tty7`

- Command to run Wayland based container on a TI62 in Torizon OS 6:
`docker run -e ACCEPT_FSL_EULA=1 -d --rm --name=weston --net=host --cap-add CAP_SYS_TTY_CONFIG -v /dev:/dev -v /tmp:/tmp -v /run/udev/:/run/udev/ --device-cgroup-rule='c 4:* rmw' --device-cgroup-rule='c 13:* rmw' --device-cgroup-rule='c 226:* rmw' torizon/weston-am62:$CT_TAG_WESTON --developer --tty=/dev/tty7`

Notes:
- Note1: if you run this command and the image is not in your system docker will automatically look for it on the internet, pull it first and run it after.
- Note2: The CT_TAG variables are already set on Torizon. Example, on your IMX8 you can echo ${CT_TAG_WAYLAND_BASE_VIVANTE}
- Note3: Torizon OS 6 command references to run Weston container on different platforms: <https://developer.toradex.com/torizon/application-development/use-cases/gui/web-browser-kiosk-mode-with-torizoncore/>

After successfully launching your weston container you should see a grey screen (may be blue or different color depending on the container you are using) that looks like this:

![alt text](https://github.com/MaximilianoZa/sb_demos/blob/main/docker_containers_torizoncoreos6xy/images/weston_container_up.png?raw=true)


if you run docker `-ps` you should see the container with the status `Up`:

Notes: 
- Note1: To enter your container you can use this command: 	
`docker exec -t -i [ContainerID] /bin/bash`
- Note2: To check Weston Information, you can run:	
`weston --version`
weston-info
- Note3: Find your weston.ini configuration with this command:	
`find / -name *weston.ini*`
- Note4: To edit the weston.ini file you probably must copy it outside the container first, edit it and copy it back inside.


