@0xecd9ae512afce161;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("qi::pen::proto");

struct CubicBezier2d {
  x0 @0 : Float64;
  x1 @1 : Float64;
  x2 @2 : Float64;
  x3 @3 : Float64;
  y0 @4 : Float64;
  y1 @5 : Float64;
  y2 @6 : Float64;
  y3 @7 : Float64;
}

struct Stroke {
  segments @0 : List(CubicBezier2d);
}

struct Page {
  strokes @0 : List(Stroke);
}
