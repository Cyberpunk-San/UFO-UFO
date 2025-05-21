# 🛸MISSION_MANGAL 🎮  



---

## 🚀 **Description**  
A **fast-paced 2D space shooter** built with **C++ and Raylib**, featuring:  
- 🌠 **Three challenging levels** (Asteroids → Aliens → Quantum Boss)  
- 🛡️ **Energy-based shield mechanics**  
- 🧠 **Logic gate puzzles** in the final boss fight  
- 🔊 **Retro sound effects**  

---
## **Demo video**
- (https://drive.google.com/file/d/1v79QTYQ6LJE1-BW4VDwpCIzqaPud9dup/view?usp=drive_link)

---

## 🔧 **Installation**  

### **Prerequisites**  
- **C++17** compiler (GCC/Clang/MSVC)  
- **Raylib 4.5+** ([Installation Guide](https://www.raylib.com/))  

### **Build & Run**  
```sh
# Clone the repo
git clone https://github.com/Cyberpunk-San/UFO-UFO.git
cd UFO-UFO
cd mission_mangal

# Compile 
g++ -g -Wall -Wextra main.cpp -o main.exe -IC:\raylib\src -LC:\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm 

# Run
./main.exe
```

---

## 🎯 **Gameplay**  

### **Controls**  
| **Key**                | **Action**                |
|------------------------|--------------------------|
| `W/A/S/D` or `↑/↓/←/→` | Move UFO               |
| `Right Mouse Click`    | Shoot (aims at cursor) |
| `Left Mouse Hold`      | Activate energy shield |

### **Levels**  
1. **🌌 Asteroid Field**  
   - Destroy asteroids to reach **500 points**.  
   - ❌ Collision = lose 1 life (unless shielded).  

2. **👽 Alien Invasion**  
   - Defeat alien ships (+20 pts each).  
   - 🚨 Aliens shoot homing bullets!  

3. **⚡ Quantum Core Boss**  
   - **Phase 1:** Solve 7 logic gates by shooting:  
     - 🔺 **Top half = 1 (TRUE)**  
     - 🔻 **Bottom half = 0 (FALSE)**  
   - **Phase 2:** Attack the boss during vulnerability windows!  

---

## 📂 **Project Structure**  
```
.
├── main.cpp             # Game entry point
├── resources/           # Assets folder
│   ├── textures..(.png) # PNG images (ufo, asteroids, etc.)
│   └── sounds..(.wav)   # WAV files (shooting, explosions)
├── Makefile             # Build script (optional)
└── README.md            # This file
```

---

## 🛠 **Technical Details**  
- **Language:** C++17  
- **Libraries:**  
  - **Raylib** (rendering/audio/input)  
  - **STL** (`vector`, `algorithm`)  
- **Design Patterns:**  
  - Object-Oriented Programming (OOP)  
  - Finite State Machine (level/game states)  

## 🙌 **Contributing**  
Pull requests welcome! For major changes, open an issue first.  

---
**Happy shooting!** 👾💥  

> Made with 🤍 by Cyberpunk-San
