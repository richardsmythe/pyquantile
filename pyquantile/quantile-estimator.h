#pragma once
#include <vector>
#include <tuple>
#include <cmath>

class QuantileEstimator {
public:
    explicit QuantileEstimator(double p);
    void Add(double s);
    double GetQuantile() const;
    std::tuple<double, double, double, double, double> GetCurrentMarkers() const;

private:
    void SetIncrements(double p);
    void UpdateMarkers(double s);
    void AdjustMarkers();

    double CalculateParabolicValue(double markerPrev, double markerCurrent, double markerNext,double posPrev, double posCurrent, double posNext, double d);
    double CalculateLinearValue(double markerPrev, double markerCurrent, double markerNext,double posPrev, double posCurrent, double posNext, double d);

    int N = 0;
    double p;
    std::vector<double> initialValues;
    double Q0 = 0.0, Q1 = 0.0, Q2 = 0.0, Q3 = 0.0, Q4 = 0.0;
    double pos0 = 1.0, pos1 = 2.0, pos2 = 3.0, pos3 = 4.0, pos4 = 5.0;
    double desPos0 = 1.0, desPos1 = 2.0, desPos2 = 3.0, desPos3 = 4.0, desPos4 = 5.0;
    double inc0 = 0.0, inc1 = 0.0, inc2 = 0.0, inc3 = 0.0, inc4 = 0.0;

    // N-Dependent constant used as a limit to transition from linear to parabolic
    const double N_THRESHOLD = 500.0; 
};