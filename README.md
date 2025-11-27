===========================================================
===            LAN MESSAGE SERVICE (C++ / Winsock)       ===
===========================================================

A lightweight LAN-based text chat written in C++ using Winsock.
One instance runs as the SERVER, others connect as CLIENTS.

*** REQUIREMENTS ***
/// Windows 11 (64-bit)
/// Visual Studio with Desktop development for C++
/// Same Wi-Fi / LAN network for everyone
/// We attempted using ngrok for external networking, but it was unstable.

-----------------------------------------------------------

*** BUILD ***
1) Clone or download the project
2) Open in Visual Studio
3) Build x64 (Debug or Release)

-----------------------------------------------------------

*** COLORS (0x00RRGGBB) ***
0x00FF0000   red
0x0000FF00   green
0x000000FF   blue
0x00FFFFFF   white
0x00FFD700   gold
0x00FF69B4   hot pink
0x00FFA500   orange

-----------------------------------------------------------

*** COMMANDS ***

+----------------------+-------------------------------+
| USERNAME <name>      | set displayed name            |
| COLOR <hex>          | set name color                |
| CONNECT <ip> <port>  | join server                   |
| CREATE               | start server                  |
| CLOSE                | exit menu                     |
| /quit or /exit       | disconnect while chatting     |
+----------------------+-------------------------------+

-----------------------------------------------------------

*** CLIENT USAGE (LAN EXAMPLE) ***

USERNAME Kev
COLOR 0x0000FF00
CONNECT 192.168.1.42 8080

Enter messages normally
Use /quit or /exit to leave

-----------------------------------------------------------

*** HOST USAGE ***

USERNAME Host
COLOR 0x00FFD700
CREATE

Example output:

Server listening on port 8080

Local connect:
    CONNECT 127.0.0.1 8080

LAN connect:
    CONNECT 192.168.1.42 8080

-----------------------------------------------------------

*** NOTES ***
/// ngrok TCP tunneling was tested for outside users
/// It worked inconsistently due to latency and unstable tunnels
/// Recommended for LAN-only usage right now

===========================================================
