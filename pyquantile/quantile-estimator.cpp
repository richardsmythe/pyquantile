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

		// only adjust if there's enough space and if adjustment is needed
		if ((delta >= 1 && posNext > posCurrent + 1) || (delta <= -1 && posPrev < posCurrent - 1)) {

			// if delta is + we move +1 or -1 if not
			delta = std::copysign(1.0, delta);
			double parabolicValue = CalculateParabolicValue(markerPrev, markerCurrent, markerNext, posPrev, posCurrent, posNext, delta);
			double newValue;
			if (markerPrev < parabolicValue && parabolicValue < markerNext) {
				newValue = parabolicValue;
			}
			else {  // fall back to linear  
				double t = delta > 0 ?
					delta / (posNext - posCurrent) :
					delta / (posCurrent - posPrev);
				newValue = markerCurrent + delta * (delta > 0 ?
					(markerNext - markerCurrent) / (posNext - posCurrent) :
					(markerCurrent - markerPrev) / (posCurrent - posPrev));
			}

			switch (i) {
			case 1:
				pos1 += delta;
				Q1 = std::max(Q0, std::min(Q2, newValue));
				break;
			case 2:
				pos2 += delta;
				Q2 = std::max(Q1, std::min(Q3, newValue));
				break;
			case 3:
				pos3 += delta;
				Q3 = std::max(Q2, std::min(Q4, newValue));
				break;
			}
		}
	}
}

double QuantileEstimator::CalculateParabolicValue(
	double markerPrev, double markerCurrent, double markerNext,
	double posPrev, double posCurrent, double posNext, double d) {

	// get position differences
	double d0 = posCurrent - posPrev;
	double d1 = posNext - posCurrent;

	// protects against small number division
	if (d0 <= EPSILON || d1 <= EPSILON) {
		return markerCurrent;
	}

	double term1 = d * (markerNext - markerCurrent) / d1;
	double term2 = d * (markerCurrent - markerPrev) / d0;
	double estimate = markerCurrent + (term1 + term2) / 2;

	if (estimate < markerPrev || estimate > markerNext) {
		double slope = d > 0 ?
			(markerNext - markerCurrent) / d1 :
			(markerCurrent - markerPrev) / d0;

		estimate = markerCurrent + d * slope;
	}

	return std::max(markerPrev, std::min(markerNext, estimate));
}

void QuantileEstimator::UpdateMarkers(double s) {
	if (s < Q0) {
		Q0 = s;
		pos1++; pos2++; pos3++; pos4++;
	}
	else if (s < Q1) {
		pos1++; pos2++; pos3++; pos4++;
	}
	else if (s < Q2) {
		pos2++; pos3++; pos4++;
	}
	else if (s < Q3) {
		pos3++; pos4++;
	}
	else if (s < Q4) {
		pos4++;
	}
	else {
		Q4 = s;
	}

	// give bias correction for high quantiles
	double n = static_cast<double>(N);
	desPos0 += inc0;
	if (p > 0.8) {
		double bias = (p - 0.8) / 0.2;
		desPos1 += inc1 * (1.0 + 0.5 * bias);
		desPos2 += inc2 * (1.0 + 0.3 * bias);
		desPos3 += inc3 * (1.0 + 0.2 * bias);
	}
	else {
		desPos1 += inc1;
		desPos2 += inc2;
		desPos3 += inc3;
	}
	desPos4 += inc4;

	for (int i = 1; i <= 4; i++) {
		double* pos = nullptr;
		double* desPos = nullptr;

		switch (i) {
		case 1: pos = &pos1; desPos = &desPos1; break;
		case 2: pos = &pos2; desPos = &desPos2; break;
		case 3: pos = &pos3; desPos = &desPos3; break;
		case 4: pos = &pos4; desPos = &desPos4; break;
		}

		double delta = *desPos - *pos;
		if (std::abs(delta) > 1) {
			// need more careful position adjustment for extreme quantiles
			// for high quantiles, must be more conservative with higher markers
			if (p > 0.8 && i >= 3) {
				*pos += std::copysign(0.5, delta);
			}
			else {
				*pos += std::copysign(1.0, delta);
			}
		}
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