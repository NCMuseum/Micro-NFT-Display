# Parts List

Solderless Breadboard similar to this **https://core-electronics.com.au/solderless-breadboard-830-tie-point-zy-102.html**

LOLIN D32 Pro ESP32 Dev Board **https://www.aliexpress.com/item/32883116057.html**

Graphical OLED Display (128x64)  **https://www.pololu.com/product/3760**

SD Card to fit your animations

1 x 18650 Lithium Ion Cell (2600mah)

1 x power switch

Bunch of cabling


# Libraries Used

The EPS32 PNG Lib is included in the project folder & does not need to be installed as a library **https://github.com/lagunax/ESP32-upng** - modified to work with SD card instead of SPIFFS on the ESP32

The POLOLU OLED Lib needs to be installed in your Arduino Libraries folder https://github.com/pololu/pololu-oled-arduino

# Creating an animation file set from any video file

The ESP32 is not powerfull enough to play actual video files like .mp4, but we can preprocess any video file as sequenced PNG images. To do this you will need VLC Player **https://www.videolan.org/** After its installed you will need to configure on the Video Filters to output PNG files for you. Follow these steps to set it up:

1.  Open VLC Player
2.  Click on the TOOLS menu and select PREFERENCES
3.  Tick the "ALL" radio button in SHOW SETTINGS so you can see all teh settings at once
4.  Ont the left pane, scroll down to the bottom and DOUBLE CLICK on VIDEO
5.  On the right PANE tick "SCENE VIDEO FILTER"
6.  On the left pane scroll down to and click on "SCENE FILTER"
7.  On the right pane set the Image Format to "png"
8.  Image width to 128
9.  Image height to 64
10.  Filename prefix to whatever you want your animation to be called eg.. "MyAnimation_"
11.  If you want the PNG files output to specific folder enter it in the "Directory Path Prefix" field
12.  Set the recording Ration to 1
13.  Hit Save and restart VLC Player


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

