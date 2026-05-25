# AutoPower

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platforms: macOS | Windows | Linux](https://img.shields.io/badge/Platforms-macOS%20%7C%20Windows%20%7C%20Linux-lightgrey)](https://github.com/SantaJiang/AutoPower/releases)
[![Qt](https://img.shields.io/badge/Qt-5.15%2B%20%7C%206-green)](https://www.qt.io/)

**AutoPower** is a cross-platform scheduled power-operation utility. Set a specific time or countdown, and let it automatically shut down, reboot, hibernate, sleep, or turn off your display — all from a clean Qt‑based GUI. It runs quietly in the system tray and works natively on macOS, Windows, and Linux.

## ✨ Features

### ⚡ Five Power Operations

| Operation | Description |
|-----------|-------------|
| **Shutdown** | Turn off the computer completely |
| **Reboot** | Restart the system |
| **Hibernate** | Save current state to disk and power off |
| **Sleep** | Suspend to RAM for quick resume |
| **Close Monitor** | Turn off the display only |

### ⏱️ Two Timer Modes

- **Scheduled Time** – Pick an exact date and time from the built‑in calendar widget (with lunar calendar support)
- **Countdown** – Set a delay in hours and minutes from the current moment

### 🔧 Cross‑Platform, Native Implementation

All five operations are implemented natively on each platform — no external dependencies beyond Qt:

| Platform | Mechanism |
|----------|-----------|
| **Windows** | WinAPI (`ExitWindowsEx`, `SetSuspendState`, `SendMessage`) with `AdjustTokenPrivileges` for shutdown rights |
| **macOS** | `osascript` with administrator privileges (`shutdown`, `pmset sleepnow`, `pmset displaysleepnow`) |
| **Linux** | systemd‑logind D‑Bus (`PowerOff` / `Reboot` / `Suspend` / `Hibernate`) and `xset dpms force off` |

- Permission probing on Linux: `CanXxx` methods detect whether the current session allows each operation; unavailable options are hidden in the UI
- Background scheduling via `systemd-run --on-active=` on Linux and `osascript` on macOS

### 🖥️ System Tray

Minimise to the system tray and let AutoPower run silently in the background. The tray icon gives quick access to restore the window or exit.

### 🌍 Internationalization

Built‑in translation support via Qt Linguist; automatically follows the system locale.

## 🖥️ Supported Platforms

| Platform | Package Format | Minimum System |
|----------|----------------|----------------|
| **macOS** | `.dmg` | macOS 13.0+ |
| **Windows** | NSIS installer (`.exe`) + `.zip` | Windows 10/11 (64‑bit) |
| **Linux** | DEB, RPM, `.tar.gz` | systemd‑based distros |

## 🚀 Getting Started

### Prerequisites

- **Qt** 5.15+ or Qt 6 (Widgets module; DBus module on Linux)
- **CMake** 3.16+
- **C++17** compiler (MSVC / GCC / Clang)
- **Optional for packaging**: NSIS (Windows), `dpkg‑deb` & `rpmbuild` (Linux)

### Build from Source

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Compile
cmake --build build --config Release --parallel
```

### Package for Distribution

```bash
# Single command — CPack auto‑detects the host platform
cmake --build build --config Release --target package
```

Generated packages by platform:

| Platform | Packages |
|----------|----------|
| macOS | `.dmg` |
| Windows | `.exe` (NSIS) + `.zip` |
| Linux | `.deb` + `.rpm` + `.tar.gz` |

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `AUTOPOWER_APP_ID` | `com.santa.autopower` | Reverse-DNS application identifier |
| `AUTOPOWER_DEPLOY_QT` | `ON` | Automatically deploy Qt runtime libraries at install/package time |

### Direct Install on Linux

```bash
# Install to /usr/local
PREFIX=/usr/local BUILD_DIR=build deploy/linux/install.sh

# Uninstall
PREFIX=/usr/local deploy/linux/uninstall.sh
```

## 📸 Screenshots

![Screenshot 1](https://github.com/SantaJiang/AutoPower/blob/main/screenshot1.png)

![Screenshot 2](https://github.com/SantaJiang/AutoPower/blob/main/screenshot2.png)

## 🏗️ Project Structure

```
AutoPower/
├── CMakeLists.txt                # Root CMake + CPack configuration
├── LICENSE                       # MIT License
├── README.md                     # This file
├── main.cpp                      # Entry point (High‑DPI, translation)
├── mainwindow.h / .cpp / .ui     # Main window — UI + logic
├── powerop.h / .cpp              # Power operation core (#ifdef per platform)
├── worker.h / .cpp               # Timer thread (QThread, 1‑second polling)
├── trayicon.h / .cpp             # System tray icon
├── src.qrc                       # Qt resource file
├── schedule/                     # Calendar / scheduling widgets
│   ├── customdateedit.h / .cpp   # Custom date‑edit control
│   ├── lunarcalendarinfo.h / .cpp# Lunar calendar data
│   ├── minicalendaritem.h / .cpp # Mini‑calendar item
│   └── minicalendarwidget.h / .cpp # Mini‑calendar widget
└── deploy/                       # Platform‑specific deployment assets
    ├── mac/Info.plist.in         # macOS Bundle plist
    ├── win/app.rc.in             # Windows resource template
    └── linux/                    # Desktop entry, AppStream metadata, icon, install/uninstall scripts
```

## 📥 Downloads

Pre‑built binaries are available on the [Releases](https://github.com/SantaJiang/AutoPower/releases) page.

## 📄 License

AutoPower is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Pull Requests are welcome.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing‑feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing‑feature`)
5. Open a Pull Request

---

*AutoPower — set it, forget it, and let your computer turn itself off.*