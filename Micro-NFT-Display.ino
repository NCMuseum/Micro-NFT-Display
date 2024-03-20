/*
Micro CD Card PinOut for the Wemos D32 Pro as taken from 
https://www.wemos.cc/en/latest/_static/files/sch_d32_pro_v2.0.0.pdf

NS		1	NC
/CS		2	IO    04	TF_CS
DI		3	IO    23	MOSI
VDD		4	3.3V
CLK		5	IO    18	SCK
VSS		6	GND
D0		7	IO    19	MISO
RSV		8	NC
CD(1)	G	GND

PINOUT for OLED Screen
VCC     +5V
GND     GND
CLK     22
MOS     26
RES     0
DC      21
CS      27
 */

#include <WiFi.h>
#include "SPI.h"
#include <PololuOLED.h>
#include "FS.h"
#include "SD.h"
#include <string>
#include "upng.h"
#include "animationQue.h"

//Bitmaping Data
const byte rows=64, cols=128;    //rows = HEIGHT(Y)  cols = WIDTH(X)

//Colour Data
byte tempColour[3] = {0,0,0};

//Counters
unsigned short int cIndex=0, secondaryCIndex=0;
unsigned short int yCnt=0, xCnt=0;

//File Paths
char playListFile[] = "/playlist";
char integerCharString[] = "00000";

//PNG stuff
upng_t* upng;
unsigned short int xCount, yCount, imageSize, bufferIndex=0, testCnt=0;

//Card Stuff
uint8_t cardType=0;
long cardSize=0;

//Display
//The pins are specified in this order: CLK, MOS, RES, DC, CS.
PololuSH1106 display(22, 26, 0, 21, 27);
uint8_t graphics[1024];

//Pre Processor
File outputStream;
animationQue globalAnimationQue;

void setup()
{
  //Turn off WIFI we do not need it here
  WiFi.mode(WIFI_OFF); 
  
  //Init Serial
  Serial.begin(115200);
  Serial.printf("\r\n\r\n\r\nSystem Booting....\r\n");

  //Set up the OLED Display
  display.setLayout11x4WithGraphics(graphics);
  memset(graphics, 0, sizeof(graphics));
  display.noAutoDisplay();
  display.clear();
  display.display();

  //Init File System
  initFS();
  
  //Populate Animation List from SD Card file /playlist
  compilePlaylist();
  
  //Pre Process Multiple Frames into 1 File Stream
  compileStream();

}

void initFS()
{
  Serial.printf("\r\n\tSetting up SD Card FS...");
  
  SPI.begin();
  //begin(18, 19, 23, 4)
  SPI.setFrequency(40000000);
  if(! SD.begin(SS, SPI, 40000000, "/sd", 5, false) )
  {
    Serial.printf("\r\n\tSD Card Mount Failed...");
    while(true)
    {
      yield();
    }
  }
  cardType = SD.cardType();
  if(cardType == CARD_NONE)
  {
    Serial.printf("\r\n\tNo SD card attached!");
    return;
  }
}

