# mmolch_qtutil_mount_watcher

A tiny Qt6 utility library that monitors mount/unmount events on Linux by watching `/proc/self/mountinfo`.
It provides a simple `MountWatcher` class and an optional `MountWatcherThread` wrapper for threaded usage.

---

## Features

- Lightweight, single‑purpose Qt6 static library
- Emits signals when filesystems are mounted or removed
- Thread‑safe access to the current mount list
- Optional worker thread wrapper (`MountWatcherThread`)
- Example application included

---

## Build

```bash
git clone https://github.com/your/repo.git
cd repo
cmake -B build
cmake --build build
```

The example application is built automatically when this project is the top‑level CMake project.

---

## Usage

### Basic example

```cpp
auto watcher = new mmolch::qtutil::MountWatcher{&app};

QObject::connect(watcher, &MountWatcher::mountAdded,
                 [](const QString &m){ qInfo() << "Added:" << m; });

QObject::connect(watcher, &MountWatcher::mountRemoved,
                 [](const QString &m){ qInfo() << "Removed:" << m; });

watcher->setEnabled(true);
```

### Threaded usage

```cpp
mmolch::qtutil::MountWatcherThread thread;

QObject::connect(thread.watcher(), &MountWatcher::mountAdded,
                 [](const QString& m){ qInfo() << "Added:" << m; });

thread.start();
```

---

## Project Structure

```
include/mmolch/qtutil_mount_watcher.h   # Public API
src/qtutil_mount_watcher.cpp            # Implementation
example/                                # Minimal usage example
```

---

## License

MIT
