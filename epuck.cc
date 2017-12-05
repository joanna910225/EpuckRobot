
#ifndef IOSTREAM_H
#include <iostream>
#endif


#ifndef MATH_H
#include <math.h>
#endif

#ifndef CAMERA_H
#include <camera.h>
#endif

#ifndef SPICOMMINTERFACE_H
#include <SPICommInterface.h>
#endif

#ifndef IPCInterface_H
#include <IPCInterface.h>
#endif

#ifndef PATHPLANNER_H
#include <PathPlanner.h>
#endif

#ifndef TASKALLOCATIONMETHOD_H
#include <TaskAllocationMethod.h>
#endif

#ifndef RTM_H
#include <RTM.h>
#endif

#ifndef ARTM_H
#include <ARTM.h>
#endif

#ifndef FSTREAM
#include <fstream>
#endif

#define PI 3.1415926
#define RHO_WC 0.97 //Wrapped Cauchy concentration
#define LAMDA 15    //Distance select parameter 
#define IMPACTRANGE 200
#define ROBOTRADIUS 35
#define BOUNDARYDISTANCE 100
#define MEMORYTIME 18000
#define DETECTRANGE 10

#include "epuck.hh"

using namespace std;

int g_logtofile(0);
int g_turning_speed(10);

ofstream Fposout("logpos.txt");
ofstream Fboundaryout("logboundary.txt");
ofstream Fcollisionout("logcollision.txt");

/***************Global Varibles*****************/
extern int timecount;
extern int sensorcount[8];
extern int userQuit;
extern char * g_host;
extern int filecount;

/***************Internal Lib*******************/
#ifndef CAMERA_H
#include <camera.h>
#endif

#ifndef SPICOMMINTERFACE_H
#include <SPICommInterface.h>
#endif

#ifndef IPCInterface_H
#include <IPCInterface.h>
#endif

#ifndef PATHPLANNER_H
#include <PathPlanner.h>
#endif

#ifndef TASKALLOCATIONMETHOD_H
#include <TaskAllocationMethod.h>
#endif

#ifndef RTM_H
#include <RTM.h>
#endif

#ifndef ARTM_H
#include <ARTM.h>
#endif


/** Main Robot's class constructor 
   \param str Robot's identifier */

Robot::Robot(const char *str) : 

  //Avoid Obstacles Constant Arrays
  //avoid_weightleft {-10,-10,-5, 0, 0, 5, 10, 10},
  //avoid_weightright {10, 10, 5, 0, 0, -5, -10, -10},

  MAXSPEED(800),
  TURNSPEED(410),
  MINSPEED(10),
  CurrentState(0),
  idletimer(0),
  timer(0),
  PreviousState(0),
  currentheading(0),
  turndirection(0),
  currentpos{0,0},
  distanceleft(0),
  sensorcount(0),
  sensorflag(-1)

  //MOVESPEED(500), //Speed at which epuck travel 10cm in 1 sec 
                  //when timer updates every 10ms

  //TURNSPEED(500), //Speed at which epuck turn 90 degrees in 900ms 
                  //when timer updates every 10ms

{
  // Initialization for rand()
  // functions
  srand(time(NULL));

  //Task Allocation Method Initialization
  TaskAllocation = new ARTM(this);
  CurrentActionRunning=1;
  FoodCollected=0;

  cout << "Create robot "<< endl;
  Name = str;
    
  // Create Robot's Devices
  EmbeddedCamera = new Camera(320,240);
  SPI = new SPICommInterface();
  //IPCROSMONITOR = new IPCInterface("192.168.20.199",10000);
  //IPCROSVICON = new IPCInterface("192.168.20.199",12000);
  //IPCROSNEST = new IPCInterface("192.168.20.199",14000);
  //PATHPLANNER = new PathPlanner(IPCROSVICON);

  // Camera Related Variables
  CameraMargin = 20;
  CameraWidthCenter = EmbeddedCamera->GetCameraWidth()/2;

  // Initialization of Robot Velocities
  this->RightWheelVelocity = 0;
  this->LeftWheelVelocity = 0;
}

/** Robot's class destructor */
Robot::~Robot(){
  cout<< " Destroy robot "<<endl;
  delete TaskAllocation;
  delete EmbeddedCamera;
  delete SPI;
  //delete IPCROSMONITOR;
  //delete IPCROSVICON;
  //delete IPCROSNEST;
  //delete PATHPLANNER;
}

/** Robot's update process returns true after updating robot's state
    \param TimeStamp TimeStamp */
