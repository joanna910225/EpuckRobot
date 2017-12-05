#ifndef ROBOT_H
#include <epuck.hh>
#endif

#ifndef SPICOMMINTERFACE_HH
#include <SPICommInterface.h>
#endif

#ifndef ELOLMESSAGE_H
#include <ethlolmsg.h>
#endif

#ifndef CAMERA_H
#include <camera.h>
#endif

#ifndef VECTOR_H
#include <vector>
#endif

#ifndef IOSTREAM_H
#include <iostream>
#endif

#include <IPCInterface.h>

using namespace std;

IPCInterface::IPCInterface(string IPAddress,int Port){
  this->IPAddress = IPAddress;
  this->Port = Port;
}

void IPCInterface::Init(Robot *RobotPointer){
  this->RobotPointer = RobotPointer;
  InitializeSPI(this->RobotPointer->GetSPICommInterface());
  InitializeCamera(this->RobotPointer->GetCameraDevice());

  cout << "Init IPC Client for ROS Process" << endl;
  ipc.SetCallback(ReceiveCallback, (void*)this);
  ipc.Start(IPAddress.c_str(), Port, false);
}

void IPCInterface::Init(){
  cout << "Init IPC Client for ROS Process" << endl;
  ipc.SetCallback(ReceiveCallback, (void*)this);
  ipc.Start(IPAddress.c_str(), Port, false);
}

void IPCInterface::InitializeSPI(SPICommInterface *SPI){
  this->SPI = SPI;
}

void IPCInterface::InitializeCamera(Camera *EmbeddedCamera){
  this->EmbeddedCamera = EmbeddedCamera;
}

void IPCInterface::Send(){
  // SendSensorValues();
  // SendCameraValues();
  SendTaskAllocationMethodValues();
}

void IPCInterface::SendTaskAllocationMethodValues(){
  SendCurrentRunningAction();
  SendAmountOfFoodCollected();
  SendCurrentTheta();
  SendNValue();
}

void IPCInterface::SendSensorValues(){
  SendIRData();
  SendIRFilteredData();
  SendAccelerometerData();
  SendMicData();
  SendBatteryData();
}

void IPCInterface::SendCameraValues(){
  SendCameraResolution();
  SendBlobCenter();
}

// CAMERA VALUES
void IPCInterface::SendCameraResolution(){
  int DataArray[2];
  DataArray[0] = EmbeddedCamera->GetCameraWidth();
  DataArray[1] = EmbeddedCamera->GetCameraHeight();  
  ipc.SendData(IPC::CAMRESOLUTIONDATA, (uint8_t*)DataArray, sizeof(int)*2);     
}

void IPCInterface::SendBlobCenter(){
  int DataArray[2];
  DataArray[0] = EmbeddedCamera->GetBiggestBlobCenterX();
  DataArray[1] = EmbeddedCamera->GetBiggestBlobCenterY();
  ipc.SendData(IPC::BLOBCENTERDATA, (uint8_t*)DataArray, sizeof(int)*2);     
}

// ON-BOARD SENSOR VALUES
void IPCInterface::SendIRData(){
  std::vector<int> *DataVector = SPI->GetIRDataVector();
  int DataArray[DataVector->size()];
  copy(DataVector->begin(), DataVector->begin()+DataVector->size(), DataArray);
  ipc.SendData(IPC::IRDATA, (uint8_t*)DataArray, sizeof(int)*DataVector->size());  
}

void IPCInterface::SendIRFilteredData(){
  std::vector<int> *DataVector = SPI->GetIRFilteredDataVector();
  int DataArray[DataVector->size()];
  copy(DataVector->begin(), DataVector->begin()+DataVector->size(), DataArray);
  ipc.SendData(IPC::IRFILTEREDDATA, (uint8_t*)DataArray, sizeof(int)*DataVector->size());      
}

void IPCInterface::SendAccelerometerData(){
  std::vector<int> *DataVector = SPI->GetAccelerometerDataVector();
  int DataArray[DataVector->size()];
  copy(DataVector->begin(), DataVector->begin()+DataVector->size(), DataArray);
  ipc.SendData(IPC::ACCDATA, (uint8_t*)DataArray, sizeof(int)*DataVector->size());
}

void IPCInterface::SendMicData(){
  std::vector<int> *DataVector = SPI->GetMicDataVector();
  int DataArray[DataVector->size()];
  copy(DataVector->begin(), DataVector->begin()+DataVector->size(), DataArray);
  ipc.SendData(IPC::MICDATA, (uint8_t*)DataArray, sizeof(int)*DataVector->size()); 
}

void IPCInterface::SendBatteryData(){
  int *DataValue = SPI->GetBatteryLevel();
  ipc.SendData(IPC::BATTERYDATA, (uint8_t*)DataValue, sizeof(int));      
}

void IPCInterface::SendCurrentRunningAction(){
  int *DataValue = RobotPointer->GetCurrentActionRunning();
  ipc.SendData(IPC::CURRENTACTIONRUNNING, (uint8_t*)DataValue, sizeof(int));      
}

void IPCInterface::SendAmountOfFoodCollected(){
  int *DataValue = RobotPointer->GetAmountOfFoodCollected();
  ipc.SendData(IPC::FOODCOLLECTED, (uint8_t*)DataValue, sizeof(int));      
}

void IPCInterface::SendCurrentTheta(){
  double *DataValue = RobotPointer->GetCurrentTheta();
  ipc.SendData(IPC::CURRENTTHETA, (uint8_t*)DataValue, sizeof(double));      
}

void IPCInterface::SendNValue(){
  int *DataValue = RobotPointer->GetNValue();
  ipc.SendData(IPC::NVALUE, (uint8_t*)DataValue, sizeof(int));      
}

void IPCInterface::ReceiveCallback(const ELolMessage*msg, void *conn, void *ptr){

  IPCInterface *ClassPointer = (IPCInterface*)ptr;
 
  switch(msg->command)
    {
    case IPC::ROBOTPOSITION:
      {
	memcpy(ClassPointer->ROBOTPOSITION,msg->data,msg->length);
      }
      break;

    case IPC::AMOUNTOFFOODATNEST:
      {
	memcpy(ClassPointer->AmountOfFoodAtNest,msg->data,msg->length);
      }
      break;
      
    default:
      break;
    }
}

double IPCInterface::GetAmountOfFoodAtNest(){
  return AmountOfFoodAtNest[0];
}

int* IPCInterface::GetRobotPositionArray(){
  return ROBOTPOSITION;
}

int IPCInterface::GetRobotPositionX(){
  return ROBOTPOSITION[0];
}

int IPCInterface::GetRobotPositionY(){
  return ROBOTPOSITION[1];
}

int IPCInterface::GetRobotPositionA(){
  return ROBOTPOSITION[2];
}

IPCInterface::~IPCInterface(){}
