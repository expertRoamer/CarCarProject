#ifndef MOTORSYSTEM_H
#define MOTORSYSTEM_H

void driveHard(double, double);

void driveKinematic(double speed, double turn);

void drive(double, double);

void driveFor(double, double, int);

void driveLeft(double);

void driveRight(double);

void driveIndividual(int, int, int, double);

void back(int, int, int, int , int, int);

bool startPID(double);

void stop(int, int);

#endif // MOTORSYSTEM_H