bool Robot::Update(uint64_t TimeStamp) 
{
  //cout << "robot update" << endl;
  timestamp = TimeStamp;  
  SPI->Run();
  //cout<< timecount <<endl;
  //cout << "current state: "<<CurrentState <<endl;
  //TurnRight();
  StateSwitch();
  updatememory();
  //PrintProximityValues();
  //PrintTACValues();
  /*if(NotBoundryNotRobot() || IsRobotImage()){
    cout << "is robot" << endl;
    cout << endl;
  }
  else
    cout << "not robot"<< endl;*/



  //PrintBatteryLevel();
  
    /*if(CaptureBoundryImage())
      cout << "boundry" << endl;
    else
      cout<<"not boudnry"<<endl;*/
 /* memorycell obs1;
  obs1.type = 1;
  memorycell obs2;
  obs2.type = 0;
  memoryvect.push_back(obs1);
  memoryvect.push_back(obs2);
  memoryvect.push_back(obs1);
  memoryvect.push_back(obs2);
  cout << "---------------------current memory---------------------- " << endl;
          for(int i = 0; i<memoryvect.size(); i++){
            cout << i << ". pos["<< memoryvect[i].pos[0] << "," << memoryvect[i].pos[1] << "] "
                      << " escapeflag = " << memoryvect[i].escapeflag
                      << " timecountdown = " << memoryvect[i].timecountdown
                      << " type = " << memoryvect[i].type << endl;
          }
  cout << "---------------------current memory---------------------- " << endl;
          
  for(int i=0;i<memoryvect.size();i++){
    if(memoryvect[i].type == 0)
      memoryvect.erase(memoryvect.begin()+i);
  }

  cout << "---------------------after memory---------------------- " << endl;
          for(int i = 0; i<memoryvect.size(); i++){
            cout << i << ". pos["<< memoryvect[i].pos[0] << "," << memoryvect[i].pos[1] << "] "
                      << " escapeflag = " << memoryvect[i].escapeflag
                      << " timecountdown = " << memoryvect[i].timecountdown
                      << " type = " << memoryvect[i].type << endl;
          }
  cout << "---------------------after memory---------------------- " << endl;
*/

    //IPCROSMONITOR->Send();
    //PrintProximityValues();

  /*
    CurrentActionRunning = TaskAllocation->Action();
  
    if(CurrentActionRunning==0)
      RandomWalk();
    else if(CurrentActionRunning==1)
      Wait();
    else if(CurrentActionRunning==2)

  
    // I AM SURE THERE IS A WAY TO CHANGE THIS
    FoodAtNest = IPCROSNEST->GetA)mountOfFoodAtNest();
  CurrentTheta = TaskAllocation->GetCurrentTheta();
  NValue = TaskAllocation->GetNValue();*/


  SPI->SetSpeeds(LeftWheelVelocity,RightWheelVelocity);
  FileCurrentPos();
  FileBoundry();
  FileCollision();
  //SPI->SetSpeeds(0,0);
  //cout << timecount<<": set speed "<< LeftWheelVelocity << " " << RightWheelVelocity <<endl;
  //GetCurrentPos();
  //GetCurrentHeading();
 //SPI->Update(SPI->spi_device);
  return true; // run again
}

/** Robot's Initialization procedure.  It basically creates and
   intializes video and communications objects.  */
bool Robot::Init(){

  cout << " Init ... " << std::endl;

  // Variable for changing the Random Step
  RandomWalkTimeStep = 0;
  
  // Initialization of the SPICommInterface
  SPI->Init();
    
  // Initialization of the Embedded Camera
  EmbeddedCamera->Init();

  // Initialization of the IPC Communication Pipe
  //IPCROSMONITOR->Init(this);      
  //IPCROSVICON->Init();      
  //IPCROSNEST->Init();      

  return true;
}



/** Robot resets its the SPIComm process  */
bool Robot::Reset(){
    SPI->Reset();
    SPI->Run();
    return true;
}

/** Returns a pointer to Robot's Embedded Camera Device  */
Camera* Robot::GetCameraDevice(){
  return EmbeddedCamera;
}

/** Returns a pointer to Robot's SPIComm Interface  */
SPICommInterface* Robot::GetSPICommInterface(){
  return SPI;
}
/*********************Basic Movement Functions**************/

/** Robot Stops */
bool Robot::Stop(){
    LeftWheelVelocity = 0;
    RightWheelVelocity = 0;
    SPI->SetSpeeds(LeftWheelVelocity,RightWheelVelocity);
    SPI->Run();
    return true;
}

/** Move the Robot Forward */
void Robot::GoForward(){
  RightWheelVelocity = MAXSPEED;
  LeftWheelVelocity = MAXSPEED;
}

/** Move the Robot Backwards */
void Robot::GoBackwards(){
  RightWheelVelocity = -MAXSPEED;
  LeftWheelVelocity = -MAXSPEED;
}

/** The Robot Turns Left Constantly */
void Robot::TurnLeft(){
  turndirection = 1;
  RightWheelVelocity = TURNSPEED;
  LeftWheelVelocity = -TURNSPEED;
}

/** The Robot Turns Right Constantly */
void Robot::TurnRight(){
  turndirection = -1;
  RightWheelVelocity = -TURNSPEED;
  LeftWheelVelocity = TURNSPEED;
}

/*********************STATES SUPPORTS*******************************/

// Wrapped Cauchy 
int Robot::SelectRandomAngle(){
  float x = rand()%(1000)/(float)(1001);
  float wc = 2 * atan( (1-RHO_WC)/(1+RHO_WC) * tan( PI * (x-0.5) ) );
  int degree = wc * 180/PI;
  //cout << " random degree "<< degree << endl;
  return degree;
}

float Robot::SelectRandomDistance(){ 
  float x = rand()%(1000)/(float)(1001);
  float dis = -LAMDA *log(x);
  //cout << " logx " << log(x) <<endl;
  //cout<<" x " <<x<<endl;
  //cout<< " random distance " << dis <<endl;
  // limitations
  if(dis >50)
    dis = 50;
  if (dis < 7)
    dis = 7;
  return dis; 
}

