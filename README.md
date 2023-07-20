# node-red-garden-control
Garden control app using node-RED and MQTT protocol for communication between controller and device over a local network. Device functions are simulated by LED lights and interface is remotely accessible from devices connected to the same network.

<br />

Node-RED app and MQTT protocol were implemented on Raspberry Pi 3 model B which acts as a controller. This device is well known in the industry for its versatility. It supports remote app building and has more than enough processing power to process incoming requests.

<br />

Garden device is implemented on ESP32-DevKitM-1. This device offers flexibility and a wide range of applications. ESP32 architecture is proven to work well with a wide range devices and offers programming compatibility with Arduino IDE which was used for this project.

<br />

Devices connected to ESP32:

- DHT11 - air humidity

- BMP280 - temperature

- KY-018 - light intensity

- Capacitive soil moisture sensor v1.2 - soil moisture

- 1602 LCD display

<br />

Apps:

- Node-RED: https://nodered.org/

- MQTT: https://mosquitto.org/

- Arduino: https://www.arduino.cc/

<br />

System topology:

![System topology](https://github.com/dinoshoebill/node-red-garden-control/assets/94995989/eb60f616-bbaf-492f-874b-5ece9afb8b47)

<br />

Schematic:

![Schematic](https://github.com/dinoshoebill/node-red-garden-control/assets/94995989/06d64a1e-7486-4865-9a5f-16b96ccf8608)

<br />

App:

- Control Board, synchronization from device to controller (red button) was later removed due to unnecessity:

![Control board](https://github.com/dinoshoebill/node-red-garden-control/assets/94995989/86ce051d-5c3d-4317-b26a-ecdd3d127159)

- Graphs:

![Graphs](https://github.com/dinoshoebill/node-red-garden-control/assets/94995989/9956d936-cdcf-428d-9438-0cff9b832c6f)

- Meters:

![Meters](https://github.com/dinoshoebill/node-red-garden-control/assets/94995989/d9dd9cc5-33d1-487f-8502-a6c4802e50fe)

- Table:

![Table](https://github.com/dinoshoebill/node-red-garden-control/assets/94995989/b4e4360c-cd68-46d4-9cb1-1b2d6944b7f0)
