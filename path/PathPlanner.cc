
#ifndef IPCInterface_H
#include <IPCInterface.h>
#endif

#ifndef IOSTREAM_H
#include <iostream>
#endif

#ifndef MATH_H
#include <math.h>
#endif

#include <PathPlanner.h>

#define PI 3.14159265

PathPlanner::PathPlanner(IPCInterface *IPCROSVICON){
  this->IPCROSVICON = IPCROSVICON;

  NestPositionX = 0;
  NestPositionY = 0;
  NestMargin = 10;
  NestRadiusInMts = 0.25;    
}

PathPlanner::~PathPlanner(){}

/** Returns true if the Robot is 10 cm away from the arena boundaries */
bool PathPlanner::RobotIsOutOfBoundaries(){
  double DistanceInXAxis =   fabs(((double)IPCROSVICON->GetRobotPositionX()/10000));
  double DistanceInYAxis =   fabs(((double)IPCROSVICON->GetRobotPositionY()/10000));

  if(DistanceInXAxis >= 0.85 || DistanceInYAxis >= 0.75)
    return true;
  else
    return false;
}


/** Returns true if the Robot is within the Nest */
bool PathPlanner::RobotIsWithinNest(){
  double CurrentDistanceToNest = CalculateDistanceToNestInMeters();

  if(CurrentDistanceToNest<=NestRadiusInMts && CurrentDistanceToNest>0)
    return true;
  else
    return false;
}

double PathPlanner::CalculateDistanceToNestInMeters(){

  double DistanceXInMeters =   fabs(NestPositionX - ((double)IPCROSVICON->GetRobotPositionX()/10000));
  double DistanceYInMeters =   fabs(NestPositionY - ((double)IPCROSVICON->GetRobotPositionY()/10000));
  double DistanceToNestInMeters = sqrt(pow(DistanceXInMeters,2) + pow(DistanceYInMeters,2));

  return DistanceToNestInMeters;
}

/** Returns true if the Nest is at the robot's front given a certain
    margin */
bool PathPlanner::NestIsInFront(){
   if(fabs(GetAngleFromRobotToNest())<NestMargin)
    return true;
  else
    return false;
}

/** Returns true if the Nest is at the robot's back given a certain
    margin */
bool PathPlanner::NestIsBehind(){
  if(fabs(GetAngleFromRobotToNest())>(180-NestMargin))
    return true;
  else
    return false;
}

double PathPlanner::GetAngleFromRobotToNest(){
  CalculateCurrentTracjectoryVector();
  CalculateRobotToNestVector();
  return CalculateAngleFromRobotToNest();
}  

double PathPlanner::CalculateAngleFromRobotToNest(){
  double Dividend = ((CurrentRobotTrajectoryVectorX*RobotToNestVectorX) + 
		     (CurrentRobotTrajectoryVectorY*RobotToNestVectorY));
    
  double Divisor = (sqrt(pow(CurrentRobotTrajectoryVectorX,2)+pow(CurrentRobotTrajectoryVectorY,2)) * 
		    sqrt(pow(RobotToNestVectorX,2)+pow(RobotToNestVectorY,2)));  
  
  return (acos(Dividend/Divisor) * 180 / PI);
}

void PathPlanner::CalculateCurrentTracjectoryVector(){
  double CurrentAngleinRadians = (double)IPCROSVICON->GetRobotPositionA() / 1000;
  CurrentRobotTrajectoryVectorX = cos(CurrentAngleinRadians);
  CurrentRobotTrajectoryVectorY = sin(CurrentAngleinRadians);
}

void PathPlanner::CalculateRobotToNestVector(){
  RobotToNestVectorX = NestPositionX - (double)IPCROSVICON->GetRobotPositionX();
  RobotToNestVectorY = NestPositionY - (double)IPCROSVICON->GetRobotPositionY();
  printf("x: %f\ty:%f\n", RobotToNestVectorX, RobotToNestVectorY);
}

