# CS241 Final Project - Metro Traffic Analyzer

## Status
|[macOS][macos-link]|[iOS][ios-link]| [Ubuntu][ubuntu-link]| [Windows][win-link]|
|-----------------|----------------|---------------|---------------|
| ![macos-badge]   |![ios-badge]   | ![ubuntu-badge]      | ![win-badge]  |


[win-link]: https://github.com/BugenZhao/MTAnalyzer/actions?query=workflow%3AWindows "WindowsAction"
[win-badge]: https://github.com/BugenZhao/MTAnalyzer/workflows/Windows/badge.svg  "Windows"

[ubuntu-link]: https://github.com/BugenZhao/MTAnalyzer/actions?query=workflow%3AUbuntu "UbuntuAction"
[ubuntu-badge]: https://github.com/BugenZhao/MTAnalyzer/workflows/Ubuntu/badge.svg "Ubuntu"

[macos-link]: https://github.com/BugenZhao/MTAnalyzer/actions?query=workflow%3AmacOS "macOSAction"
[macos-badge]: https://github.com/BugenZhao/MTAnalyzer/workflows/macOS/badge.svg "macOS"

[ios-link]: https://github.com/BugenZhao/MTAnalyzer/actions?query=workflow%3AiOS "iOSAction"
[ios-badge]: https://github.com/BugenZhao/MTAnalyzer/workflows/iOS/badge.svg "iOS"


## Introduction

<img src="./imgs/plots.png" alt="plots" style="zoom: 33%;" />As one of the methods for citizens to travel, metro plays an important role in urban transportation. In order to give full play to public resources, relieve urban traffic pressure and improve citizens' happiness, it is necessary to analyze metro data in a multidimensional way. **Metro Traffic Analyzer** is an all-in-one simple GUI data analysis tool implemented in Qt. Based on the above requirements, **Metro Traffic Analyzer** allows users to filter and import traffic data sets, visually analyze data trends, search travel paths and predict times, and retrieve the entire database in any forms.

## Build and Run
### Build
First of all, make sure you have installed Qt on the computer and the version is higher than `5.13`.
#### -  Using `cmake`:
- Modify `set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /path/to/your/qt/5.x.x/xxx_64/lib/cmake)` in `CMakeLists.txt` to 
your own Qt `cmake` path.
- In terminal, run `cmake .` and `make`.

#### - Using `qmake`:
- In terminal, run `qmake` and `make`.

#### - Using Qt Creator:
- Open `MTAnalyzer.pro`.
- Configure the project.
- Build and run.


### Run
- Execute `./MTAnalyzer` on Unix or directly run `MTAnalyzer.exe` on Windows.


## Results

### 1. Filtered Import and Analysis

In the **Importers & Filters** on the left, **Metro Traffic Analyzer** categorizes the raw data sets by date, allowing the user to select the raw data to import, file by file. The user can choose whether to import optional fields such as Device ID and User ID. Also, users can use Filters for lines and payment types to analyze only the data they are interested in.

**Importers & Filters** is **reusable**, users can add or remove any data at any time without restarting the application. Also, the widget can be moved, closed and reopened whenever you want.

<img src="./imgs/filter.png" alt="filter" style="zoom: 15%;" />

### 2. High-performance, Pretty and Multi-tabbed Plots


Through **Add Tab...** actions via **File** menu or keyboard shortcuts **⌘1~4** , there are **4 different types of plots** available to the user. The user can open **unlimited plots** or even perform analysis at the same time, toggle different tabs freely at the tab bar in the bottom of the user interface.

When the animations are on, the analysis results will be dynamically appended to the plots over time, which brings the plot a **nice look and feel**. With animations off, most plots can be analyzed in **less than 0.5 seconds**, even though 16 million records have been imported.

<img src="./imgs/timeline.png" alt="timeline" style="zoom: 33%;" />

Introducing the **"Flow of Stations" plot**. The plot provides the user with a **timeline** that allows the user to slide the timeline to analyze inflows and outflows of all stations at different times.

### 3. Route Planning and Time Estimation

![estimation](./imgs/estimation.png)

In the Path section, **Metro Traffic Analyzer** can plan an optimal travel route, quickly **estimate the times** based on imported data, and list hundreds of travel time records for users' reference.

