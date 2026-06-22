## Testing and Debugging High Level Control Software

This document is meant to document the known errors when running the high level control software and how to solve potential issues that can arise.

If a new unknown error occurs, please write down the solution in this document for future reference.

### Building the high level control software docker container

Build failed? Try to rebuild

### Controlling AP4 with controller

All nodes up and running and xbox controllers input is showing but propulsion and steering not working?
Wait a couple minutes and if it still doesn't work try restarting

### Gazebo and Rviz does not show up when starting the container

Output from console:

```
Authorization required, but no authorization protocol specified
```

Solution: Graphics permission have not been set, the docker container is not allowed to display GUI elements. Run this command from a terminal outside of the container and restart the docker container

```bash
xhost +local:*
```


