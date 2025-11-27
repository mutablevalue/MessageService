# LAN Message Service (C++ / Winsock)

A lightweight LAN-based text chat written in C++ using Winsock.  
One instance runs as the **server**, others connect as **clients**.

---

## Requirements
- Windows 11 (64-bit)
- Visual Studio with **Desktop development for C++**
- Same Wi-Fi / LAN network
- ngrok tested for external networking, unstable
- You need to use the SLN for code organization (SORRY)

---

## Build
1. Clone or download the project
2. Open in Visual Studio
3. Build **x64 / Debug or Release**

---

## Colors (0x00RRGGBB)

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
|---------|-------------|
| `USERNAME <name>` | Set displayed name |
| `COLOR <hex>` | Set name color |
| `CONNECT <ip> <port>` | Join server |
| `CREATE` | Start server |
| `CLOSE` | Exit menu |
| `/quit` or `/exit` | Disconnect while chatting |

---

## Client Usage

```
USERNAME Kev
COLOR 0x0000FF00
CONNECT 192.168.1.42 8080
```

---

## Host Usage

```
USERNAME Host
COLOR 0x00FFD700
CREATE
```

Server example output:

```
Server listening on port 8080

Local connect:
  CONNECT 127.0.0.1 8080

LAN connect:
  CONNECT 192.168.1.42 8080
```

---

## Notes
- ngrok TCP tunneling tested
- unstable due to latency + tunnel resets
- recommended for LAN only
