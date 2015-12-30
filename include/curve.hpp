#ifndef RENDERER_CURVE_HPP
#define RENDERER_CURVE_HPP

#include "base.hpp"
#include "geometry.hpp"
#include <vector>

namespace renderer {

	void interpolate(float t, int order,  std::vector<Point2dF> points, std::vector<float> knots, std::vector<float> weights, Point2dF* result)
	{

		int n = points.size();
		int d = 2;

		if (order < 2) throw "order must be at least 2 (linear)";
		if (order > n) throw "order must be less than point count";

		if (weights.empty()) {
			weights.resize(n);
			for (int i = 0; i<n; i++) {
				weights[i] = 1;
			}
		}

		if (knots.empty()) {
			// build knot vector
			knots.resize(n + order);
			for (int i = 0; i<n + order; i++) {
				knots[i] = i;
			}
		}
		else {
			if (knots.size() != n + order) throw "bad knot vector length";
		}

		int domain[] = {
			order - 1,
			knots.size() - 1 - (order - 1)
		};

		// remap t to the domain where the spline is defined
		int low = knots[domain[0]];
		int high = knots[domain[1]];
		t = t * (high - low) + low;

		if (t < low || t > high) throw "out of bounds";

		for (int s = domain[0]; s<domain[1]; s++) {
			if (t >= knots[s] && t <= knots[s + 1]) {
				break;
			}
		}

		// convert points to homogeneous coordinates
		int v = new Array(n);
		for (int i = 0; i<n; i++) {
			v[i] = new Array(d + 1);
			for (int j = 0; j<d; j++) {
				v[i][j] = points[i][j] * weights[i];
			}
			v[i][d] = weights[i];
		}

		// l (level) goes from 1 to the curve order
		for (int l = 1; l <= order; l++) {
			// build level l of the pyramid
			for (int i = s; i>s - order + l; i--) {
				int a = (t - knots[i]) / (knots[i + order - l] - knots[i]);

				// interpolate each component
				for (int j = 0; j<d + 1; j++) {
					v[i][j] = (1 - a) * v[i - 1][j] + a * v[i][j];
				}
			}
		}

		for (int i = 0; i<d; i++) {
			result[i] = v[s][i] / v[s][d];
		}
	}
};