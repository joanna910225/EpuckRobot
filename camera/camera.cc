
#ifndef IOSTREAM_H
#include <iostream>
#endif

#include <camera.h>

using namespace std;

pthread_mutex_t Camera::vision_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Camera::vision_cond =PTHREAD_COND_INITIALIZER;

/** Embedded Camera Class for Epuck Robot 
 \param CamImgWidth Width for Camera's resolution
 \param CamImgHeight Height for Camera's resolution*/
Camera::Camera(int CamImgWidth,int CamImgHeight){

  VideoDevice = "/dev/video1";
  ColorFile = "colors.txt";

  // Channel ID
  ColorID = 1;

  // Initialize Camera Image Parameters
  this->CamImgWidth=CamImgWidth;
  this->CamImgHeight=CamImgHeight;

  // Prepare the Image Frame
  frame.hdr.type = RawImageFrame::ImageTypeRawYUV;
  frame.hdr.width = CamImgWidth;
  frame.hdr.height = CamImgHeight;
}

/** Robot's blobdetection (through embedded Camera) mechanism
    \param ptr Void pointer for pthread compliance */
void * Camera::BlobDetection(void *ptr){
  
  cout << "Blob detection is running" << endl;

  Camera *CameraPointer = (Camera*)ptr;
  static int count = 0;

  timeval starttime, sys_time;
  gettimeofday(&starttime, NULL);

  while(userQuit!=1){    
    pthread_mutex_lock(&vision_mutex);
    CameraPointer->img = CameraPointer->cap.captureFrame();

    if(CameraPointer->img !=NULL){
	CameraPointer->frame.hdr.timestamp = CameraPointer->img->timestamp;
	CameraPointer->frame.data = CameraPointer->img->data;
	CameraPointer->vision.processFrame(reinterpret_cast<image_pixel*>(CameraPointer->img->data));	
      }

    CameraPointer->cap.releaseFrame(CameraPointer->img);
    gettimeofday(&sys_time, NULL);

    CameraPointer->blob = CameraPointer->vision.getRegions(CameraPointer->ColorID);
    CameraPointer->BiggestBlob = CameraPointer->GetBiggestBlob();
    pthread_mutex_unlock(&vision_mutex);
  }
  
  cout << "Blob detection is terminating" << endl;
  return NULL;
}

/** Outputs all found blobs for current Color */
void Camera::DisplayAllBlobs(){
  for(int i=0;i<vision.numRegions(ColorID);i++)
    {
      cout <<\
	" Color : " << blob->color <<		\
	" Region Number : " << i <<		\
	" Center : " <<			\
	" X - " << blob->cen_x <<		\
	" Y - " << blob->cen_y <<		\
	" Area : " << blob->area <<		\
	endl;
	
      blob = blob->next;
    }
}

/** Outputs the biggest blob for the current color */
CMVision::region Camera::GetBiggestBlob(){
 
  CMVision::region CurrentBiggestBlob;

  CurrentBiggestBlob.area = 0;
  CurrentBiggestBlob.cen_x = 0;
  CurrentBiggestBlob.cen_y = 0;
  
    for(int i=0;i<GetNumberOfBlobsDetected();i++)
      {
	if(CurrentBiggestBlob.area<blob->area)
	    CurrentBiggestBlob=*blob;
	
	blob=blob->next;
      }
  return CurrentBiggestBlob;
}

/** Outputs the total amount of blobs for the given color */
int Camera::GetNumberOfBlobsDetected(){
  return vision.numRegions(ColorID);
}

/** Outputs if robot encouters the boundry*/
bool Camera::RobotDetected(){
  if(int num = vision.numRegions(0) > 0){
    printf("%d\n",num);
    return true;
  }
  else{
    printf("%d\n",num);
    return false;
  }
}

bool Camera::NBoundryNRobot(){
 if(int num = vision.numRegions(1) > 0){
    printf("%d\n",num);
    return true;
  }
  else{
    printf("%d\n",num);
    return false;
  }
}
/** Outputs the area of the biggest blob */
int Camera::GetBiggestBlobArea(){
  return BiggestBlob.area;
}
/** Outputs the X component for the center of the Biggest Blob*/
int Camera::GetBiggestBlobCenterX(){
  return BiggestBlob.cen_x;
}

/** Outputs the Y component for the center of the Biggest Blob*/
int Camera::GetBiggestBlobCenterY(){
  return BiggestBlob.cen_y;
}

/** Initialization routines for Epuck's Camera */
bool Camera::Init(){
  CameraModuleLoad();
  CMVisionInitialization();
  ThreadInitialization();
}

/** Camera's Module Loading */
bool Camera::CameraModuleLoad(){
  int trial = 0;

  //Module Loading 
  while(!cap.init(VideoDevice.c_str(),1,CamImgWidth,CamImgHeight,PIXEL_FORMAT)){
    cout << "Error initializing Camera" << endl;
    system("modprobe -r poX030");
    system("modprobe -r atmel_isi");
    system("modprobe  poX030");

    if(trial++ >=3){
      printf("no success to load the Camera driver, quit the program\n");
      return false;
    }
  }
}

/** CMvision Object Initilization */
bool Camera::CMVisionInitialization(){

  // Image size Initialization
  if(!vision.initialize(CamImgWidth, CamImgHeight)){
    cout << "Error initializing vision" << endl;
    return false;
  }
  // ColorFile Loading
  if(!vision.loadOptions(ColorFile.c_str())){
    cout << "Error loading color file" << endl;
    return false;
  }
}

/** Camera's Thread Initialization */
void Camera::ThreadInitialization(){
  pthread_create(&VisionThread, NULL, BlobDetection, this);
}

/** Retrieves Camera's width resolution */
int Camera::GetCameraWidth(){
  return CamImgWidth;
}

/** Retrieves Camera's height resolution */
int Camera::GetCameraHeight(){
  return CamImgHeight;
}

/** Camera's Class Destructor */
Camera::~Camera()
{}