void compilePlaylist()
{
  char* tempAnimationName;
  char* tempAnimationLength;
  char* tempAnimationDelay;
  char* playListFileBuffer;
  unsigned short int animationCount=0;
  unsigned short int aCnt=0, startI=0, endI=0, needleIndex=0, tempLength=0;
  PLNODE* queLocater;
  
  outputStream = SD.open(playListFile, "r");
  playListFileBuffer = new char[ outputStream.size() ];
  outputStream.readBytes(playListFileBuffer, outputStream.size());
  outputStream.close();
  Serial.printf("\r\nReadin\t[%s]", playListFileBuffer);

  //Grab number of animations in playlist
  animationCount = countNeedles(playListFileBuffer, ';');
  Serial.printf("\r\nAnimation Count in playlist file\t[%d]", animationCount);
  
  for(aCnt=0; aCnt<animationCount; aCnt++)
  {
    //Serial.printf("\r\nADD..");
    //locate 1st ','  animationName
    needleIndex++;
    endI = findNeedleCount(playListFileBuffer, ',', needleIndex);
    tempLength = endI-startI;
    tempAnimationName = new char[ tempLength+1 ];
    clearCharArray(tempAnimationName);
    memcpy(tempAnimationName, playListFileBuffer+startI, tempLength);
    tempAnimationName[tempLength]='\0';
    //locate 2nd ','  frameCount
    startI = endI+1;
    needleIndex++;
    endI = findNeedleCount(playListFileBuffer, ',', needleIndex);
    tempLength = endI-startI;
    tempAnimationLength = new char[ tempLength+1 ];
    clearCharArray(tempAnimationLength);
    memcpy(tempAnimationLength, playListFileBuffer+startI, tempLength);
    tempAnimationLength[tempLength]='\0';
    startI = endI+1;
    //locate 3rd ';'  delay
    endI = findNeedleCount(playListFileBuffer, ';', aCnt+1);
    tempLength = endI-startI;
    tempAnimationDelay = new char[ tempLength+1 ];
    clearCharArray(tempAnimationDelay);
    memcpy(tempAnimationDelay, playListFileBuffer+startI, tempLength);
    tempAnimationDelay[tempLength]='\0';
    startI = endI+1;
    globalAnimationQue.add(tempAnimationName, atoi(tempAnimationLength), atoi(tempAnimationDelay));
    yield();
    Serial.printf("\r\nItem\t[%d]\t[%s][%d][%d]", aCnt, tempAnimationName, atoi(tempAnimationLength), atoi(tempAnimationDelay) );
    //clear memory
    delete tempAnimationName;
    delete tempAnimationLength;
    delete tempAnimationDelay;
  }
  
  //Dump Animations
  Serial.printf("\r\n\tTotal Nodes\t%d", globalAnimationQue.totalNodes);
  for(aCnt=0; aCnt<globalAnimationQue.totalNodes; aCnt++)
  {
    queLocater = globalAnimationQue.findByID(aCnt);
    if(queLocater!=NULL)
    {
      Serial.printf("\r\nAnimation\t[%d][%d]\tName[", aCnt, queLocater->_nodeID );
      Serial.print(queLocater->_animationName);
      Serial.printf("]\tFolder Path[");
      Serial.print(queLocater->_folderPath);
      Serial.printf("]\tFull Path[");
      Serial.print(queLocater->_fullPath);
      Serial.printf("]\tFrame Count[");
      Serial.print(queLocater->_totalFrames);
      Serial.printf("]\tFrame Delay[");
      Serial.print(queLocater->_frameDelay);
      Serial.printf("]");
    }
    else
    {
      Serial.printf("\r\n\tNode [%d] not found!", aCnt);
    }
  }

}

unsigned short int countFiles(char* dirName)
{
    unsigned short fileCount=0;
    char* folderPath = new char[strlen(dirName)+2];
    
    folderPath[0] = '/';
    memcpy(folderPath+1, dirName, strlen(dirName));
    folderPath[strlen(dirName)+1] = 0;
    
    
    File root = SD.open(folderPath);
    File file = root.openNextFile();
    while(file)
    {
        fileCount++;
        file = root.openNextFile();
    }
    return fileCount;
}

void clearCharArray(char* charArray)
{
  unsigned short int charCount=0;
  for(charCount=0; charCount<strlen(charArray); charCount++)
  {
    charArray[charCount]=0;
  }
}
short int findNeedleCount(char* haystack, char needle, unsigned short needleCount)
{
  unsigned short int hayCount=0, nCount=0;
  for(hayCount; hayCount<strlen(haystack); hayCount++)
  {
    if(haystack[hayCount]==needle)
    {
      nCount++;
      if(nCount==needleCount)
      {
        return hayCount;
      }
    }
  }
  return -1;
}
short int countNeedles(char* haystack, char needle)
{
  unsigned short int found;
  unsigned short int hayCount;
  
  for(hayCount=0; hayCount<strlen(haystack); hayCount++)
  {
    if(haystack[hayCount]==needle)
    {
      found++;
    }
  }
  return found;
}