### 4. AnyExplore

<img src="./imgs/anyexplore.png" alt="anyexplore" style="zoom: 25%;" />

**AnyExplore** supports any standard SQL statements to explore the entire database. It provides the user with **unlimited possibilities** to inspect the data sets in all ways. **Metro Traffic Analyzer** also presets several examples to help the user get started with **AnyExplore**.

### 5. Preferences

<img src="./imgs/preferences.png" alt="preferences" style="zoom: 33%;" />

The Preferences panel allows users to customize subdirectories where the adjacency tables and data sets are placed. It also provides a slider for users to adjust the analysis speed (animations and sleep time).

## Implementation Details

1. **Metro Traffic Analyzer** is based on an in-memory SQLite database. It provides multi-dimensional data analysis and high performance at the same time. After continuous logic optimization and application of multi-core optimization techniques, importing operations perform almost as well as a simple time-based hash table, while **ALL OF THE ANALYSIS gain much better performance than the latter**. (See also: **Discussions => Performance**)

2. Most of the widgets are implemented in separate modules, which is **loosely coupled** with the main window, resulting in clearer code logic, strong reusability and performance improvement.

3. Make full use of multi-threading and Qt signal-slot technique to **avoid time-consuming operation** in the GUI thread and ensure the program GUI's fluency and stability.

4. Easy operation logic and attention to details, such as: 
  
  - Provides user manual to guide users to get started easily.

	- Adopts Layout comprehensively, scaling the window arbitrarily is allowed.
	- Toggles the state of buttons and actions at proper time, bringing to strong robustness. 
	- Indicates current information and operation status through the status bar and title bar.
	- Keyboard shortcuts are supported, which is in line with the user's usual operation logic. 

## Discussions

### Performance

| Task                                          | Average Time      |
| --------------------------------------------- | ----------------- |
| Import All Data Sets                          | 78.254 s          |
| Import All Data Sets with All Optional Fields | 84.265 s          |
| Import Data of One Day                        | 10.817 s          |
| Plot 1 - Inflow and Outflow                   | 0.291 s           |
| Plot 2 - Total Flow                           | 0.534 s           |
| Plot 3 - Inflow with Lines                    | 0.491 s           |
| Plot 4 - Station Flow                         | 0.163 s - 0.660 s |
| Time Estimation (235 records)                 | 1.166 s           |
| Select 2.25 Million Records in **AnyExplore** | 2.120 s           |

According to the results, **Metro Traffic Analyzer** succeeds to achieve a balance between import speed and analysis performance, providing extremely fast analysis speed. 

With all data imported, **Metro Traffic Analyzer** will takes up about 1.5GB of memory. All of the results above were tested on MacBook Pro (2.3 GHz Eight-Core Intel Core i9 CPU and 32 GB 2667 MHz DDR4 Memory), with "Fastest Analysis Speed" set in Preferences panel.

See also: **Implementation Details (1)**

### Analysis Results

1. The metro flow is highly correlated with the date. During weekdays, traffic peaks at 8:30 AM and 5:30 PM. There are small peaks at 8:00, 9:00 and 10:00 PM, especially on line B, which may be caused by overtime work. On weekends, there is no significant peak in the flow curve, which maximizes at 5:30 PM.

2. Among the three lines, line B is the busiest, followed by line C, while line A has less traffic. On weekdays, there is little difference between traffic flow of line B and C, but there is twice difference on weekends. Thus, line B may contain more shopping malls, tourist attractions and other service locations.

3. The busiest stations are Station 15, 9, 4, 7 and so on. In particular, Station 15 has the highest inflow and outflow most of the time. Considering that there are only three lines in the Hangzhou metro system, it can be inferred that the Station 15 is a railway station or a transfer station, while the other busy stations are downtown or around the West Lake. Most likely they are all on line B.

## Acknowledgment

- Thanks to Dr. Ling and Dr. Jin for their careful teaching.

- Thanks to TAs for their kind guidance.
- Thanks to my classmate Yifei Wei, Yimin Zhao and others for their ideas and inspiration.


## Copyright
BugenZhao, SJTU, Dec 2019.