void Robot::Turn(int ang){
  timer = abs(ang);
  if(ang > 0){
    TurnLeft(); 
    CalculateHeading();
    cout << "turn left " << abs(ang) << " degree "<< endl;
    //cout << endl;
  }
  if(ang < 0){
    TurnRight();
    CalculateHeading();
    cout << "turn right " << abs(ang) << " degree "<< endl;
    //cout << endl;

  }
  if(ang == 0){
    //Stop();
    cout << " no turn -->stop" << endl;
    //cout << endl;
  }
}
void Robot::RandomTurn(){
  CurrentState = -1;
  int ang = SelectRandomAngle();
  cout << " select random angle = " << ang << endl;
  if(abs(ang)<5){
    ang = 0;
    cout << "selcted ang < + - 5, change to: " << ang << endl;
  }
  Turn(ang);
   // will start turn in current time slot
}

void Robot::RandomMove(){
  CurrentState= 2;
  int dis = SelectRandomDistance();
  cout << " select random distance = " << dis << endl;
  timer = dis * 10;
  //cout << "set move timer = " << timer << endl;
  GoForward();
  CalculatePos();
}

void Robot::idle(){
  timer = 0;
  //cout << "previous state = " << PreviousState << endl;
  switch(PreviousState){
    case 0:{ 
     // cout << "previous state = " << PreviousState << endl;
      CurrentState = 1;
     // cout << "going to state " << CurrentState << endl;
      break;
    }
    case 2:{
     // cout << "previous state = " << PreviousState << endl;
      CurrentState = 0;
     // cout << "going to state " << CurrentState << endl;
      break;
    }
    case 3:{
      CurrentState = 4; 
    }
  }
  if(timer > 0)
    Stop();
}

void Robot::DetectBoundry(int *sensorID){
  vector<int>* IRSENSORSVECTOR = SPI->GetIRDataVector();
  //cout << " sensor reading [ ";
  int maxvalue = 0;
  for(vector<int>::iterator CurrentIR = IRSENSORSVECTOR->begin();
       CurrentIR != IRSENSORSVECTOR->end();
       ++CurrentIR){

    // find sensor with largest value (closest direction to obstable)
    if(*CurrentIR > maxvalue && *CurrentIR > 200 ){ 
      maxvalue = *CurrentIR;
      *sensorID =  CurrentIR - IRSENSORSVECTOR->begin();
      //sensorcount[*sensorID]++;
    }
  }

}

int Robot::ActualSensorResult(int sensorID){
  vector<int>* IRSENSORSVECTOR = SPI->GetIRDataVector();
  switch(sensorID){
    case 0:{
      if(*IRSENSORSVECTOR->begin()+1 > 0 || *IRSENSORSVECTOR->end() > 0 )
        return sensorID;
      else 
        return -1;
      break;
    }
    case 1:{
      if(*IRSENSORSVECTOR->begin() > 0 || *IRSENSORSVECTOR->begin()+2 > 0  )
        return sensorID;
      else
        return -1;
      break;
    }
    case 2:{
      if(*IRSENSORSVECTOR->begin()+1 > 0 || *IRSENSORSVECTOR->begin()+3 > 0  )
        return sensorID;
      else
        return -1;
      break;
    }
    case 5:{
      if(*IRSENSORSVECTOR->begin()+4 > 0 || *IRSENSORSVECTOR->begin()+6 > 0  )
        return sensorID;
      else
        return -1;
      break;
    }
    case 6:{
      if(*IRSENSORSVECTOR->begin()+5 > 0 || *IRSENSORSVECTOR->end() > 0  )
        return sensorID;
      else
        return -1;
      break;
    }
    case 7:{
      if(*IRSENSORSVECTOR->begin()+6 > 0 || *IRSENSORSVECTOR->begin()+1 > 0  )
        return sensorID;
      else
        return -1;
      break;
    }
  }
}


void Robot::BoundryAvoidance(){
  int sensorID = -1;
  DetectBoundry(&sensorID);

  // avoid sudden error in sensor
  if(sensorID != -1 && sensorcount < 4){
    sensorID = -1;
    sensorcount++;
  }
  else
    sensorcount = 0;
  //sensorID =  ActualSensorResult(sensorID);

  PreviousState = 2;
  sensorflag = sensorID;
  //sensorcount[sensorID]++;
  //cout <<"timecountafter " << timecount << endl;

  switch(sensorID){
    case -1:{
      //PrintProximityValues();
      if(avoidbannedarea()){ // run into banned area
        CurrentState = -1;
        PreviousState = 3;
      }
      break;

    }
    case 0:{
      PrintProximityValues();
      cout << "object detected at sensor: " << sensorID << endl; 
      //cout << "turnleft 180 degree" << endl; 
      //cout << "timecount = " << timecount << endl;
      //distanceleft = timer - timecount;
      //distanceleft = BOUNDARYDISTANCE;
      timer = 0;
      timecount = 0;
      CurrentState = 3;
      break;
    }
    case 1:{
      PrintProximityValues();
      cout << "object detected at sensor: " << sensorID << endl;
      cout << "turnright 60 degree" << endl;
      //cout << "timecount = " << timecount<< endl;
      TurnRight(); 
      CalculateHeading();
      //distanceleft = timer - timecount;
      //distanceleft = BOUNDARYDISTANCE;
      timer = 60;
      timecount = 0;
      CurrentState = 3;
      break;
    }
    case 2:{
      PrintProximityValues();
      cout << "object detected at sensor: " << sensorID << endl;
      cout << "turnright 90 degree" << endl;
      //cout << "timecount = " << timecount<< endl;
      TurnRight();
      CalculateHeading();
      //distanceleft = timer - timecount;
      //distanceleft = BOUNDARYDISTANCE;
      timer = 90;
      timecount = 0;
      CurrentState = 3;
      break;
    }
    case 3:break;
    case 4:break;
    case 5:{
      PrintProximityValues();
      cout << "object detected at sensor: " << sensorID << endl;
      cout << "turnleft 90 degree" << endl;
      //cout << "timecount = " << timecount<< endl;
      TurnLeft();
      CalculateHeading();
      //distanceleft = timer - timecount;
      //distanceleft = BOUNDARYDISTANCE;
      timer = 90;
      timecount = 0;
      CurrentState = 3;
      break;
    }
    case 6:{
      PrintProximityValues();
      cout << "object detected at sensor: " << sensorID << endl;
      cout << "turnleft 60 degree" << endl;
      //cout << "timecount = " << timecount<< endl;
      TurnLeft();
      CalculateHeading();
      //distanceleft = timer - timecount;
      //distanceleft = BOUNDARYDISTANCE;
      timer = 60;
      timecount = 0;
      CurrentState = 3;
      break;
    }
    case 7:{
      PrintProximityValues();
      cout << "object detected at sensor: " << sensorID << endl;
      //cout << "turnright 180 degree" << endl;
      //distanceleft = timer - timecount;
      //distanceleft = BOUNDARYDISTANCE;
      timer = 0;
      timecount = 0;
      CurrentState = 3;
      break;
    }
    default: break;
  }
  //cout << "current heading: " << currentheading << endl;
    
}


