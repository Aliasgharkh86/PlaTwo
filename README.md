# PlaTwo - Multiplayer Game Hub 🎮

[![Repo](https://img.shields.io/badge/GitHub-Repository-blue?logo=github)](https://github.com/Aliasgharkh86/PlaTwo)

PlaTwo is a comprehensive two-player game hub built with **C++** and the **Qt Framework**. It allows users to compete in classic board games over a network using a robust Server-Client architecture.

## 🕹️ Featured Games
1. **Dots and Boxes:** Classic grid game with custom board sizes (6x6 to 8x8) and 9 unique player colors. Securing a box grants an extra turn, and the winner is determined by the most boxes captured.
2. **Nine Men's Morris (Mills):** Strategic board game featuring full logic for placing, moving, flying, and forming "Mills" to capture opponent pieces.
3. **Fanorona:** A traditional strategy game with complex capture mechanics including Approach, Withdrawal, and sequential Chain Captures.

## ✨ Key Features
* **User Authentication:** Secure sign-up/log-in with hashed passwords, phone/email validation, and profile management.
* **Server-Client Multiplayer:** Play across different machines using IP and Port connections via Qt Sockets.
* **Custom Game Rooms:** Host rooms with customizable settings (time limits, board sizes, ports) or join existing rooms as a guest.
* **Match History & Scoring:** Persistent records of past games, opponent details, play dates, and total scores for each user.
* **Robust Game Logic:** Auto-validation of legal moves, UI move highlighting, timer management (timeout = loss), and automatic win/loss/draw detection.
* **State Persistence:** Ability to pause, save, and resume matches at a later time. Reconnection handling for dropped players.

## 🛠️ Technologies & Concepts Used
* **Language:** C++
* **Framework:** Qt Creator (GUI, Network/Sockets, Multithreading)
* **Concepts:** Object-Oriented Programming (OOP), STL, Polymorphism, Clean Code, Clean Architecture.

## 🚀 How to Run
1. Clone the repository to your local machine:
   ```bash
   git clone https://github.com/Aliasgharkh86/PlaTwo.git
   ```
2. Open the project in **Qt Creator**.
3. Build and compile the project (ensure Qt Network modules are enabled in your project file).
4. Run the Server application to host the game logic.
5. Run the Client application(s), create an account, and connect to the Server IP.

## 👥 Contributors
This project was developed collaboratively as the Final Project for Advanced Programming.
* **[Aliasgharkh86](https://github.com/Aliasgharkh86)**
* **[amrhatamiii-rgb](https://github.com/amrhatamiii-rgb)**

> **Note for Grading:** `Amirmehdig` and `ro-Nec` will be added as collaborators to the repository as per project requirements.

# PlaTwo
A two-player Qt/C++ game hub with Dots and Boxes, Nine Men's Morris, and Fanorona over network sockets.
