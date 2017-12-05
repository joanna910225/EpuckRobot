#ifndef PATHPLANNER_H
#define PATHPLANNER_H

class IPCInterface;

class PathPlanner
{
 public :

  PathPlanner(IPCInterface *IPCROSVICON);
  ~PathPlanner();

  bool RobotIsOutOfBoundaries();
  bool RobotIsWithinNest();
  bool NestIsInFront();
  bool NestIsBehind();
  double GetAngleFromRobotToNest();
  double CalculateDistanceToNestInMeters();
  double CalculateAngleFromRobotToNest();
  void CalculateCurrentTracjectoryVector();
  void CalculateRobotToNestVector();

 private :

  IPCInterface *IPCROSVICON;

  double NestMargin;
  double NestRadiusInMts;

  double CurrentRobotTrajectoryVectorX;
  double CurrentRobotTrajectoryVectorY;
  
  double RobotToNestVectorX;
  double RobotToNestVectorY;
  
  double NestPositionX;
  double NestPositionY;
};

#endif
