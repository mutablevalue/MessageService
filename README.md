# LAN Message Service (C++ / Winsock)

A lightweight LAN-based text chat written in C++ using Winsock.  
One instance runs as the **server (host)** and others connect as **clients** via command line.

---

## Requirements
- Windows 11 (64-bit)
- Visual Studio with **Desktop development for C++**
- All participants must be on the **same LAN / Wi-Fi network**

---

## Build
1. Clone or download the project
2. Open in Visual Studio
3. Build **x64 / Debug** or **Release**

---

## Colors (Format: `0x00RRGGBB`)
| Value | Color |
|-------|--------|
| `0x00FF0000` | red |
| `0x0000FF00` | green |
| `0x000000FF` | blue |
| `0x00FFFFFF` | white |
| `0x00FFD700` | gold |
| `0x00FF69B4` | hot pink |
| `0x00FFA500` | orange |

---

## Commands
| Command | Description |
|---------|------------|
| `USERNAME <name>` | Set displayed name |
| `COLOR <hex>` | Set name color |
| `CONNECT <ip> <port>` | Join a server |
| `CREATE` | Start the server |
| `CLOSE` | Exit the menu |
| `/quit` or `/exit` | Disconnect while chatting |

---

## Host Usage (Start the Server)

Run the program and type:

```text
USERNAME Host
COLOR 0x00FFD700
CREATE