void Robot::StateSwitch(){
  switch(CurrentState){
    case -1:{
      if(timeup()){
        cout << endl;
        GetCurrentPos();
        GetCurrentHeading();
        cout << "current state: "<< CurrentState << endl;
        idle();
        PreviousState = -1;
        break;
      }
      else{ //turning
        //cout << "current state" << CurrentState << endl;
       // cout << "state -1 CalculateHeading timecount" << timecount << endl;
        CalculateHeading();
        //GetCurrentHeading();
        //cout << "turndirection" << turndirection<<endl;
        //cout << "timecount" << timecount <<endl;
        break;
      }
    }
    case 0:{
      if(timeup()){  
        cout << endl;
        GetCurrentPos();
        GetCurrentHeading();
        cout << "current state: "<< CurrentState << endl;      
        RandomTurn();  
        PreviousState = 0;
        break;
      }
      else {        
        break;
      }
    }
    case 1:{
      if(timeup()){
        cout << endl;
        GetCurrentPos();
        GetCurrentHeading();
        cout << "current state: "<< CurrentState << endl;
        RandomMove();
        PreviousState = 1;
        break;
      }
      else{ //idling
        break;
      }
    } 
    case 2:{
      if(timeup()){ // moved the desired distance
        distanceleft = 0;
        cout << endl;
        GetCurrentPos();
        GetCurrentHeading();
        cout << "current state: "<< CurrentState << endl;
        CurrentState = -1; 
        PreviousState = 2;
        break;
      }
      else // still moving
        //cout << "state 2 moving......." <<timecount << endl;
        CalculatePos();
        BoundryAvoidance();
        //GetCurrentPos();      
        break;
    }
    case 3:{ // turn to obstacle and see if it is bourndy or not
      if(timeup()){ 
        cout << " turned to face obstacles" << endl;
        GetCurrentPos();
        GetCurrentHeading();

        // store obstacle position in a temp variable
        double tempobstacle[2];
        tempobstacle[0] = currentpos[0]+(ROBOTRADIUS*2 + DETECTRANGE)*cos(currentheading);
        tempobstacle[1] = currentpos[1]+(ROBOTRADIUS*2 + DETECTRANGE)*sin(currentheading);

        // Camera detection: if is robot or not boundry,add pos, and repel
        if(NotBoundryNotRobot() || IsRobotImage() ){ //if robot or not boundry,add pos, and repel
          cout << "It was a robot" << endl;

          // If it is in the impact range of ROBOT pos that already in the memory, do not store it
          if(!InMemoryRange(tempobstacle,IMPACTRANGE+ROBOTRADIUS)){ 
            memorycell obstaclepos;
            obstaclepos.pos[0] = tempobstacle[0];
            obstaclepos.pos[1] = tempobstacle[1];
            obstaclepos.escapeflag = 0;
            obstaclepos.timecountdown = -1;
            obstaclepos.type = 1; //is robot
            memoryvect.push_back(obstaclepos);
            cout << "---------------------current memory---------------------- " << endl;
            PrintCurrentMemory();
          }
          else
            cout << "InMemoryRange ROBOT " << endl;
          
          //repel
          PreviousState = 3;
          CurrentState = -1;
          distanceleft = IMPACTRANGE;
          cout << "Collision Repel"<< endl;
          repelcollision();
          //repelcollision();
          break;
        }
        else{ // if it is boundry, remember in boundry list forever
          cout << "It is boundry" << endl;

          // If it is in the impact range of BOUNDRY pos that already in the memory, do not store it
          if(!InMemoryRange(tempobstacle,BOUNDARYDISTANCE+ROBOTRADIUS)){ 
            memorycell obstaclepos;
            obstaclepos.pos[0] = currentpos[0]+(DETECTRANGE + IMPACTRANGE)*cos(currentheading);
            obstaclepos.pos[1] = currentpos[1]+(DETECTRANGE + IMPACTRANGE)*sin(currentheading);
            obstaclepos.escapeflag = 0;
            obstaclepos.timecountdown = -1;
            obstaclepos.type = 0; //is boundry
            memoryvect.push_back(obstaclepos);
            PrintCurrentMemory();
          }
          else
            cout << "InMemoryRange BOUNDRY " << endl;

          //repel
          distanceleft = BOUNDARYDISTANCE;
          PreviousState = 3;
          CurrentState = -1;
          cout << "Boundry Repel" << endl;
          repelcollision();
          //repelboundry();
          break;
        }
      }
      else{ //turning to front
        CalculateHeading();
        break;
      }
    }

    case 4:{ // Turned to repel angle and start to move forward
      if(timeup()){
        GoForward();
        CalculatePos();
        timer = distanceleft;
        cout << "Turned to repel angle and start to move forward " << timer << endl;
        CurrentState = 2;
        break;
      }
      else
        break;
    }
  }
}


