#ifndef CAMERA_H
#define CAMERA_H

#ifndef STRING_H
#include <string>
#endif

#ifndef PTHREAD_H
#include <pthread.h>
#endif

#ifndef CAPTURE_H
#include <capture.h>
#endif

#ifndef CMVISION_H
#include <cmvision.h>
#endif

#ifndef GLOBAL_H
#include <global.h>
#endif

#define PIXEL_FORMAT V4L2_PIX_FMT_UYVY
#define NUM_BLOB_CHANNELS               3
#define MAX_OBJECTS_TRACKED             3

extern int userQuit;

class Camera
{

 private :

  std::string VideoDevice;
  std::string ColorFile;

  int ColorID;
  CMVision::region* blob;
  CMVision::region BiggestBlob;

  uint64_t TimeStamp;

  int CamImgWidth;
  int CamImgHeight;

  Capture cap;
  CMVision vision;
  const Capture::Image *img;
  RawImageFrame frame;
  
  // Thread for Vision Processes
  pthread_t VisionThread;
  static pthread_mutex_t vision_mutex;
  static pthread_cond_t vision_cond;

 public :
   
  Camera(int CamImgWidth,int CamImgHeight);

  static void * BlobDetection(void *ptr);

  bool Init();
  bool CameraModuleLoad();
  bool CMVisionInitialization();
  void ThreadInitialization();

  int GetCameraWidth();
  int GetCameraHeight();


  void DisplayAllBlobs();
  CMVision::region GetBiggestBlob();
  int GetNumberOfBlobsDetected();
  int GetBiggestBlobArea();
  int GetBiggestBlobCenterX();
  int GetBiggestBlobCenterY();
  bool RobotDetected();
  bool NBoundryNRobot();

  ~Camera();
};
#endif
