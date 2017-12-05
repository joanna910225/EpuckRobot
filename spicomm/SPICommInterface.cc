

#ifndef IOSTREAM_H
#include <iostream>
#endif

#ifndef SPICOMM_H
#include <spicomm.h>
#endif

#include <SPICommInterface.h>

pthread_mutex_t SPICommInterface::spi_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t SPICommInterface::spi_cond = PTHREAD_COND_INITIALIZER;

using namespace std;

/** 
    Class Constructor
 */
SPICommInterface::SPICommInterface(){

}

/** 
    Continues with the thread process
    unless a signal condition is met
 */
void SPICommInterface::Run(){
  LockMutex();
  pthread_cond_signal( &spi_cond );
  UnLockMutex();
}

/** 
    Locks the Mutex for the running thread
 */
void SPICommInterface::LockMutex(){
  pthread_mutex_lock(&spi_mutex);
}

/** 
    Unlocks the Mutex for the running thread
 */
void SPICommInterface::UnLockMutex(){
  pthread_mutex_unlock(&spi_mutex);
}

/** 
    Initializes SPI Interface as well as 
    creating the SPI thread
 */
bool SPICommInterface::Init(){

  spi_device = init_spi();

  if(spi_device == -1)
    {
      cout << "Fail to initialise SPI interface, exit" << endl;
      return false;
    }

  // Thread creation for SPIComm 
  pthread_create(&spi_thread, NULL, SPICommCallback, this);
}

/** Robot's communication callback
    \param ptr Void pointer for pthread compliance */
void * SPICommInterface::SPICommCallback(void * ptr){
  cout << "SPIComm is running" << endl;
  
  SPICommInterface *SPI = (SPICommInterface*)ptr;

  while(1){
    SPI->LockMutex();
    pthread_cond_wait( &spi_cond, &spi_mutex );
    SPI->Update(SPI->spi_device);

    /*SPI->GetIRDataVector();
    SPI->GetIRFilteredDataVector();
    SPI->GetAccelerometerDataVector();
    SPI->GetMicDataVector();*/
    SPI->UnLockMutex();
  }
    
  SPI->Update(SPI->spi_device);
  cout << "SPIComm is exiting" << endl;
  return NULL;
}



/** 
    Returns TAC Sensor Data Vector (step in wheels)
 */
vector<int>* SPICommInterface::GetTACDataVector(){  
  int TACDATA[NUM_TAC];
  get_tac_data(TACDATA);
  TACVECTOR.assign(TACDATA,TACDATA + NUM_TAC);
  return &TACVECTOR;
}


/** 
    Returns IR Data Vector
 */
vector<int>* SPICommInterface::GetIRDataVector(){  
  int IRSENSORS[NUM_IRS];
  get_ir_data(IRSENSORS,NUM_IRS);
  //get_proximity_filtered_data(IRSENSORS,NUM_IRS);
  //cout << "IRSENSORS" << IRSENSORS[0] << " " << IRSENSORS[1] <<endl;
  IRSENSORSVECTOR.assign(IRSENSORS,IRSENSORS + NUM_IRS);
  return &IRSENSORSVECTOR;
}

/**
    Returns IR Sensor Filtered (Signals at 32Hz) Data Vector
 */
vector<int>* SPICommInterface::GetIRFilteredDataVector(){
  int IRSENSORSFILTERED[NUM_IRS];
  get_proximity_filtered_data(IRSENSORSFILTERED,NUM_IRS);

  IRSENSORSFILTEREDVECTOR.assign(IRSENSORSFILTERED,IRSENSORSFILTERED + NUM_IRS);
  return &IRSENSORSFILTEREDVECTOR;
}

/**
    Returns Accelerometer Sensor Data Vector
 */
vector<int>* SPICommInterface::GetAccelerometerDataVector(){
  int ACCELEROMETERS[NUM_ACC];
  get_acc_data(ACCELEROMETERS,NUM_ACC);
  
  ACCELEROMETERSVECTOR.assign(ACCELEROMETERS,ACCELEROMETERS + NUM_ACC);
  return &ACCELEROMETERSVECTOR;
}



/**
    Returns Mic Sensor Data Vector
    in ???? units
*/
vector<int>* SPICommInterface::GetMicDataVector(){
  int MICROPHONES[NUM_MIC];
  get_mic_data(MICROPHONES,NUM_MIC);

  MICROPHONESVECTOR.assign(MICROPHONES,MICROPHONES + NUM_MIC);
  return &MICROPHONESVECTOR;
}

/** 
    Returns Current Battery Level
    int ???? units
*/
int* SPICommInterface::GetBatteryLevel(){  
  BatteryLevel = get_battery();
  return &BatteryLevel;
}

/** Differential Wheels Speed Assignment  
    \param lspeed Left Wheel Speed 
    \param rspeed Right Wheel Speed tacl;     // steps made on l/r motors
        int16_t tacr;
        int16_t batt;     // battery level
*/
void SPICommInterface::SetSpeeds(int LeftSpeed,int RightSpeed){  
  LockMutex();
  setSpeed(LeftSpeed,RightSpeed);
  UnLockMutex();
}

/** Switches ON or OFF a specific LED
    \param LEDID LED ID number
    \param State boolean switch*/
void SPICommInterface::SetLED(int LEDID, int State){  
  LockMutex();
  setLED(LEDID,State);
  UnLockMutex();
}

/** Switches ON/OFF a list of LEDs
    \param LEDsID LED's list ID numbers
    \param State boolean switch*/
void SPICommInterface::SetLEDs(int LEDsID, int State){  
  setLEDs(LEDsID,State);
}

/** Blinks a specific LED
    \param LEDID LED ID number
    \param Cycle Number of cycles*/
void SPICommInterface::BlinkLED(int LEDID, int Cycles){  
  BlinkLED(LEDID,Cycles);
}

/** Blinks a list of LEDs
    \param LEDsID LED's list ID numbers
    \param Cycle Number of cycles*/
void SPICommInterface::BlinkLEDs(int LEDsID, int Cycles){  
  BlinkLEDs(LEDsID,Cycles);
}

/** Robot Blinks ON/OFF all the LEDS
    \param State boolean switch*/
void SPICommInterface::BlinkAlLEDs(bool State){  
  LockMutex();
  if(State)
    BlinkLEDs(0xFF,5000);
  else
    BlinkLEDs(0xFF,0);
  UnLockMutex();
}

void SPICommInterface::SetIRPulse(int IRID, int State){  
  LockMutex();
  setIRPulse(IRID,State);
  UnLockMutex();
}

void SPICommInterface::SetIRPulses(int IRsID, int State){  
  setIRPulses(IRsID,State);
}

/** 
    Plays a sound through the robot's speaker 
 */

void SPICommInterface::PlaySound(int SoundID){  
  playSound(SoundID);
}

/** 
    Sends Commands and gets data from dsPIC 
 */
void SPICommInterface::Update(int FileDescriptor){  
  update(FileDescriptor);
}

/** 
    Resets Robot Communication
 */
void SPICommInterface::Reset(){
  reset_robot(spi_device);
}

/** 
    Class Destructor
 */
SPICommInterface::~SPICommInterface(){}