bool Robot::InMemoryRange(double obstaclepos[2],int range){
  int i = 0;
  for(i = 0; i < memoryvect.size();i++){
    double dis = sqrt(pow( obstaclepos[0] - memoryvect[i].pos[0],2) +
                      pow( obstaclepos[1] - memoryvect[i].pos[1],2));
    if(dis < range){
      return 1;
      break;
    }
  }

  if(i == memoryvect.size())
    return 0;
}

void Robot::updatememory(){
  for(int i = 0; i<memoryvect.size(); i++){
    if(memoryvect[i].escapeflag == 0){
      // if not escaped from the range yet, calculate distance between obstacle and robot
      double dis = sqrt(pow( currentpos[0] - memoryvect[i].pos[0],2) +
                        pow( currentpos[1] - memoryvect[i].pos[1],2));
      if(memoryvect[i].type == 0 && dis > (BOUNDARYDISTANCE + ROBOTRADIUS) ){
      // escaped boundary range 
        memoryvect[i].escapeflag = 1;
        cout << " escape boundary memory position !!!!!!!!!!! when time = " << timecount << endl;
        PrintCurrentMemory();
      }
      else if(memoryvect[i].type == 1 && dis > (IMPACTRANGE + ROBOTRADIUS) ){ 
      // escaped obstacle impact range and set memory time
        memoryvect[i].escapeflag = 1;
        memoryvect[i].timecountdown = MEMORYTIME;
        cout << " escape collision memory position !!!!!!!!!!! when time = " << timecount << endl;
        PrintCurrentMemory();
      }  
      

    }
    else if(memoryvect[i].escapeflag == 1){
      // if alreday escaped from impact range
      if(memoryvect[i].timecountdown > 0)
      // count down memory time
        memoryvect[i].timecountdown--;
      else if(memoryvect[i].timecountdown == 0)
      // if countdown = 0, erase the memory
        memoryvect.erase(memoryvect.begin()+i);     
    }
    else
      cout << "escapeflag fault" << endl;
    
  }
}

int Robot::wraptoPI(double ang){
  if(fabs(ang) > 180){
    if(ang < 0)
      ang = ang + 360;
    else if (ang > 0)
      ang = ang - 360;
  }
  return ang;
}

bool Robot::avoidbannedarea(){
  int i = 0;
  for(i = 0; i < memoryvect.size(); i++){

    double dis = sqrt(pow( currentpos[0] - memoryvect[i].pos[0],2) +
                      pow( currentpos[1] - memoryvect[i].pos[1],2));

    // Calculate the angle of memory pos with repect to robot current position
    double tempang = atan( (memoryvect[i].pos[1] - currentpos[1]) / 
                           (memoryvect[i].pos[0] - currentpos[0]) ) * 180 / PI;
    int AngObWRTRob = wraptoPI(tempang);
    int ang2repel = 0;

    // angle betweeen obstacle direction and robot current heading
    int AngOb2Head = AngObWRTRob - currentheading;  

    // if run into memory position for boundry
    /*if(memoryvect[i].type == 0 && dis <= (BOUNDARYDISTANCE + ROBOTRADIUS) 
                               && memoryvect[i].escapeflag == 1 
                               && abs(AngOb2Head) < 90){

      distanceleft = BOUNDARYDISTANCE;
      cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
      cout << "distance to boundary listed: " << dis << endl;
      cout << " Avoid banned area of boundary at position [" ;
      int memoryposx = memoryvect[i].pos[0];
      int memoryposy = memoryvect[i].pos[1];
      cout << memoryposx << "," << memoryposy << "]" << endl;
      GetCurrentPos();

      
      cout << "Boundary range is " << BOUNDARYDISTANCE + ROBOTRADIUS << endl;

      //int AngAvoidOb = RandomAngAve(150,210); // Generate random angle for avoiding with respect to memory pos
      GetCurrentHeading();
      cout << "Obstacle is at " << AngObWRTRob << " degree of myself" << endl;
      
      //cout << "tempang " << tempang << endl;
      if(AngOb2Head > 0) 
        //ang2repel = -(180 - 2*AngOb2Head);
        ang2repel = RandomAngAve(150-AngOb2Head,210-AngOb2Head);
      else
        //ang2repel = 180 - 2*abs(AngObWRTRob);
        ang2repel = RandomAngAve(150+abs(AngOb2Head),210+abs(AngOb2Head));
      //cout << "About to turn to " << AngAvoidOb << " degree with respect to banned boundary" << endl;
      //int ang = AngAvoidOb - (AngObWRTRob - currentheading);
      ang2repel = wraptoPI(ang2repel);
      Turn(ang2repel);
      cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
      return true;
      break;
    }*/
    // if run into memory position for collision
    if(memoryvect[i].type == 1 && dis <= (IMPACTRANGE + ROBOTRADIUS) 
                                    && memoryvect[i].escapeflag == 1
                                    && abs(AngOb2Head) <= 90){
      distanceleft = IMPACTRANGE;
      cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
      cout << "distance to robot banned area listed: " << dis << endl;
      cout << " Avoid banned area of other robots at position [" ;
      int memoryposx = memoryvect[i].pos[0];
      int memoryposy = memoryvect[i].pos[1];
      cout << memoryposx << "," << memoryposy << "]" << endl;
      GetCurrentPos();
      cout << "Robot impact range is " << IMPACTRANGE + ROBOTRADIUS << endl;

      //int AngAvoidOb = RandomAngAve(90,270); // Generate random angle for avoiding with respect to memory pos
      GetCurrentHeading();
      cout << "Obstacle is at " << AngObWRTRob << " degree of myself" << endl;

      if(AngOb2Head > 0) 
        ang2repel = -(180 - 2*AngOb2Head);
      else
        ang2repel = 180 - 2*abs(AngObWRTRob);
      //cout << "tempang " << tempang << endl;
      //cout << "About to turn to " << AngAvoidOb << " degree with respect to banned boundary" << endl;
      //int ang = AngAvoidOb - (AngObWRTRob - currentheading);
      ang2repel = wraptoPI(ang2repel);
      if( abs(ang2repel) < 10 )
        ang2repel = 0;
      Turn(ang2repel);
      cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
      return true;
      break;
    }
  }

  if(i == memoryvect.size())
    return false; // the program didn't find it exceed banned area in memory;
}

