#include "quantile-estimator.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <tuple>

QuantileEstimator::QuantileEstimator(double p)
	: p(p) {
	if (p < 0 || p > 1) throw std::invalid_argument("p must be between 0 and 1.");
	SetIncrements(p);
}

void QuantileEstimator::SetIncrements(double p) {
	desPos0 = pos0;
	desPos1 = pos1;
	desPos2 = pos2;
	desPos3 = pos3;
	desPos4 = pos4;
	inc0 = 0;
	inc1 = p / 2;
	inc2 = p;
	inc3 = (1 + p) / 2;
	inc4 = 1;	
}

void QuantileEstimator::SortMarkers() {
	std::vector<double> markers = { Q0, Q1, Q2, Q3, Q4 };
	std::sort(markers.begin(), markers.end());
	Q0 = markers[0];
	Q1 = markers[1];
	Q2 = markers[2];
	Q3 = markers[3];
	Q4 = markers[4];
}

void QuantileEstimator::ValidateMarkerPositions() {
	double oldPos1 = pos1, oldPos2 = pos2, oldPos3 = pos3, oldPos4 = pos4;
	pos1 = std::max(pos1, pos0 + 1);
	pos2 = std::max(pos2, pos1 + 1);
	pos3 = std::max(pos3, pos2 + 1);
	pos4 = std::max(pos4, pos3 + 1);
	
}

void QuantileEstimator::AdjustMarkers() {
    for (int i = 1; i < 4; i++) {
        double delta = 0;
        switch (i) {
        case 1: delta = desPos1 - pos1; break;
        case 2: delta = desPos2 - pos2; break;
        case 3: delta = desPos3 - pos3; break;
        }

        double markerPrev, markerCurrent, markerNext;
        double posPrev, posCurrent, posNext;
        switch (i) {
        case 1:
            markerPrev = Q0; markerCurrent = Q1; markerNext = Q2;
            posPrev = pos0; posCurrent = pos1; posNext = pos2;
            break;
        case 2:
            markerPrev = Q1; markerCurrent = Q2; markerNext = Q3;
            posPrev = pos1; posCurrent = pos2; posNext = pos3;
            break;
        case 3:
            markerPrev = Q2; markerCurrent = Q3; markerNext = Q4;
            posPrev = pos2; posCurrent = pos3; posNext = pos4;
            break;
        }

        if ((delta >= 1 && posNext > posCurrent + 1) ||
            (delta <= -1 && posPrev < posCurrent - 1)) {
            delta = std::copysign(1.0, delta);
            double parabolicValue = CalculateParabolicValue(
                markerPrev, markerCurrent, markerNext,
                posPrev, posCurrent, posNext, delta);
            double newValue;
            if (markerPrev < parabolicValue && parabolicValue < markerNext) {
                newValue = parabolicValue;
            }
            else {
                newValue = markerCurrent + delta * (markerNext - markerCurrent) / (posNext - posCurrent);
            }

            switch (i) {
            case 1:
                pos1 += delta;
                Q1 = newValue;
                break;
            case 2:
                pos2 += delta;
                Q2 = newValue;
                break;
            case 3:
                pos3 += delta;
                Q3 = newValue;
                break;
            }
        }
    }
}


double QuantileEstimator::CalculateParabolicValue(
    double markerPrev, double markerCurrent, double markerNext,
    double posPrev, double posCurrent, double posNext, double d) {
    double forwardSlopeComponent = (markerNext - markerCurrent) / (posNext - posCurrent) * (posCurrent - posPrev + d);
    double backwardSlopeComponent = (markerCurrent - markerPrev) / (posCurrent - posPrev) * (posNext - posCurrent - d);
    double parabolicEstimate = markerCurrent + d / (posNext - posPrev) * (forwardSlopeComponent + backwardSlopeComponent);
    return parabolicEstimate;
}

void QuantileEstimator::UpdateMarkers(double s) {
	bool markerValueChanged = false;
	double prevQ0 = Q0, prevQ1 = Q1, prevQ2 = Q2, prevQ3 = Q3, prevQ4 = Q4;

	if (s < Q0) {
		Q0 = s;
		pos1++;
		pos2++;
		pos3++;
		pos4++;
		markerValueChanged = true;
	}
	else if (s < Q1) {
		pos1++;
		pos2++;
		pos3++;
		pos4++;
	}
	else if (s < Q2) {
		pos2++;
		pos3++;
		pos4++;
	}
	else if (s < Q3) {
		pos3++;
		pos4++;
	}
	else if (s < Q4) {
		pos4++;
	}
	else {
		Q4 = s;
		markerValueChanged = true;
	}

	desPos0 += inc0;
	desPos1 += inc1;
	desPos2 += inc2;
	desPos3 += inc3;
	desPos4 += inc4;

	if (std::abs(desPos1 - pos1) > 1) {
		if (desPos1 - pos1 > 0)
			pos1 += 1;
		else
			pos1 -= 1;
	}
	if (std::abs(desPos2 - pos2) > 1) {
		if (desPos2 - pos2 > 0)
			pos2 += 1;
		else
			pos2 -= 1;
	}
	if (std::abs(desPos3 - pos3) > 1) {
		if (desPos3 - pos3 > 0)
			pos3 += 1;
		else
			pos3 -= 1;
	}
	if (std::abs(desPos4 - pos4) > 1) {
		if (desPos4 - pos4 > 0)
			pos4 += 1;
		else
			pos4 -= 1;
	}

	ValidateMarkerPositions();	
	AdjustMarkers();
}

void QuantileEstimator::Add(double s) {
	N++;
	if (N <= 5) {
		switch (N) {
		case 1: Q0 = s; break;
		case 2: Q1 = s; break;
		case 3: Q2 = s; break;
		case 4: Q3 = s; break;
		case 5:
			Q4 = s;
			SortMarkers();
			break;
		}
		return;
	}
	UpdateMarkers(s);
}

double QuantileEstimator::GetQuantile() const {
	if (p <= 0.0) return Q0;
	if (p >= 1.0) return Q4;
	if (p <= 0.25) {
		double t = 4 * p;
		return Q0 + t * (Q1 - Q0);
	}
	else if (p <= 0.5) {
		double t = 4 * (p - 0.25);
		return Q1 + t * (Q2 - Q1);
	}
	else if (p <= 0.75) {
		double t = 4 * (p - 0.5);
		return Q2 + t * (Q3 - Q2);
	}
	else {
		double t = 4 * (p - 0.75);
		return Q3 + t * (Q4 - Q3);
	}
}

std::tuple<double, double, double, double, double> QuantileEstimator::GetCurrentMarkers() const {
	return { Q0, Q1, Q2, Q3, Q4 };
}