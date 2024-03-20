#ifndef animationQue_h
#define animationQue_h

typedef struct PLNode{ 
  unsigned short int _nodeID;
  char* _animationName;
  char* _folderPath;
  char* _fullPath;
  unsigned short int _totalFrames;
  unsigned short int _frameDelay;
  PLNode* nextNode; 
}PLNODE;

class animationQue
{
  public:
    animationQue();
    
    unsigned short int add(char* animationName, unsigned short int frames, unsigned short int frameDelay);
    PLNODE* findByID(unsigned short int nodeID);
	  PLNODE* findLast();
	  
	  PLNODE* startPointer;
	  unsigned short int totalNodes;
  private:
    
    
};
#endif
