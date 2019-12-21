# CS241 Final Project - Metro Traffic Analyzer

## Status
|[macOS][macos-link]|[iOS][ios-link]| [Ubuntu][ubuntu-link]| [Windows][win-link]|
|-----------------|----------------|---------------|---------------|
| ![macos-badge]   |![ios-badge]   | ![ubuntu-badge]      | ![win-badge]  |


[win-link]: https://github.com/BugenZhao/FinalProject/actions?query=workflow%3AWindows "WindowsAction"
[win-badge]: https://github.com/BugenZhao/FinalProject/workflows/Windows/badge.svg  "Windows"

[ubuntu-link]: https://github.com/BugenZhao/FinalProject/actions?query=workflow%3AUbuntu "UbuntuAction"
[ubuntu-badge]: https://github.com/BugenZhao/FinalProject/workflows/Ubuntu/badge.svg "Ubuntu"

[macos-link]: https://github.com/BugenZhao/FinalProject/actions?query=workflow%3AmacOS "macOSAction"
[macos-badge]: https://github.com/BugenZhao/FinalProject/workflows/macOS/badge.svg "macOS"

[ios-link]: https://github.com/BugenZhao/FinalProject/actions?query=workflow%3AiOS "iOSAction"
[ios-badge]: https://github.com/BugenZhao/FinalProject/workflows/iOS/badge.svg "iOS"


## Introductions
Please see [REPORT.md][report-md-link].

## Build and Run
### Build
First of all, make sure you have installed Qt on the computer and the version is higher than `5.13`.
#### -  Using `cmake`:
- Modify `set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /path/to/your/qt/5.x.x/xxx_64/lib/cmake)` in `CMakeLists.txt` to 
your own Qt `cmake` path.
- In terminal, run `cmake .` and `make`.

#### - Using `qmake`:
- In terminal, run `qmake` and `make`.

### Run
- Execute `./FinalProject` on Unix or directly run `FinalProject.exe` on Windows.

## Copyright
BugenZhao, SJTU, Dec 2019.


[report-md-link]: REPORT.md "REPORT.md"
