# Universal Remote

A universal infrared (IR) remote control system built using an ESP32 and a Flask web application. The system allows users to create and manage virtual remotes, assign buttons to them, learn IR commands from physical remotes, and transmit those commands through the ESP32.

## Overview

The project consists of two main components:

* **Web application** — Provides the user interface and REST API for managing remotes and buttons.
* **ESP32 system** — Handles IR signal reception/transmission and communicates with the web server over Wi-Fi.

The two components are contained in the same repository, with the ESP32 project located in the `esp32/` directory.

## Features

* Create and delete virtual remotes
* Rename existing remotes
* Create and delete remote buttons
* Rename buttons
* Learn IR commands from physical remotes
* Store learned IR commands in a SQLite database
* Transmit stored IR commands using the ESP32
* Web-based interface for controlling the system
* Wi-Fi communication between the ESP32 and Flask server

## Project Structure

```text
Universal_Remote_web/
├── app.py
├── database.py
├── templates/
│   ├── front_page.html
│   └── remote.html
├── static/
│   ├── css/
│   └── js/
│
└── esp32/
    ├── CMakeLists.txt
    ├── platformio.ini
    ├── sdkconfig.esp32dev
    ├── include/
    │   ├── LOGIN.h          # Local configuration; not included in Git
    │   ├── http.h
    │   ├── rx_tx.h
    │   └── wifi.h
    └── src/
        ├── CMakeLists.txt
        ├── http.c
        ├── main.c
        ├── rx_tx.c
        └── wifi.c
```

> **Note:** `LOGIN.h` is intentionally excluded from the repository because it contains private network credentials and server addresses.

## Configuration

The ESP32 requires a local `LOGIN.h` file containing the configuration needed to connect to the Wi-Fi network and communicate with the Flask server.

Create:

```text
esp32/include/LOGIN.h
```

The file should contain the following information:

* Wi-Fi SSID
* Wi-Fi password
* Flask server URLs

The configuration file should follow the following structure:

```c

#define WIFI_SSID       "your_wifi_ssid"
#define WIFI_PASSWORD   "your_wifi_password"

#define COMMAND_URL           "http://your-server-ip//ir/get_state"
#define CLEAR_URL           "http://your-server-ip//ir/clear_task"
#define UPLOAD_URL           "http://your-server-ip//ir/upload"

```

###

## How It Works

### 1. Web Application

The Flask server provides the web interface and API used to manage the virtual remotes.

Users can:

1. Create a remote.
2. Open the remote to view its buttons.
3. Create buttons for different IR functions.
4. Press a button to either transmit a stored IR command or put the system into learning mode.
5. Rename or delete remotes and buttons.

Remote and button information is stored using SQLite.

### 2. IR Learning

When a button does not have an IR command stored, pressing the button through the web interface places the ESP32 into receive mode.

The user can then point a physical remote at the IR receiver and press the desired button.

The ESP32 receives the IR signal and sends the command back to the Flask server, where it can be associated with the selected virtual button.

### 3. IR Transmission

When a virtual button already has an IR command stored, pressing the button sends the command information to the ESP32.

The ESP32 switches to transmit mode and sends the corresponding IR signal using the IR transmitter.

```text
Physical Remote
       │
       ▼
   IR Receiver
       │
       ▼
      ESP32
       │
       │ Wi-Fi / HTTP
       ▼
 Flask Web Server
       │
       ▼
    SQLite DB
       │
       ▼
 Virtual Remote
       │
       │ command
       ▼
      ESP32
       │
       ▼
  IR Transmitter
       │
       ▼
   Target Device
```

## Technologies

### Web

* Python
* Flask
* SQLite
* HTML
* CSS
* JavaScript
* Fetch API

### ESP32

* ESP32
* C
* PlatformIO
* Wi-Fi
* HTTP
* IR receiver
* IR transmitter
* FreeRTOS

## Requirements

### Web Application

* Python 3
* Flask

### ESP32

* ESP32 development board
* PlatformIO
* IR receiver
* IR transmitter
* Wi-Fi network

## Running the Web Server

Clone the repository:

```bash
git clone https://github.com/chandise12/universal-remote-web.git
cd universal-remote-web
```

Install the Python dependencies:

```bash
pip install flask
```

Start the Flask server:

```bash
python app.py
```

The web interface can then be accessed through the server's address.

## Running the ESP32

### 1. Create the private configuration file

Create:

```text
esp32/include/LOGIN.h
```

and add the required Wi-Fi and server configuration described in the **Configuration** section.

### 2. Open the ESP32 project

Open the `esp32/` directory as a PlatformIO project.

### 3. Build and upload

```bash
cd esp32
pio run
pio run --target upload
```

The ESP32 must be connected to the appropriate Wi-Fi network and the Flask server must be running and reachable at the configured address.

## Hardware

The ESP32 communicates with the IR hardware to both receive and transmit infrared signals.

The hardware consists of:

* ESP32 development board
* IR receiver
* IR transmitter
* Supporting resistors/components
* USB connection for programming and power

## Database

The web application uses SQLite to store:

* Virtual remotes
* Remote labels
* Buttons
* Button labels
* Learned IR messages

The database is created and initialized by the Flask application when required.

## API Overview

The Flask server provides endpoints for managing remotes and buttons.

| Method   | Endpoint                                           | Description                     |
| -------- | -------------------------------------------------- | ------------------------------- |
| `GET`    | `/remotes`                                         | Get the remote list             |
| `POST`   | `/remotes`                                         | Create a remote                 |
| `PUT`    | `/remotes/<remote_id>`                             | Rename a remote                 |
| `DELETE` | `/remotes/<remote_id>`                             | Delete a remote                 |
| `GET`    | `/remotes/<remote_id>/buttons`                     | Get buttons for a remote        |
| `POST`   | `/remotes/<remote_id>/buttons`                     | Create a button                 |
| `PUT`    | `/remotes/<remote_id>/buttons/<button_id>`         | Rename a button                 |
| `DELETE` | `/remotes/<remote_id>/buttons/<button_id>`         | Delete a button                 |
| `POST`   | `/remotes/<remote_id>/buttons/<button_id>/command` | Learn or transmit an IR command |

## Development

The web application and ESP32 firmware are maintained in the same repository but remain separated into their respective directories.

```text
web application → root directory
ESP32 firmware  → esp32/
```

The ESP32 project remains a standalone PlatformIO project inside the repository, while the Flask application handles the web interface, API, and database.