void compileStream()
{
  unsigned short int plCounter=0, postFrameCounter=0, charArrayIndex=0, bytesWriten=0;
  byte dirExists=0, streamExists=0;
  char * tempPathArray;
  char endOfFileChars[] = "_00000.png";
  unsigned short int barWidth=98;
  double progressBar = 0;
  PLNODE* queLocater;
  
  for(plCounter=0; plCounter<globalAnimationQue.totalNodes; plCounter++)
  {
    queLocater = globalAnimationQue.findByID(plCounter);
    if(queLocater!=NULL)
    {
      //Details of animationa re in 
      //  queLocater->_animationName
      //  queLocater->_fullPath     - full path of stream file to output to
      charArrayIndex=0;
      tempPathArray = new char[strlen(queLocater->_animationName)+2+strlen(queLocater->_animationName)+strlen(endOfFileChars)+1];
      clearCharArray(tempPathArray);
      tempPathArray[charArrayIndex] = '/';
      // [/]
      charArrayIndex++;
      memcpy(tempPathArray+charArrayIndex, queLocater->_animationName, strlen(queLocater->_animationName));
      // [/name]
      charArrayIndex+=strlen(queLocater->_animationName);
      // [/name/]
      tempPathArray[charArrayIndex] = '/';
      charArrayIndex++;
      memcpy(tempPathArray+charArrayIndex, queLocater->_animationName, strlen(queLocater->_animationName));
      charArrayIndex+=strlen(queLocater->_animationName);
      // [/name/name]
      memcpy(tempPathArray+charArrayIndex, endOfFileChars, strlen(endOfFileChars));
      // [/name/name_00000.png]
      charArrayIndex+=strlen(endOfFileChars);
      tempPathArray[charArrayIndex] = '\0';
      
      Serial.printf("\r\nProceessing frames in[");
      Serial.print(tempPathArray);
      Serial.printf("]");
      //check if output stream file exists
      if(!SD.exists(queLocater->_fullPath))
      {
          SD.mkdir(queLocater->_folderPath);
          outputStream = SD.open(queLocater->_fullPath, FILE_WRITE);
          if(!outputStream)
          {
            Serial.printf("\r\n\tERROR - Can not open stream file for writing....");
            outputStream.close();
          }
          else
          {
            //Reset Progress Bar Width
            clearCanvas();
            display.clear();
            display.print("Compiling:");
            drawRectangle(16, 20, barWidth, 40);
            display.display();
            progressBar=0;
            
            //build the stream file
            for(postFrameCounter=0; postFrameCounter<queLocater->_totalFrames; postFrameCounter++)
            {
              fileNameIncremeneter(postFrameCounter+1, tempPathArray);
              upng = upng_new_from_file(SD, tempPathArray);
              if (upng != NULL)
              {
                upng_decode(upng);
                Serial.printf("\r\nFrame\t%d\t%d\tBytes\t", postFrameCounter, upng_get_size(upng));
                bytesWriten = outputStream.write(upng_get_buffer(upng), upng_get_size(upng));
                upng_free(upng);
                Serial.printf("Written\t%d bytes", bytesWriten);
                progressBar = ((double)postFrameCounter/(double)queLocater->_totalFrames)*((double)barWidth);
                drawSolidRectangle(16, 20, (uint8_t)round(progressBar), 40);
                display.display();
              }
            }
            outputStream.close();
          }
      }
      else  
      {
        Serial.printf("\r\n\tStream already exists for this item.");
      }
      delete tempPathArray; 
    }
  }
}

void fileNameIncremeneter(unsigned short int fileNameIndex, char* filePathString)
{
  char renameAtPointer=0, renameCounter=0;
  char namePlacer = 95; //95="_"

  //find the "_" in the file name fileNamePointer
  for(renameCounter=0; renameCounter<strlen(filePathString); renameCounter++)
  {
    if(filePathString[renameCounter]==namePlacer)
    {
      renameAtPointer = renameCounter+1;
      break;
    }
  }
  itoa(fileNameIndex, integerCharString, 10);  
  if(fileNameIndex<10)
  {
    filePathString[renameAtPointer] = 48;
    filePathString[renameAtPointer+1] = 48;
    filePathString[renameAtPointer+2] = 48;
    filePathString[renameAtPointer+3] = 48;
    filePathString[renameAtPointer+4] = integerCharString[0];
  }
  else if(fileNameIndex<100)
  {
    filePathString[renameAtPointer] = 48;
    filePathString[renameAtPointer+1] = 48;
    filePathString[renameAtPointer+2] = 48;
    filePathString[renameAtPointer+3] = integerCharString[0];
    filePathString[renameAtPointer+4] = integerCharString[1];
  }
  else if(fileNameIndex<1000)
  {
    filePathString[renameAtPointer] = 48;
    filePathString[renameAtPointer+1] = 48;
    filePathString[renameAtPointer+2] = integerCharString[0];
    filePathString[renameAtPointer+3] = integerCharString[1];
    filePathString[renameAtPointer+4] = integerCharString[2];
  }
  else if(fileNameIndex<10000)
  {
    filePathString[renameAtPointer] = 48;
    filePathString[renameAtPointer+1] = integerCharString[0];
    filePathString[renameAtPointer+2] = integerCharString[1];
    filePathString[renameAtPointer+3] = integerCharString[2];
    filePathString[renameAtPointer+4] = integerCharString[3];
  }
  else if(fileNameIndex<100000)
  {
    filePathString[renameAtPointer] = integerCharString[0];
    filePathString[renameAtPointer+1] = integerCharString[1];
    filePathString[renameAtPointer+2] = integerCharString[2];
    filePathString[renameAtPointer+3] = integerCharString[3];
    filePathString[renameAtPointer+4] = integerCharString[4];
  }
}

