# Parts List

LOLIN D32 Pro ESP32 Dev Board **https://www.aliexpress.com/item/32883116057.html**

Graphical OLED Display (128x64)  **https://www.pololu.com/product/3760**

SD Card to fit your animations


# Libraries Used

The EPS32 PNG Lib is included in the project fodler & does not need to be installed as a library

ESP32 PNG Library - https://github.com/lagunax/ESP32-upng - modified to work with SD card instead of SPIFFS

The POLOLU OLED Lib needs to be installed in your Arduino Libraries folder https://github.com/pololu/pololu-oled-arduino

# Animation Pre requisites

Your animations must be in .png 24BIT colour, no transparency or anythhing else fancy. 
The animations must be 128*64px 
The file names must look like this "AnimationName_00000.png" . Each animation can have up to a maximum of 99,999 frames.
The 1st frame must start at "_00001"
(You can also set a frame delay in the playlist file below if you want to slow down or speed up the playback)

# SDCARD Pre requisites

Format the SDCARD as one FAT32 Volume. Size isnt important. The root of the SDCard needs to have the following structure:

````
[DIR] AnimationOne
[DIR] AnimationTwo
[DIR] AnimationThree
[FILE] playlist
````

Example layout of one of the animation folders:

````
/AnimationOne
AnimationOne_00001.png
AnimationOne_00002.png
AnimationOne_00003.png
AnimationOne_00004.png
AnimationOne_..............
AnimationOne_00100.png
````

The Playlist file is set up as follows:
````
[Case Senstive Animation Name],[Number of Frame Files],[Delay in MILISECONDS between each frame];
[Case Senstive Animation Name],[Number of Frame Files],[Delay in MILISECONDS between each frame];
[Case Senstive Animation Name],[Number of Frame Files],[Delay in MILISECONDS between each frame];
....
````

Example layout of the "playlist" file

````
AnimationOne,100,10;AnimationTwo,200,20;AnimationThree,50,100;
````

# Pinout for the Wemos D32 Pro module

<img src="PinOut.PNG" width="600" />


# Boot Up Flow

1.  Insert the SD CARD and power on
2.  The Pre prosessor will RUN over the playlist and compile a .STREAM file for each animation. You will see a new folder for each animation called "AnimationNameSTREAM" and inside it will be teh stream file named "AnimationName.STREAM"
3.  After all animation folders are processed the ESP32 will start playing the streams in the order listed in the file 'playlist'. You can put multiple entries for the same animation. Each animation has 2 options. Number of frames in the animation folder and the delay in MILISECONDS between each frame. 1ms delay is good for very smooth frame rates.

