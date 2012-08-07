//
//  Arc.h
//  Sketchy
//
//  Created by Josh Gargus on 8/5/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Geom_Arc_h
#define Sketchy_Geom_Arc_h

#include <cmath>
#include <vector>
#include <cassert>

#include <iostream>
using std::cerr;
using std::endl;

namespace Sketchy {
namespace Geom {

// TODO: move to it's own file
class Point2d {
 public:
    Point2d(const Point2d& p) : _x(p._x), _y(p._y) { }
    Point2d(float xx = 0.0f, float yy = 0.0f) : _x(xx), _y(yy) { }

    static Point2d polar(float radians, float radius) {
        return Point2d(radius * cos(radians),
                       radius * sin(radians));
    }

    static Point2d polar(float radians, float radius, const Point2d& origin) {
        return Point2d(radius * cos(radians) + origin._x,
                       radius * sin(radians) + origin._y);
    }

    Point2d operator+(const Point2d& p) const { return Point2d(_x + p._x, _y + p._y); }

    Point2d operator-(const Point2d& p) const { return Point2d(_x - p._x, _y - p._y); }

    float x() const { return _x; }
    float y() const { return _y; }

 private:
    float _x, _y;
};  // class Point2d


class Arc;
std::ostream& operator <<(std::ostream& os, const Arc& arc);

// TODO: make a Shape2D base class
class Arc {
 public:
    const float radius;
    const float start_radians, end_radians;
    const Point2d center;

    // Create an Arc.
    Arc(float r, float start_rads, float end_rads, Point2d cen = Point2d())
        : radius(r), start_radians(start_rads), end_radians(end_rads), center(cen) { }

    // Create an Arc starting at start_pos.  Just compute the center,
    // and delegate to the other constructor.
    Arc(Point2d start_pos, float r, float start_rads, float end_rads)
        : Arc(r, start_rads, end_rads, start_pos - Point2d::polar(start_rads, r)) { }

    // Total length of arc.
    float length() const {
        return M_PI * 2 * fabs(end_radians - start_radians) * radius;
    }

    // 2d cartesian position of start of arc.
    Point2d start() const { return Point2d::polar(start_radians, radius, center); }

    // 2d cartesian position of end of arc.
    Point2d end() const { return Point2d::polar(end_radians, radius, center); }

    // Create a list of connected Arcs that are joined with C1-continuity.
    // This implies that the end position/angle one arc are equal to the
    // start position/angle of the next.  Consequently, the center of an
    // Arc is uniquely determined by the radius of the Arc (which is given),
    // and the end position/angle of the previous arc.  For the first Arc,
    // the start position/angle are explicitly provided.
    static std::vector<Arc> piecewisePath(float start_radians,
                                          Point2d start_pos,
                                          const std::vector<float>& radii,
                                          const std::vector<float>& end_radians_list,
                                          bool prevent_reversal = true) {

        // Each arc must have a radius and an end-angle.
        assert(radii.size() == end_radians_list.size());

        // Accumulate results here.
        std::vector<Arc> path;

        // Iterate over lists of radii and angles, and accumulate Arcs.
        auto radius_iter = radii.begin();
        auto end_radians_iter = end_radians_list.begin();
        float end_radians;

        // See comment within the loop.
        bool adjusting = false;
        float previous_adjustment = 0.0f;

        while (end_radians_iter != end_radians_list.end()) {
            // Create arc and add it to the list.

            end_radians = *end_radians_iter;

            // If "prevent_reversal" is true, we want to adjust all Arcs except the very first.
            if (prevent_reversal && !adjusting) {
                adjusting = true;
                previous_adjustment = piecewisePathAdjust(start_radians, end_radians);
            } else if (adjusting) {
                // We adjust for changes in curvature-direction (eg: clockwise to widdershins).
                // This prevents the curve from doubling back upon itself.  If the curvature
                // of the previous arc matches this one, their adjustments cancel out.  Otherwise,
                // a half-rotation is either added (if we are now clockwise), or removed (widdershins).
                float next_adjustment = piecewisePathAdjust(start_radians, end_radians);
                start_radians += (next_adjustment - previous_adjustment);
                previous_adjustment = next_adjustment;
            }

            Arc arc(start_pos, *radius_iter, start_radians, end_radians);
            path.push_back(arc);

            // Prepare for next iteration
            start_pos = arc.end();
            start_radians = end_radians;

            ++end_radians_iter;
            ++radius_iter;
        }

        return path;
    }

