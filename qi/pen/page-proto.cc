// TODO: this is a work-in-progress file to prove that I can serialize/deserialize protos.
// This isn't currently added to the XCode project.

#include "qi/pen/cubicbezier.h"
#include "qi/pen/protos/page.pb.h"

void SetSegmentValues(proto::CubicBezier2d* out, CubicBezier2f bez) {
  out->set_x0(bez.pts[0].x);
  out->set_y0(bez.pts[0].y);
  out->set_x1(bez.pts[1].x);
  out->set_y1(bez.pts[1].y);
  out->set_x2(bez.pts[2].x);
  out->set_y2(bez.pts[2].y);
  out->set_x3(bez.pts[3].x);
  out->set_y3(bez.pts[3].y);
}

void SetSegmentValues(CubicBezier2f* out, const proto::CubicBezier2d& bez) {
  out->pts[0].x = bez.x0();
  out->pts[0].y = bez.y0();
  out->pts[1].x = bez.x1();
  out->pts[1].y = bez.y1();
  out->pts[2].x = bez.x2();
  out->pts[2].y = bez.y2();
  out->pts[3].x = bez.x3();
  out->pts[3].y = bez.y3();
}

void DoProtoTest() {
  CubicBezier2f bez1;
  CubicBezier2f bez2;
  CubicBezier2f bez3;

  bez1.pts[0].x = 0.0f;
  bez1.pts[0].y = 0.0f;
  bez1.pts[1].x = 1.0f;
  bez1.pts[1].y = 1.0f;
  bez1.pts[2].x = 2.0f;
  bez1.pts[2].y = 2.0f;
  bez1.pts[3].x = 3.0f;
  bez1.pts[3].y = 3.0f;
  bez2.pts[0].x = 3.0f;
  bez2.pts[0].y = 3.0f;
  bez2.pts[1].x = 4.0f;
  bez2.pts[1].y = 4.0f;
  bez2.pts[2].x = 5.0f;
  bez2.pts[2].y = 5.0f;
  bez2.pts[3].x = 6.0f;
  bez2.pts[3].y = 6.0f;
  bez3.pts[0].x = 6.0f;
  bez3.pts[0].y = 6.0f;
  bez3.pts[1].x = 7.0f;
  bez3.pts[1].y = 7.0f;
  bez3.pts[2].x = 8.0f;
  bez3.pts[2].y = 8.0f;
  bez3.pts[3].x = 9.0f;
  bez3.pts[3].y = 9.0f;

  proto::Page* page = new proto::Page;
  proto::Stroke* stroke1 = page->add_strokes();
  SetSegmentValues(stroke1->add_segments(), bez1);
  SetSegmentValues(stroke1->add_segments(), bez2);
  SetSegmentValues(stroke1->add_segments(), bez3);
  proto::Stroke* stroke2 = page->add_strokes();
  SetSegmentValues(stroke2->add_segments(), bez3);
  SetSegmentValues(stroke2->add_segments(), bez2);
  SetSegmentValues(stroke2->add_segments(), bez1);

  bool result;
  std::string serialized;
  result = page->SerializeToString(&serialized);
  std::cerr << "serialization was " << (result ? "successful" : "unsuccessful") << std::endl;

  proto::Page* page2 = new proto::Page;
  result = page2->ParseFromString(serialized);
  std::cerr << "deserialization was " << (result ? "successful" : "unsuccessful") << std::endl;

  ASSERT(page2->strokes_size() == 2);
  ASSERT(page2->strokes(0).segments_size() == 3);
  ASSERT(page2->strokes(1).segments_size() == 3);

  CubicBezier2f out1, out2;
  SetSegmentValues(&out1, page2->strokes(0).segments(0));
  SetSegmentValues(&out2, page2->strokes(1).segments(0));
  ASSERT(out1 == bez1);
  ASSERT(out2 == bez3);
  SetSegmentValues(&out1, page2->strokes(0).segments(2));
  SetSegmentValues(&out2, page2->strokes(1).segments(2));
  ASSERT(out1 == bez3);
  ASSERT(out2 == bez1);

  // TODO(jjosh): deletion
}
