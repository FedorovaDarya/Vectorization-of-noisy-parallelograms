﻿
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <cmath>

using namespace cv;
using namespace std;

class Vectorization {

private:
    double rhoStep,
            thetaStep,
            interval;

public:

    Vectorization(double rhoStepInit, double thetaStepInit, double intervalInit) {
        rhoStep = rhoStepInit;
        thetaStep = thetaStepInit;
        interval = intervalInit * rhoStepInit;
    }

    void parallelogram(vector<pair<float, float>> *inputPoints, vector<Vec3d> *outputLines) {

        Mat lines;
        vector<Point2f> pointVector;

        double rhoApproximation = 0.0;
        double distance;

        for (auto &inputPoint : *inputPoints) {
            pointVector.push_back(Point2f(inputPoint.first, inputPoint.second));

            distance = hypot(inputPoint.first, inputPoint.second);
            rhoApproximation = rhoApproximation < distance ? distance : rhoApproximation;
        }

        double rhoMin = -rhoApproximation * 1.2, rhoMax = rhoApproximation * 1.2;
        double thetaMin = 0.0, thetaMax = CV_PI;
        HoughLinesPointSet(pointVector, lines, 400, 1,
                           rhoMin, rhoMax, rhoStep,
                           thetaMin, thetaMax, thetaStep);

        lines.copyTo(*outputLines);
    }

    void fourLeaders(vector<Vec3d> *houghOutput, vector<pair<double, double>> *leaders) {

        double rhoInterval = rhoStep * 30, thetaInterval = thetaStep * 600;
        double deltaRho, deltaTheta;

        leaders->push_back(make_pair(houghOutput->at(0).val[1], houghOutput->at(0).val[2]));
        int i = 1;
        bool inBunch;
        while (leaders->size() < 4 && i < houghOutput->size()) {
            inBunch = false;
            for (auto &leader : *leaders) {
                deltaRho = abs(houghOutput->at(i).val[1] - leader.first);
                deltaTheta = abs(houghOutput->at(i).val[2] - leader.second);
                if (deltaRho <= rhoInterval && deltaTheta <= thetaInterval) {
                    inBunch = true;
                    break;
                }
            }
            if (!inBunch) {
                leaders->push_back(make_pair(houghOutput->at(i).val[1], houghOutput->at(i).val[2]));

            }
            i++;
        }

        double k, b;
        for (i = 0; i < 4; i++) {
            k = -cos(leaders->at(i).second) / sin(leaders->at(i).second); // k = -cos(theta) / sin(theta)
            b = leaders->at(i).first / sin(leaders->at(i).second); // b = r / sin(theta)
            leaders->at(i) = make_pair(k, b);
        }

    }

    void pointDistribution(vector<pair<double, double>> *leaders, vector<pair<float, float>> *points,
                           vector<vector<pair<double, double>>> *pointsSet) {

        // Note: you should tie points to leaders from bottom to top only if you confirm that 4 leaders are actually different

        double distance, x, y, k, b;
        for (auto &point : *points) {
            for (int j = 3, used = 0; j > -1 && used < 2; j--) {
                x = point.first;
                y = point.second;
                k = leaders->at(j).first;
                b = leaders->at(j).second;
                distance = sqrt(pow((x + k * y - k * b) / (k * k + 1) - x, 2) +
                                pow(k * (x + k * y - k * b) / (k * k + 1) + b - y, 2));
                if (distance <= interval) {
                    pointsSet->at(j).push_back(point);
                    used++;
                }
            }
        }
    }

};