 private:
    // Helper for piecewisePath().
    // Adjust by -pi/2 if clockwise, and pi/2 if widdershins.
    static inline float piecewisePathAdjust(float start_radians, float end_radians) {
        return (start_radians < end_radians) ? (M_PI / -2) : (M_PI / 2);
    }

};  // class Arc


template<class Vertex>
// TODO: return task instead of void.
//Task::TaskPtr
void tesselate(Vertex* vertices, int total_vertex_count,
                        const std::vector<Arc>& arcs,
                        float total_length, float length_offset = 0) {
    class Helper {
     public:
        // Helper function for writing pairs of vertices.
        static void emit(Vertex*& vertices,      // output buffer
                         float radius,           // radius of arc
                         const Point2d& center,  // center of arc
                         float radians,          // direction of current vertex from arc-center
                         float length,              // cumulative length along path
                         float normalFlip)       // manage clockwise vs. widdershins arcs
        {
            float nx = cos(radians);
            float ny = sin(radians);
            vertices[0].length = vertices[1].length = length;
            vertices[0].x = vertices[1].x = radius * nx + center.x();
            vertices[0].y = vertices[1].y = radius * ny + center.y();
            vertices[0].nx = nx * normalFlip;
            vertices[0].ny = ny * normalFlip;
            vertices[1].nx = -nx * normalFlip;
            vertices[1].ny = -ny * normalFlip;

            vertices += 2;
        }

        static float adjust(float start_radians, float end_radians) {
            return (start_radians < end_radians) ? (M_PI / -2) : (M_PI / 2);
        }
    };

    // Sanity checks.
    assert(total_vertex_count % 2 == 0);
    int generated_vertex_count = 0;  // will ensure that it matches total_vertex_count

    // Compute the desired space between vertices.
    // Divide by 2 because the tesselated arcs have width.
    // Subtract 1 because the # of intervals is one less than the # of interval boundaries.
    float spacing = total_length / static_cast<float>(total_vertex_count / 2 - 1);

    float overlap = 0.0f;

    for (auto a : arcs) {
        float len = a.length();
        int intervals = static_cast<int>(floor((len - overlap) / spacing));
        int vertex_count = intervals + 1;

        float arc_radians = a.end_radians - a.start_radians;
        float interval_radians = arc_radians * (spacing / len);

        float radians = a.start_radians + (overlap / len) * (a.end_radians - a.start_radians);

        // Resets every arc to maintain accuracy.
        // TODO: might be faster to initialize only once, but I doubt it.
        float cumulative_length = (generated_vertex_count / 2 - 1) * spacing + length_offset;

        float normal_flip = (arc_radians < 0) ? -1.0f : 1.0f;

        // Tesselate this arc.
        for (int i = 0; i <= intervals; i++) {
            Helper::emit(vertices, a.radius, a.center, radians,
                         cumulative_length, normal_flip);

            cumulative_length += spacing;
            radians += interval_radians;
        }

        // Compute the next overlap... the next vertex won't be precisely
        // at the beginning of the next arc, but rather offset by the amount
        // of a vertex-interval that didn't fit in the last arc-tesselation.
        overlap = (overlap + (intervals + 1) * spacing) - len;

        generated_vertex_count += vertex_count * 2;
    }

    // One last vertex.
    // TODO: explain why this is a sane choice (is it?)
    if (overlap < spacing / 5) {
        const Arc& a = arcs.back();
        Helper::emit(vertices, a.radius, a.center, a.end_radians,
                            total_length + length_offset,
                            ((a.end_radians - a.start_radians) < 0) ? -1.0f : 0.0f);
        generated_vertex_count += 2;
    }

    assert(generated_vertex_count == total_vertex_count);
}


template<class Vertex>
// TODO: return task instead of void
// Task::TaskPtr
void tesselate(Vertex* vertices, int vertex_count, const std::vector<Arc>& arcs) {
    float total_length = 0.0f;
    for (auto a : arcs) { total_length += a.length(); }
    return tesselate(vertices, vertex_count, arcs, total_length);
}


}  // namespace Geom
}  // namespace Sketchy

#endif  // Sketchy_Geom_Arc_h
