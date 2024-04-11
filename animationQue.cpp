#include "animationQue.h"
#include <stdlib.h> 
#include <stdio.h>
#include <math.h>
#include <string>
#include <cstring>
#include <Arduino.h>

animationQue::animationQue()
{
  startPointer = NULL;
  totalNodes = 0;
}

void animationQue::add(char* animationName, unsigned short int frames, unsigned short int frameDelay)
{
	char endPart[] = "STREAM", streamType[] = ".stream";
	char* streamFolderName = new char[ strlen(animationName)+strlen(endPart)+2 +100];
	char* streamName = new char[ strlen(streamFolderName)+1+strlen(animationName)+strlen(streamType)+1 +100];
  
	PLNODE* nodePointer;
	PLNODE* prevNode;
	nodePointer = (PLNODE*) malloc(sizeof(PLNODE));
	nodePointer->_nodeID = totalNodes;
	nodePointer->_animationName = new char[strlen(animationName)+1];
	memcpy(nodePointer->_animationName, animationName,strlen(animationName)+1);
	nodePointer->_totalFrames = frames;
	nodePointer->_frameDelay = frameDelay;
  
	//apend STREAM to the end of the animation name
	streamFolderName[0]='/';
	strcpy(streamFolderName+1, animationName);
  nodePointer->_folderPath = new char[strlen(streamFolderName)+1];
	memcpy(nodePointer->_folderPath, streamFolderName, strlen(streamFolderName));
  nodePointer->_folderPath[strlen(streamFolderName)]=0;
	strcpy(streamFolderName+strlen(animationName)+1, endPart);
	streamFolderName[strlen(animationName)+strlen(endPart)+1]=0;
  
	//create stream file name with above dir name apended
	memcpy(streamName, streamFolderName, strlen(streamFolderName));
	streamName[strlen(streamFolderName)]= '/';
	memcpy(streamName+strlen(streamFolderName)+1, animationName, strlen(animationName));
	memcpy(streamName+strlen(streamFolderName)+1+strlen(animationName), streamType, strlen(streamType));
	streamName[strlen(streamFolderName)+1+strlen(animationName)+strlen(streamType)]=0; 

	nodePointer->_fullPath = new char[strlen(streamName)+1];
  memcpy(nodePointer->_fullPath, streamName, strlen(streamName));
  nodePointer->_fullPath[strlen(streamName)]=0;

  nodePointer->_streamFolderPath = new char[strlen(nodePointer->_folderPath)+strlen(endPart)+1];
  memcpy(nodePointer->_streamFolderPath, streamFolderName, strlen(streamFolderName));
  memcpy(nodePointer->_streamFolderPath+strlen(streamFolderName), endPart, strlen(endPart) );
  nodePointer->_streamFolderPath[strlen(nodePointer->_folderPath)+strlen(endPart)]=0;
  
  //set up next point
  if(totalNodes==0)
  {
    startPointer = nodePointer;
    nodePointer->nextNode = NULL;
  }
  else
  {
    prevNode = findLast();
    prevNode->nextNode = nodePointer;
    nodePointer->nextNode = NULL;
  }
  totalNodes++;
}


PLNODE* animationQue::findByID(unsigned short int nodeID)
{
  PLNODE* nodePointer = startPointer;
  
  while(nodePointer != NULL)
  { 
      if(nodePointer->_nodeID==nodeID)
      {
        return nodePointer;
      }
      nodePointer = nodePointer->nextNode; 
   } 
   return NULL;
}

PLNODE* animationQue::findLast()
{
  PLNODE* nodePointer = startPointer;
  while(nodePointer != NULL)
  { 
      if(nodePointer->nextNode==NULL)
      {
        return nodePointer;
      }
      nodePointer = nodePointer->nextNode; 
   } 
   return NULL;
}
