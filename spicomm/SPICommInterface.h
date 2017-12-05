#ifndef SPICOMMINTERFACE_H
#define SPICOMMINTERFACE_H

#ifndef PTHREAD_H
#include <pthread.h>
#endif

#ifndef VECTOR_H
#include <vector>
#endif

// Number of IR proximity sensors
#define NUM_IRS	8        
#define NUM_ACC	3        
#define NUM_MIC	3      
#define NUM_TAC 2  

class SPICommInterface
{

 private :
  
  // Sensor Data Vectors
  std::vector<int> IRSENSORSVECTOR;
  std::vector<int> IRSENSORSFILTEREDVECTOR;
  std::vector<int> ACCELEROMETERSVECTOR;
  std::vector<int> MICROPHONESVECTOR;
  std::vector<int> TACVECTOR;
  // Battery Level
  int BatteryLevel;

  // Control Variable for SPI Communication
  int spi_device;

  // Threads for SPI Communication
  pthread_t spi_thread;
  static pthread_mutex_t spi_mutex;
  static pthread_cond_t spi_cond;

 public :
   
  SPICommInterface();

  // SPICommunication Callback
  static void * SPICommCallback(void *ptr);

  // SPI Initilization and thread creation
  bool Init();

  // Thread Related Functions
  void LockMutex();
  void UnLockMutex();
  
  // Get Sensor Data 
  std::vector<int>* GetTACDataVector();
  std::vector<int>* GetIRDataVector();
  std::vector<int>* GetIRFilteredDataVector();
  std::vector<int>* GetAccelerometerDataVector();
  std::vector<int>* GetMicDataVector();
  int* GetBatteryLevel();

  // Epuck Hardware related functions
  void SetSpeeds(int LeftSpeed,int RightSpeed);
  void SetLED(int LEDID, int State);
  void SetLEDs(int LEDsID, int State);
  void BlinkLED(int LEDID, int Cycles);
  void BlinkLEDs(int LEDID, int Cycles);
  void BlinkAlLEDs(bool State);
  void SetIRPulse(int IRID, int State);
  void SetIRPulses(int IRsID, int State);
  void PlaySound(int SoundID);


  // SPIComm Managment Functions
  void Run();  
  void Reset();
  void Update(int FileDescriptor);
  
  ~SPICommInterface();
};
#endif