void Robot::PrintCurrentMemory(){
  cout << "---------------------current memory---------------------- " << endl;
  for(int i = 0; i<memoryvect.size(); i++){
    int memoryposx = memoryvect[i].pos[0];
    int memoryposy = memoryvect[i].pos[1];
     cout << i << ". pos["<< memoryposx << "," << memoryposy << "] "
               << " escapeflag = " << memoryvect[i].escapeflag
               << " timecountdown = " << memoryvect[i].timecountdown
               << " type = " << memoryvect[i].type << endl;
          }
  cout << "---------------------current memory---------------------- " << endl;
}

int Robot::RandomAngAve(int min, int max ){
  int ang = rand() % ( max - min + 1 ) + min;
  ang = wraptoPI(ang);
  return ang;
}

void Robot::repelboundry(){
  int ang = RandomAngAve(150,210); // output ang between -180 ~ 180 degree
  Turn(ang);
}

void Robot::repelcollision(){
  int ang = RandomAngAve(150,210); // output ang between -180 ~ 180 degree
  Turn(ang);
}

void Robot::repelgeneral(){
  int ang = 0;
  switch(sensorflag){
    case 0: {ang = RandomAngAve(90,180);break;
    }
    case 1: {ang = RandomAngAve(90,180);break;
    }
    //case 2: {ang = 10;break;
    //}
    //case 5: {ang = -10;break;
    //}
    case 6: {ang = RandomAngAve(180,270);break;
    }
    case 7: {ang = RandomAngAve(180,270);break;
    }
    default: break;
  }
  sensorflag = -1;
  Turn(ang);
}

bool Robot::timeup(){
  if(timecount < timer)
    return false;
  else{
    timecount = 0;
    timer = 0;
    return true ;
  }
}

void Robot::CalculateHeading(){
  currentheading = currentheading + turndirection;
  //cout << "+1 heading" << endl;
  if( currentheading > 180)
    currentheading = currentheading - 360;
  if( currentheading < -180)
    currentheading = currentheading + 360;
}

void Robot::CalculatePos(){
  double xdiff = cos(currentheading*PI/180);
  double ydiff = sin(currentheading*PI/180);
 // cout << "current heading" << currentheading << endl;
  currentpos[0] = currentpos[0] + xdiff;
  //cout << "+1 x"<< xdiff << endl;
  currentpos[1] = currentpos[1] + ydiff;
  //cout << "+1 y"<< ydiff << endl;
  //GetCurrentPos();
}

void Robot::GetCurrentState(){
  cout << "currentstate: " << CurrentState << endl;
 
}

void Robot::GetCurrentPos(){
  int posx = currentpos[0];
  int posy = currentpos[1];
  cout << "currentpos: [ " << posx << "," << posy << " ]"<<endl;
}

void Robot::FileCurrentPos(){
  int posx = currentpos[0];
  int posy = currentpos[1];
  Fposout << filecount << " "<< posx << " " << posy <<endl;
}

void Robot::FileBoundry(){
  Fboundaryout << filecount << " ";
  for(int i = 0; i<memoryvect.size(); i++){
    if(memoryvect[i].type == 0){
      int boundaryposx = memoryvect[i].pos[0];
      int boundaryposy = memoryvect[i].pos[1];
      Fboundaryout << boundaryposx << " " << boundaryposy << " ";
    }
  }
  Fboundaryout << endl;
}

void Robot::FileCollision(){
  Fcollisionout << filecount << " ";
  for(int i = 0; i<memoryvect.size(); i++){
    if(memoryvect[i].type == 1){
      int collisionposx = memoryvect[i].pos[0];
      int collisionposy = memoryvect[i].pos[1];
      Fcollisionout << collisionposx << " " << collisionposy << " ";
    }
  }
  Fcollisionout << endl;
}