void loop()
{
	unsigned short int streamCount=0;
	while(true)
	{
	  for(streamCount=0; streamCount<globalAnimationQue.totalNodes; streamCount++)
	  {
		  playStream(streamCount);
	  }
	  yield();
	}
}

void playStream(unsigned short int streamID)
{
  unsigned short int frameIndex=0, bufferIndex=0;
  char* frameBuffer = new char[rows*cols*3];
  unsigned short int pixelValue=0;

  PLNODE* queLocater = globalAnimationQue.findByID(streamID);
  if(queLocater!=NULL)
  {
    display.clear();
    Serial.printf("\r\nPlaying [");
    Serial.print(queLocater->_fullPath);
    Serial.printf("] Que Index[%d]\tFrames to Play[%d]\tFrameDelay[%d]", streamID, queLocater->_totalFrames, queLocater->_frameDelay);
    outputStream = SD.open(queLocater->_fullPath, "r");
    for(frameIndex=0; frameIndex<queLocater->_totalFrames; frameIndex++)
    {
      outputStream.readBytes(frameBuffer, rows*cols*3);
      for(yCount=0; yCount<rows; yCount++)
      {
        for(xCount=0; xCount<cols; xCount++)
        {
          memcpy(tempColour, frameBuffer+bufferIndex, 3);
          pixelValue = tempColour[0]+tempColour[1]+tempColour[2];
          if(pixelValue>0)
          {
            setPixel(xCount, yCount, true);
          }
          else
          {
            setPixel(xCount, yCount, false);
          }
          bufferIndex+=3;
        }
      }
      bufferIndex=0;
      display.display();
      clearCanvas();
      if(queLocater->_frameDelay==0){yield();}
      else{delay(queLocater->_frameDelay);}
    }
    outputStream.close();
  } 
  delete frameBuffer;
}


//OLED SCREEN Helper Functions
void clearCanvas()
{
  unsigned short int index=0;
  for(index=0; index<1024; index++)
  {
    graphics[index]=0;
  }
}

void setPixel(uint8_t x, uint8_t y, bool value)
{
  if (x >= cols || y >= rows) { return; }
  if (value)
  {
    graphics[x + (y >> 3) * cols] |= 1 << (y & 7);
  }
  else
  {
    graphics[x + (y >> 3) * cols] &= ~(1 << (y & 7));
  }
}

void drawSolidRectangle(uint8_t topLeftX, uint8_t topLeftY, uint8_t width, uint8_t height)
{
  if (topLeftX >= cols) { return; }
  if (width > (uint8_t)(cols - topLeftX)) { width = cols - topLeftX; }

  uint8_t y = topLeftY;
  for (uint8_t page = 0; page < 8; page++)
  {
    if (height == 0) { return; }
    if (y >= 8) { y -= 8; continue; }

    uint8_t heightOnThisPage = min(height, (uint8_t)(8 - y));
    uint8_t mask = ((1 << heightOnThisPage) - 1) << y;
    for (uint8_t x = topLeftX; x < topLeftX + width; x++)
    {
      graphics[x + cols * page] |= mask;
    }
    y = 0;
    height -= heightOnThisPage;
  }
}

void drawRectangle(uint8_t topLeftX, uint8_t topLeftY, uint8_t width, uint8_t height)
{
  for (uint8_t x = topLeftX; x < topLeftX + width; x++)
  {
    setPixel(x, topLeftY, 1);
    setPixel(x, topLeftY + height - 1, 1);
  }

  for (uint8_t y = topLeftY; y < topLeftY + height; y++)
  {
    setPixel(topLeftX, y, 1);
    setPixel(topLeftX + width - 1, y, 1);
  }
}
