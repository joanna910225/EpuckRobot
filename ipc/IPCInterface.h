#ifndef IPCINTERFACE_H
#define IPCINTERFACE_H

#ifndef STRING_H
#include <string>
#endif

#ifndef IPC_HH
#include <ipc.hh>
#endif

#ifndef IPC_DATATYPES_H
#include <IPCDataTypes.h>
#endif

extern int userQuit;

class ELolMessage;
class SPICommInterface;
class Camera;
class Robot;
class IPCInterface
{

 private :
  
  int Port;
  std::string IPAddress;
  
  // Main IPC Object
  IPC::IPC ipc;

  // Pointers to Robot Devices and sensors
  Robot *RobotPointer;
  SPICommInterface *SPI;
  Camera *EmbeddedCamera;

  // Robot Position Array
  int ROBOTPOSITION[3];

  // Amount OF Food
  double AmountOfFoodAtNest[1];

 public :
   
  IPCInterface(std::string IPAddress,int Port);

  // INITIALIZATION OF IPC CLASS
  void Init(Robot *RobotPointer);
  void Init();

  // We capture the SPI pointer for retrieving
  // the onboard sensor values
  void InitializeSPI(SPICommInterface *SPI);

  // We capture the Embedded Camera pointer
  // for retrieving the blob's data
  void InitializeCamera(Camera *EmbeddedCamera);

  // MAIN SEND FUNCTION
  void Send();

  // TASK ALLOCATION METHOD VALUS
  void SendTaskAllocationMethodValues();
  void SendCurrentRunningAction();
  void SendAmountOfFoodCollected();
  void SendCurrentTheta();
  void SendNValue();

  // CAMERA VALUES
  void SendCameraValues();
  void SendCameraResolution();
  void SendBlobCenter();

  // ON-BOARD SENSOR VALUES
  void SendSensorValues();
  void SendIRData();
  void SendIRFilteredData();
  void SendAccelerometerData();
  void SendMicData();
  void SendBatteryData();
  
  // Receiver Callback
  static void ReceiveCallback(const ELolMessage*msg, void *conn, void *ptr);

  // Retrieve Amounf Of Food At Nest Obtained by ROS
  double GetAmountOfFoodAtNest(); 
  
  // Retrieve Robot's Position Obtained by ROS
  int* GetRobotPositionArray();
  int GetRobotPositionX();
  int GetRobotPositionY();
  int GetRobotPositionA();

  ~IPCInterface();
};
#endif