void Robot::GetCurrentHeading(){
  cout << "currentheading: " << currentheading << endl;
}

bool Robot::IsRobotImage(){
  return EmbeddedCamera->RobotDetected();
}
bool Robot::NotBoundryNotRobot(){
  return EmbeddedCamera->NBoundryNRobot();
}
/*********************Print Sensor Data********************/

/** Prints IR Sensor Vector */
void Robot::PrintProximityValues(){

  cout << this->timestamp\
       << " : "\
       << this->Name.c_str()\
       << " IR --[";

  vector<int>* IRSENSORSVECTOR = SPI->GetIRDataVector();
  //vector<int>* IRSENSORSVECTOR = SPI->GetIRFilteredDataVector();

  for (vector<int>::iterator CurrentIR = IRSENSORSVECTOR->begin();\
       CurrentIR != IRSENSORSVECTOR->end();\
       ++CurrentIR)
    printf(" %d", *CurrentIR);

  cout << "]" << endl;
}

/** Prints Battery Level */
void Robot::PrintBatteryLevel(){

  int* BatteryLevel  = SPI->GetBatteryLevel();

  cout << this->timestamp\
       << " : "\
       << this->Name.c_str()\
       << " Bat --[ "\
       << *BatteryLevel\
       << " ] " << endl;
}

/** Prints current robot's state */
/*void Robot::PrintStatus(){

  printf("%d:%s in state [%s] (%d - %d)\n", \
   this->timestamp,     \
   this->Name.c_str());
}*/

/** Returns the Current Action for the Robot  */
int* Robot::GetCurrentActionRunning(){
  return &CurrentActionRunning;
}

/** Returns the Amount of Food Collected by the Robot  */
double* Robot::GetAmountOfFoodAtNest(){
  return &FoodAtNest;
}



/** Returns the Amount of Food Collected by the Robot  */
int* Robot::GetAmountOfFoodCollected(){
  return &FoodCollected;
}

bool Robot::RobotIsWithinNest(){
  return PATHPLANNER->RobotIsWithinNest();
}

/** Returns the Current Theta for the Task Allocation Model  */
double* Robot::GetCurrentTheta(){
  return &CurrentTheta;
}

/** Returns the Current N Value for the Task Allocation Model  */
int* Robot::GetNValue(){
  return &NValue;
}

/** Robot's Collect Action */
/*void Robot::CollectBlob(){
  if(ThereIsABlobInFront())
    {
      if(!GrabbedSomething())
	MoveTowardsBlob();
    }
  else if(GrabbedSomething())
    {
      if(!RobotIsWithinNest())
	GoToNest();
      else
	FoodCollected++;
    }
}*/

/** Robot moves towards a Blob that is seen on the front, the
 robot will move forward until it grabs the blob. */
/*void Robot::MoveTowardsBlob(){

  RetrieveBlobCenter();
  DoBlobAlignment();

  if(IsBlobAligned())
    if(!GrabbedSomething())
      GoForward();
}*/


/** Checks if the Robot Grabbed Something */
bool Robot::GrabbedSomething(){  
  vector<int>* IRSENSORSVECTOR = SPI->GetIRDataVector();

  // if(IRSENSORSVECTOR->front()>= 50 && IRSENSORSVECTOR->back()>= 200)
  if(IRSENSORSVECTOR->back()>= 50)
    return true;
  else
    return false;
}
/*void Robot::SetRandomSpeeds(){

  if(RandomWalkTimeStep==15)
    {
      int range = MAXSPEED - MINSPEED + 1;
      RightWheelVelocity = rand() % range + MINSPEED;
      
      int range2 = MAXSPEED - MINSPEED + 1;
      LeftWheelVelocity = rand() % range2 + MINSPEED;
    
      RandomWalkTimeStep = 0;
    }
  else
    RandomWalkTimeStep++;
}
/** Robot performs a RandomWalk with Simple Obstacle Avoidance */
/*void Robot::RandomWalk(){
  SetRandomSpeeds();
  AvoidArenaBoundaries();
  ObstacleAvoidance();
}*/



/** Robot's Colilsion Avoidance function */
/*void Robot::ObstacleAvoidance(){
  
    vector<int>* IRSENSORSVECTOR = SPI->GetIRDataVector();

    for (vector<int>::iterator CurrentIR = IRSENSORSVECTOR->begin();\
    	 CurrentIR != IRSENSORSVECTOR->end();\
    	 ++CurrentIR)
    {
        LeftWheelVelocity +=\
	  avoid_weightleft[distance(IRSENSORSVECTOR->begin(),CurrentIR)] * (*CurrentIR>>3);
        RightWheelVelocity +=\
	  avoid_weightright[distance(IRSENSORSVECTOR->begin(),CurrentIR)] * (*CurrentIR>>3);
    }
    
    // ??????
    if(timestamp == 100)
      SPI->SetIRPulse(0x1,true);
}

void Robot::AvoidArenaBoundaries(){

  if(PATHPLANNER->RobotIsOutOfBoundaries())
    if(!PATHPLANNER->NestIsInFront())
      TurnLeft();
}*/

/** Move the Robot Towards the Nest */
/*void Robot::GoToNest(){
  if(PATHPLANNER->NestIsInFront())
    GoForward();
  else
    DoNestAlignment();
}




void Robot::GetOutOfNest(){
  RightWheelVelocity = -MAXSPEED*2;
  LeftWheelVelocity = -MAXSPEED*2;
}*/


