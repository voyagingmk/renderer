#ifndef RENDERER_CURVE_HPP
#define RENDERER_CURVE_HPP

#include "base.hpp"
#include "com/geometry.hpp"

namespace renderer {

	class Spline {
		std::vector<float> knots, weights;
		std::vector<Point2dF> points;
		std::vector<std::vector<float>> p;
		int order, ctrlPointsNum;
		int d = 2; //2D
	public:
		void setKnots(std::vector<float>& param_knots) {
			knots = param_knots;
		}

		void setWeights(std::vector<float>& param_weights) {
			weights = param_weights;
		}

		void setCtrlPoints(std::vector<Point2dF>& param_points) {
			points = param_points;
			ctrlPointsNum = points.size();
			p.resize(ctrlPointsNum);
		}

		void setOrder(int param_order) {
			order = param_order;
		}

		void initSplineParam(){
			int n = ctrlPointsNum;
			//////////////////////////////////WEIGHTS
			if (weights.empty()) {
				weights.resize(n);
				for (int i = 0; i<n; i++) {
					weights[i] = 1;
				}
			}
			//////////////////////////////////P
			// convert points to homogeneous coordinates (NURBS)
			p.resize(n);
			for (int i = 0; i < n; i++) {
				p[i].resize(d + 1);
			}
			/*
			p = [
			[ points[0].x,	 points[0].y,	weight[0] ],
			[ points[1].x,	 points[1].y,	weight[1] ],
			[ points[n-1].x, points[n-1].y,	weight[n-1] ],
			]
			*/
			//////////////////////////////////KNOTS
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
		}

		void interpolate(Point2dF* result, float t)
		{
			int n = ctrlPointsNum;
			if (order < 2) throw "order must be at least 2 (linear)";
			if (order > n) throw "order must be less than point count";

            auto k = knots.size() - 1 - (order - 1); // = n

			int domain[] = { order - 1, static_cast<int>(k) };

			// remap t to the domain where the spline is defined
			float low = knots[domain[0]];
			float high = knots[domain[1]];
			t = t * (high - low) + low;

			if (t < low || t > high) {
				//t must be in [0,1]
				throw "out of bounds";
			}

			int s;
			for (s = domain[0]; s<domain[1]; s++) {
				if (t >= knots[s] && t <= knots[s + 1]) {
					break;
				}
			}
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < d; j++) {
					p[i][j] = points[i][j] * weights[i];
				}
				p[i][d] = weights[i];
			}
			// l (level) goes from 1 to the curve order
			for (int l = 1; l <= order; l++) {
				// build level l of the pyramid
				for (int i = s; i>s - order + l; i--) {
					float a = (t - knots[i]) / (knots[i + order - l] - knots[i]);
					// interpolate each component
					for (int j = 0; j<d + 1; j++) {
						p[i][j] = (1 - a) * p[i - 1][j] + a * p[i][j];
					}
				}
			}
			for (int i = 0; i<d; i++) {
				(*result)[i] = p[s][i] / p[s][d];
			}
		}

	};
};


#endif
