# Shared Painter

<img src="https://raw.github.com/gunoodaddy/SharedPainter/master/SharedPainterIntro.png"/>

## Features

* Automatically server finding by udp broadcast (if same network)
* TCP directly connection to host (host with NAT not available yet..)
* Multiuser sharing
* Free Pen Object
* Text Object
* Background Image (Screen shot)
* File Object (limit 20MB per a file)
* Image File Object (limit 20MB per a file)
* Image File Object Scale
* Window Resize
* All Objects Position
* Object Remove
* Export/Import file
* Undo/Redo
* Playback 
* System tray

## Requirement
* Now, you can build with Qt Creator (tested in Windows only..)

### Common Requirements
* Qt 4.8.2+ (http://qt-project.org/downloads) <br>
  *You should install the Qt SDK for reducing your stamina waste..*
* boost 1.51 with asio (if Windows, just download this : http://www.boostpro.com/download/)

### Visual Studio 2008 Build Requirements
* Visual Studio 2008 (not express version)
* Qt Visual Studio Add-in (http://releases.qt-project.org/vsaddin/qt-vs-addin-1.1.11-opensource.exe)

### QT Creator Build Requirements
* QT Creator (if you installed QT SDK, QT Creator is automatically installed..)
* debugging tool(for QT Creator on windows) : http://msdn.microsoft.com/ko-kr/windows/hardware/hh852363


### Add System Environment <br>
 `QT_DIR` : qt home directory. <br>
 `BOOST_DIR` : boost home directory <br>


