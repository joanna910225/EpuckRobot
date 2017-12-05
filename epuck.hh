#ifndef EPUCK_H
#define EPUCH_H

#ifndef STDINT_H
#include <stdint.h>
#endif

#ifndef STRING_H
#include <string>
#endif

#ifndef VECTOR_H
#include <vector>
#endif


typedef struct{
    double pos[2];
    int escapeflag;
    int timecountdown;
    int type;
}memorycell;




class Camera;
class SPICommInterface;
class TaskAllocationMethod;
class IPCInterface;
class PathPlanner;
class Robot
{

private:

  // Task Allocation Methdod Pointer
  TaskAllocationMethod *TaskAllocation;
  int CurrentActionRunning;
  double FoodAtNest;
  int FoodCollected;
  int NValue;;
  double CurrentTheta;


  // Robot's Identifier
  std::string Name;

  // Navigation related Variables
  int MAXSPEED;
  int MINSPEED;  
  int TURNSPEED;
  int avoid_weightleft[8];
  int avoid_weightright[8];
  int LeftWheelVelocity;
  int RightWheelVelocity;

  // Status
  int CurrentState;
  int timer;
  int idletimer;
  int PreviousState;
  double currentpos[2];  //in mm
  int currentheading; //in degree
  int turndirection; //+1 left, -1 right
  std::vector<memorycell> memoryvect;
  int distanceleft;
  int sensorcount;
  int sensorflag; // indicate which sensor detects obstacle before turned to the obstacle


  // Timming related variables
  int RandomWalkTimeStep;
  uint32_t timestamp;

  // Embedded Camera Pointer
  Camera *EmbeddedCamera;
  int BlobCenterX;
  int BlobCenterY;
  int CameraWidthCenter;
  int CameraMargin;

  // SPIComm Pointer
  SPICommInterface *SPI;  

  // Robot Monitoring/Debug
  IPCInterface *IPCROSMONITOR;  

  // Vicon Position Retrival
  IPCInterface *IPCROSVICON;  

  // Vicon Position Retrival
  IPCInterface *IPCROSNEST;  
  
  // Pointer to PathPlanner Object
  PathPlanner *PATHPLANNER;  
  
public:

  Robot(const char * n = NULL);
  ~Robot();

  // SYSTEM FUNCTIONS
  bool Update(uint64_t timestamp);
  bool Init();
  bool Reset();
  Camera* GetCameraDevice();
  SPICommInterface* GetSPICommInterface();

  // TASK ALLOCATION RELATED FUNCTIONS
  int* GetCurrentActionRunning();
  int* GetAmountOfFoodCollected();
  double* GetAmountOfFoodAtNest();
  double * GetCurrentTheta();
  int* GetNValue();

  // MAIN ROBOT ACTIONS
  void CollectBlob();
  void MoveTowardsBlob();
  bool GrabbedSomething();
  void RandomWalk();
  void SetRandomSpeeds();
  void ObstacleAvoidance();
  void AvoidArenaBoundaries();
  void GoToNest();
  void GoForward();
  void GoBackwards();
  void GetOutOfNest();
  bool RobotIsWithinNest();
  void DoNestAlignment();
  void TurnLeft();
  void TurnRight();
  void Wait();
  bool Stop();

  // CAMERA RELATED FUNCTIONS
  bool IsBlobAligned();
  void DoBlobAlignment();
  void RetrieveBlobCenter();
  bool ThereIsABlobInFront();

  // DEBUGGING FUNCTIONS
  void PrintProximityValues();
  void PrintTACValues();
  void PrintMicValues();
  void PrintAccelerometersValues();
  void PrintBatteryLevel();
  void PrintStatus();  

  // States Supports
  int SelectRandomAngle();
  float SelectRandomDistance();
  void RandomTurn();
  void RandomMove();
  void StateSwitch();
  bool timeup(); 
  void idle();
  void BoundryAvoidance();
  void DetectBoundry(int *sensorID);
  bool IsRobotImage();
  bool NotBoundryNotRobot();

  void CalculateHeading();
  void CalculatePos();

  void GetCurrentPos();
  void GetCurrentState();
  void GetCurrentHeading();

  int RandomAngAve(int min, int max );
  void repelboundry();
  void repelcollision();

  void updatememory();
  bool avoidbannedarea();
  void Turn(int ang);
  int wraptoPI(double ang);
  bool InMemoryRange(double obstaclepos[2],int range);
  void repelgeneral();
  void PrintCurrentMemory();
  int ActualSensorResult(int sensorID);

  void FileCurrentPos();
  void FileBoundry();
  void FileCollision();

};

#endif
