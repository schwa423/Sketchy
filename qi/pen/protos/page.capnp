@0xecd9ae512afce161;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("qi::page::proto");

struct CubicBez {
  x0 @0 : Float32;
  x1 @1 : Float32;
  x2 @2 : Float32;
  x3 @3 : Float32;
  y0 @4 : Float32;
  y1 @5 : Float32;
  y2 @6 : Float32;
  y3 @7 : Float32;
}

# TODO: consider giving each stroke a scale/origin
# struct CoordinateSystem {
#   origin @0 : Float64;
#   scale @1 : Float32;
# }

struct Stroke {
  id @0 : UInt64;
  segments @1 : List(CubicBez);
}

struct Page {
  strokes @0 : List(Stroke);
}
