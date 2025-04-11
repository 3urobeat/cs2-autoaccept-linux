# csgo-autoaccept-cpp
This C++ script automatically accepts CS:GO matches for you by analyzing your screen every 4 seconds and moving your mouse when it detects the 'Accept' button.  
Since this script does not interfere with any game files it should be completely VAC safe.  

This script is for Linux only. If you are on Windows see [my other repository for a Windows version.](https://github.com/3urobeat/csgo-autoaccept)  

![Screenshot](https://raw.githubusercontent.com/3urobeat/csgo-autoaccept-cpp/master/.github/img/showcase.png)   
  
&nbsp;

> Note: Does **not** support Wayland yet!  

&nbsp;

## Download
**Easy way:**  
Head over to the [release section](https://github.com/3urobeat/csgo-autoaccept/releases/latest) and download the latest executable.  
  
Open a terminal in the folder the executable is now located (your Downloads folder I guess?).  
Make sure the binary is executable: `chmod +x ./csgo-autoaccept-cpp`  
And run it: `./csgo-autoaccept-cpp`  

&nbsp;


**"Harder" way if you would like to compile it yourself:**  
Clone this repository using git or download it as a .zip.  
Make sure you have `libxtst-dev`, `cmake`, `make` and `g++` installed.  

Then open a terminal in the folder containing the files that you cloned from this repository.  
Go into the build folder: `cd build`  
Create makefile: `cmake ..`  
Then compile using make: `make -j 8`  
  
> Replace `-j 8` in the last command with how many CPU threads you would like to dedicate.

When completed you can move the `csgo-autoaccept-cpp` binary from the `build` folder to anywhere you like.  
  
&nbsp;

## Usage  
Open a terminal in the folder where you placed the `csgo-autoaccept-cpp` file.  
To start, just type `./csgo-autoaccept-cpp`. You can stop the program by pressing `CTRL+C` in the terminal.  

**Note: It is important that you run the executable from a terminal. By executing it with a double click it could run forever in the background and you'll only see it in your Task Manager/System Monitor or however it is called in your distribution.**

The script will start scanning your screen every 4 seconds.  
Open CS:GO and queue for a match. The script will automatically accept it for you.  
  
When you are in the loading screen you can close the terminal window.  
If not everyone accepted just leave the script open and it will continue scanning.  

> The game must be focused and visible. If you minimze the game the script won't work. If it doesn't recognize the button try changing your game brightness in the Video settings of CS:GO.  