/** Get the Robot into Waiting Mode */
/*void Robot::Wait(){

  if(RobotIsWithinNest())
    GetOutOfNest();
  else
    if(!PATHPLANNER->NestIsBehind())
      TurnLeft();
    else
      Stop();
}*/



/** Make the Robot Align with the Nest
    Otherwise it will keep on turning until it aligns*/
/*void Robot::DoNestAlignment(){
  
  if(!PATHPLANNER->NestIsInFront())
    TurnRight();
  else
    Stop();
}*/

/** Returns true if the Robot is aligned with the Blob 
    False otherwise */
/*bool Robot::IsBlobAligned(){

  if(BlobCenterX<=0)
    return false;
  
  if (BlobCenterX >= (CameraWidthCenter - CameraMargin)\
      && BlobCenterX <= (CameraWidthCenter + CameraMargin))
    return true;
  else
    return false;
}
*/
/** Robot Aligns with the Blob in front of it */
/*void Robot::DoBlobAlignment(){

  if(BlobCenterX<=0)
    return;
      
  //blob to the left of center
  if(BlobCenterX < (CameraWidthCenter - CameraMargin))
    {
      RightWheelVelocity = 0;
      LeftWheelVelocity = MAXSPEED*10/100;
      return;
    }

  //blob to the right of center
  else if(BlobCenterX > (CameraWidthCenter + CameraMargin))
    {
      RightWheelVelocity = MAXSPEED*10/100;
      LeftWheelVelocity = 0;
      return;
    }
}*/

/** Robot Aligns with the Blob in front of it */
/*void Robot::DoBlobAlignment(){

  if(BlobCenterX<=0)
    return;
      
  //blob to the left of center
  if(BlobCenterX < (CameraWidthCenter - CameraMargin))
    {
      RightWheelVelocity = 0;
      LeftWheelVelocity = MAXSPEED*10/100;
      return;
    }

  //blob to the right of center
  else if(BlobCenterX > (CameraWidthCenter + CameraMargin))
    {
      RightWheelVelocity = MAXSPEED*10/100;
      LeftWheelVelocity = 0;
      return;
    }
}*/

void Robot::RetrieveBlobCenter(){
  BlobCenterX = EmbeddedCamera->GetBiggestBlobCenterX();
  BlobCenterY = EmbeddedCamera->GetBiggestBlobCenterY();
}
/** Returns true if there is a Blob in front of the 
    Robot. Returns false otherwise. */
bool Robot::ThereIsABlobInFront(){
  if(EmbeddedCamera->GetNumberOfBlobsDetected()>0)
    return true;
  else
    return false;
}



void Robot::PrintTACValues(){

  vector<int>* TACSENSORSVECTOR = SPI->GetTACDataVector();
  for (vector<int>::iterator CurrentTAC = TACSENSORSVECTOR->begin();\
       CurrentTAC != TACSENSORSVECTOR->end();\
       ++CurrentTAC)
    printf(" %d", *CurrentTAC);

  printf("\n");

}

/** Prints MIC Values Vector */
/*oid Robot::PrintMicValues(){

  cout << this->timestamp\
       << " : "\
       << this->Name.c_str()\
       << " MIC --[";

  vector<int>* MICROPHONESVECTOR = SPI->GetMicDataVector();

  for (vector<int>::iterator CurrentMIC = MICROPHONESVECTOR->begin();\
       CurrentMIC != MICROPHONESVECTOR->end();\
       ++CurrentMIC)
    printf(" %d", *CurrentMIC);

  cout << "]" << endl;
}*/


/** Prints ACC Values Vector */
void Robot::PrintAccelerometersValues(){

  cout << this->timestamp\
       << " : "\
       << this->Name.c_str()\
       << " ACC --[";

  vector<int>* ACCELEROMETERSVECTOR = SPI->GetAccelerometerDataVector();

  for (vector<int>::iterator CurrentACC = ACCELEROMETERSVECTOR->begin();\
       CurrentACC != ACCELEROMETERSVECTOR->end();\
       ++CurrentACC)
    printf(" %d", *CurrentACC);

  cout << " ]" << endl;
}



/*bool Robot::StepWalked(int *stepl,int *stepr){
  vector<int>* TACSENSORSVECTOR = SPI->GetTACDataVector();
  //cout << "debug " << "left "<< TACSENSORSVECTOR[0] << "right "<< TACSENSORSVECTOR[1] << endl;
  vector<int>::iterator TACl = TACSENSORSVECTOR->begin();
  vector<int>::iterator TACr = TACSENSORSVECTOR->begin()+1;
  //int diffl = *TACl - laststepl;
  //int diffr = *TACr - laststepr;
  cout << " currentstepl: " << *TACl << " currentstepr: " << *TACr <<endl; 
  //cout << " diffstepl: " << diffl << " diffstepr: " << diffr<< " "<<endl;
  //cout << " laststepl: " << laststepl << " laststepr: " << laststepr<< " "<<endl;
  if( *TACl  >= *stepl && *TACr >= *stepr){
    laststepl = *TACl;
    laststepr = *TACr;
    //cout << " after laststepl: " << laststepl << " laststepr: " << laststepr<< " "<<endl;
    return true;
  }
  else{
    laststepl = *TACl;
    laststepr = *TACr;
    //cout << " after laststepl: " << laststepl << " laststepr: " << laststepr<< " "<<endl;
    return false;
  }
}*/