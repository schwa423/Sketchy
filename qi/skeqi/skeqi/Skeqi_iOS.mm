//
//  Skeqi_iOS.mm
//  skeqi
//
//  Created by Josh Gargus on 5/15/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import "Skeqi_iOS.h"

// Interface extensions that can't be exposed to Swift because they use C++.
#import "QiControllerDelegateImpl.h"

#import "qi/disk/sqlitecache.h"
#import "qi/gfx/port_ios/buffer_ios.h"
#import "qi/gfx/port_ios/device_ios.h"
#import "qi/pen/cubicbezier.h"
#import "qi/ui/touchhandler.h"

#import "skeqi-Swift-Wrapper.h"

// TODO: remove <iostream>
#import <iostream>
#import <map>
#import <unordered_set>

#import <Metal/Metal.h>

// TODO: remember to move this when Page serialization moves.
//#include "page.capnp.h"
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <iostream>

// TODO: move to its own file, and document
namespace qi {
namespace gfx {
namespace port {

class RenderCommandEncoder_iOS {
 public:
  RenderCommandEncoder_iOS(id<MTLRenderCommandEncoder> encoder)
      : encoder_(encoder) {}

  void SetVertexBuffer(Buffer* buffer, int offset, int index);
  void DrawTriangleStrip(int vertex_start, int vertex_count);
  void DrawLineStrip(int vertex_start, int vertex_count);
  void DrawPoints(int vertex_start, int vertex_count);

 private:
  id<MTLRenderCommandEncoder> encoder_;
};

void RenderCommandEncoder_iOS::SetVertexBuffer(
    Buffer* buffer, int offset, int index) {
  ASSERT(dynamic_cast<Buffer_iOS*>(buffer));
  id<MTLBuffer> mtl_buffer = static_cast<Buffer_iOS*>(buffer)->GetBuffer();
  [encoder_ setVertexBuffer:mtl_buffer offset:offset atIndex:index];
}

void RenderCommandEncoder_iOS::DrawTriangleStrip(
    int vertex_start, int vertex_count) {
  [encoder_ drawPrimitives:MTLPrimitiveTypeTriangleStrip
            vertexStart:vertex_start vertexCount:vertex_count];
}

void RenderCommandEncoder_iOS::DrawLineStrip(
    int vertex_start, int vertex_count) {
  [encoder_ drawPrimitives:MTLPrimitiveTypeLineStrip
            vertexStart:vertex_start vertexCount:vertex_count];
}

void RenderCommandEncoder_iOS::DrawPoints(
    int vertex_start, int vertex_count) {
  [encoder_ drawPrimitives:MTLPrimitiveTypePoint
            vertexStart:vertex_start vertexCount:vertex_count];
}

}  // namespace port

#if defined(QI_PLATFORM_IOS)
typedef port::RenderCommandEncoder_iOS RenderCommandEncoder;
#else
#error
#endif

}  // namespace gfx
}  // namespace qi



namespace qi {
namespace page {

class Page2;

class Stroke2 {
 public:
  typedef std::vector<CubicBezier2f> Path;

  void Finalize();
  void SetPath(Path&& path);
  void SetSamplePoints(const std::vector<Pt2f>& points);

 private:
  Stroke2(Page2* page)
      : page_(page), vertex_count_(0), offset_(0), finalized_(false) {}

  void EncodeDrawCalls(gfx::RenderCommandEncoder* encoder);

  // If |encoder| is not nil, tesselate the stroke using a compute shader.
  // Otherwise, do it on the CPU.
  void Tesselate(id<MTLComputeCommandEncoder>encoder);

  struct Vertex {
    float px, py, pz, pw;
    float nx, ny, length;
    unsigned char cr, cg, cb, ca;
  };

  Path path_;
  Page2* page_;
  shared_ptr<gfx::Buffer> buffer_;
  int vertex_count_;
  int offset_;
  std::atomic_bool finalized_;

  friend class Page2;
};

typedef shared_ptr<Stroke2> Stroke2Ptr;


class Page2 {
 public:
  Page2(shared_ptr<gfx::Device> device, id<MTLLibrary> library);

  Stroke2Ptr NewStroke() {
    // TODO: can't use make_shared because constructor is private... what is best idiom?
    Stroke2Ptr stroke(new Stroke2(this));
    strokes_.push_back(stroke);
    return stroke;
  }

  void DeleteStroke(const Stroke2Ptr& stroke) {
    auto it = std::find(strokes_.begin(), strokes_.end(), stroke);
    ASSERT(it != strokes_.end());

    deleted_strokes_1_.push_back(stroke);
    strokes_.erase(it);
    dirty_strokes_.erase(stroke.get());
  }

  // Compute the number of vertices to use to tesselate each Stoke path segment.
  std::vector<size_t> ComputeVertexCounts(Stroke2::Path& path);

  // TODO: use a qi::gfx::RenderCommandEncoder.
  void EncodeDrawCalls(id<MTLRenderCommandEncoder> encoder);

  // TODO: perhaps put all strokes into one shared Buffer.
  void FinalizeStroke(Stroke2* stroke) {}

  shared_ptr<gfx::Buffer> NewBuffer(size_t length) { return device_->NewBuffer(length); }

  // Pre-render update.
  void Update(id<MTLCommandQueue>queue);

 private:
  void SetupRenderPipeline(id<MTLLibrary> library);
  void SetupComputePipeline(id<MTLLibrary> library);

  shared_ptr<gfx::Device> device_;
  id<MTLRenderPipelineState> render_pipeline_;
  id<MTLComputePipelineState> compute_pipeline_;
  std::vector<Stroke2Ptr> strokes_;
  std::unordered_set<Stroke2*> dirty_strokes_;

  // Hack to keep deleted Strokes alive until they're finished rendering.
  std::vector<Stroke2Ptr> deleted_strokes_1_;
  std::vector<Stroke2Ptr> deleted_strokes_2_;
  std::vector<Stroke2Ptr> deleted_strokes_3_;

  friend class Stroke2;
};


// Stroke method implementations //////////////////////////////////////////////

void Stroke2::Finalize() {
  bool was_finalized = finalized_.exchange(true);
  if (!was_finalized) {
    page_->FinalizeStroke(this);

    if (!path_.empty()) {
      std::cerr << "STROKE (" << path_.size() << " segments):" << std::endl;
      for (int i = 0; i < path_.size(); ++i) {
        std::cerr << "       " << path_[i] << std::endl;
      }
    }

/*
    // Verify that we can serialize/deserialize Cap'n Proto messages.
    // TODO: Super hacky!
    typedef page::proto::Page PageCapnp;
    typedef page::proto::Stroke StrokeCapnp;
    typedef page::proto::CubicBezier2d BezCapnp;

    capnp::MallocMessageBuilder message;
    StrokeCapnp::Builder stroke = message.initRoot<StrokeCapnp>();

    ::capnp::List<BezCapnp>::Builder segments =
        stroke.initSegments(static_cast<unsigned int>(path_.size()));
    for (int i = 0; i < path_.size(); ++i) {
      BezCapnp::Builder seg = segments[i];
      seg.setX0(path_[i].pts[0].x);
      seg.setX1(path_[i].pts[1].x);
      seg.setX2(path_[i].pts[2].x);
      seg.setX3(path_[i].pts[3].x);
      seg.setY0(path_[i].pts[0].y);
      seg.setY1(path_[i].pts[1].y);
      seg.setY2(path_[i].pts[2].y);
      seg.setY3(path_[i].pts[3].y);
    }

    int fildes[2];
    int status = pipe(fildes);
    ASSERT(status != -1);

    capnp::writePackedMessageToFd(fildes[1], message);

    ::capnp::PackedFdMessageReader message2(fildes[0]);

    StrokeCapnp::Reader stroke2 = message2.getRoot<StrokeCapnp>();
    ASSERT(stroke2.getSegments().size() == path_.size());
    for (int i = 0; i < path_.size(); ++i) {
      BezCapnp::Reader bez = stroke2.getSegments()[i];
      ASSERT(bez.getX0() == path_[i].pts[0].x);
      ASSERT(bez.getX1() == path_[i].pts[1].x);
      ASSERT(bez.getX2() == path_[i].pts[2].x);
      ASSERT(bez.getX3() == path_[i].pts[3].x);
      ASSERT(bez.getY0() == path_[i].pts[0].y);
      ASSERT(bez.getY1() == path_[i].pts[1].y);
      ASSERT(bez.getY2() == path_[i].pts[2].y);
      ASSERT(bez.getY3() == path_[i].pts[3].y);
    }

    close(fildes[0]);
    close(fildes[1]);
*/
  }
}

// TODO: pass "Path" instead of "Path&&"?
void Stroke2::SetPath(Path&& path) {
  ASSERT(!finalized_);
  path_ = move(path);
  page_->dirty_strokes_.insert(this);
}

void Stroke2::SetSamplePoints(const std::vector<Pt2f>& points) {
  ASSERT(!finalized_);
  path_.clear();
  size_t buffer_size = points.size() * sizeof(Vertex);
  if (!buffer_ || buffer_->GetLength() < buffer_size) {
    buffer_ = page_->NewBuffer(buffer_size * 1.2 + 200);
  }
  auto verts = reinterpret_cast<Stroke2::Vertex*>(buffer_->GetContents());
  for (int i = 0; i < points.size(); i++) {
    verts[i].px = points[i].x;
    verts[i].py = points[i].y;
    verts[i].pz = 0.0;
    verts[i].pw = 1.0;
    verts[i].nx = 0.0f;
    verts[i].ny = 0.0f;
    verts[i].cr = 255;
    verts[i].cb = 0;
    verts[i].cg = 0;
    verts[i].ca = 255;
  }
  vertex_count_ = points.size();
}

void Stroke2::EncodeDrawCalls(gfx::RenderCommandEncoder* encoder) {
  if (vertex_count_ > 0) {
    encoder->SetVertexBuffer(buffer_.get(), offset_, 0);
    if (path_.empty()) {
      encoder->DrawPoints(0, vertex_count_);
    } else {
      encoder->DrawTriangleStrip(0, vertex_count_);
    }
  }
}

void Stroke2::Tesselate(id<MTLComputeCommandEncoder>encoder) {
  if (path_.empty()) return;

  std::vector<size_t> vertex_counts = page_->ComputeVertexCounts(path_);
  size_t total_vertex_count = 0;
  for (size_t count : vertex_counts) {
    ASSERT(count == (count / 2) * 2);
    total_vertex_count += count;
  }

  // TODO multithreading: don't set buffer_ and vertex_count_ immediately.
  if (!buffer_ || buffer_->GetLength() < total_vertex_count * sizeof(Vertex)) {
    // Allocate a new Buffer with plenty of room to grow.
    // Note: +2  for final segment.
    int buffer_size = (total_vertex_count + 200 + 2) * sizeof(Vertex);
    buffer_ = page_->NewBuffer(buffer_size);
  }
  vertex_count_ = total_vertex_count;

  if (encoder != nil) {
    // Tesselate stroke on GPU using compute shader.
    int offset = 0;
    for (int i = 0; i < path_.size(); ++i) {
      auto bez = path_[i];
      [encoder setBytes:&bez length:sizeof(bez) atIndex:0];

      float t_divisor = vertex_counts[i] / 2.0f - 1.0f;
      [encoder setBytes:&t_divisor length:sizeof(float) atIndex:1];

      // TODO: avoid static_cast<>
      auto buf = static_cast<gfx::port::Buffer_iOS*>(buffer_.get());
      [encoder setBuffer:buf->GetBuffer() offset:offset atIndex:2];

      MTLSize threadgroup_size = MTLSizeMake(vertex_counts[i] / 2, 1, 1);
      MTLSize threadgroups = MTLSizeMake(1, 1, 1);
      [encoder dispatchThreadgroups:threadgroups
               threadsPerThreadgroup: threadgroup_size];

      offset += vertex_counts[i] * sizeof(Stroke2::Vertex);
    }
    return;
  }

  // Use CPU to generate vertices for each Path segment.
  auto verts = reinterpret_cast<Stroke2::Vertex*>(buffer_->GetContents());
  for (int i = 0; i < path_.size(); ++i) {
    CubicBezier2f bez = path_[i];
    unsigned char rgb = (255.0 * i) / path_.size();

    int seg_vert_count = vertex_counts[i];

    // On all segments but the last, we don't want the Bezier parameter to reach
    // 1.0, because this would evaluate to the same thing as a parameter of 0.0 on
    // the next segment.
    float param_incr = (i == path_.size() - 1) ?
      1.0 / (seg_vert_count - 2) :
      1.0 / seg_vert_count;

    for (int i = 0; i < seg_vert_count; i += 2) {
      // We increment index by 2 each loop iteration, so the last iteration will have
      // "index == kVertexCount - 2", and therefore a parameter value of "i * incr == 1.0".
      std::pair<Pt2f, Pt2f> pt = EvaluatePointAndNormal(bez, i * param_incr);
      verts[i].px = verts[i+1].px = pt.first.x;
      verts[i].py = verts[i+1].py = pt.first.y;
      verts[i].pz = verts[i+1].pz = 0.0;
      verts[i].pw = verts[i+1].pw = 1.0;
      verts[i].nx = pt.second.x;
      verts[i].ny = pt.second.y;
      verts[i+1].nx = -pt.second.x;
      verts[i+1].ny = -pt.second.y;

      // TODO length
      verts[i].length = verts[i+1].length = 0.0;
      verts[i].cb = verts[i+1].cb = rgb;
      verts[i].cg = verts[i+1].cg = rgb;
      verts[i].cr = verts[i+1].cr = rgb;
      verts[i].ca = verts[i+1].ca = 255;
    }
    // Bump Stroke2::Vertex pointer to tesselate the next segment.
    verts += seg_vert_count;
  }
}


// Page method implementations ////////////////////////////////////////////////

Page2::Page2(shared_ptr<gfx::Device> device, id<MTLLibrary> library) : device_(device) {
  SetupRenderPipeline(library);
  SetupComputePipeline(library);
}

void Page2::SetupRenderPipeline(id<MTLLibrary> library) {
  auto vert_func = [library newFunctionWithName:@"strokeVertex"];
  auto frag_func = [library newFunctionWithName:@"strokeFragmentPassThrough"];

  auto pipeline_desc = [[MTLRenderPipelineDescriptor alloc] init];
  pipeline_desc.vertexFunction = vert_func;
  pipeline_desc.fragmentFunction = frag_func;
  // TODO: needed if set elsewhere earlier?
  pipeline_desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

  NSError *errors = nil;
  render_pipeline_ = [library.device newRenderPipelineStateWithDescriptor:pipeline_desc error:&errors];
  ASSERT(render_pipeline_ && !errors);
}

void Page2::SetupComputePipeline(id<MTLLibrary> library) {
  auto kernel_func = [library newFunctionWithName:@"strokeBezierTesselate"];
  NSError *errors = nil;
  compute_pipeline_ =[library.device newComputePipelineStateWithFunction:kernel_func error:&errors];
  ASSERT(compute_pipeline_ && !errors);
}

std::vector<size_t> Page2::ComputeVertexCounts(Stroke2::Path& path) {
  std::vector<size_t> counts;
  counts.reserve(path.size());
  for (CubicBezier2f bez : path) {
    // TODO: compute a number based on the length of each path segment.
    static const size_t kVertexCount = 32;
    counts.push_back(kVertexCount);
  }
  return counts;
}

void Page2::EncodeDrawCalls(id<MTLRenderCommandEncoder> mtl_encoder) {
  [mtl_encoder pushDebugGroup:@"Render Page Strokes"];
  [mtl_encoder setRenderPipelineState:render_pipeline_];
  gfx::port::RenderCommandEncoder_iOS encoder(mtl_encoder);
  for (auto& stroke : strokes_) {
    stroke->EncodeDrawCalls(&encoder);
  }
  [mtl_encoder popDebugGroup];

  // Move "2" into "3", then "1" into "2", then clear "1".
  std::swap(deleted_strokes_2_, deleted_strokes_3_);
  std::swap(deleted_strokes_1_, deleted_strokes_2_);
  deleted_strokes_1_.clear();
}

void Page2::Update(id<MTLCommandQueue>queue) {
  if (dirty_strokes_.empty()) return;

  id<MTLCommandBuffer> cmd_buffer = [queue commandBuffer];
  id<MTLComputeCommandEncoder> cmd_encoder = [cmd_buffer computeCommandEncoder];
  [cmd_encoder pushDebugGroup:@"Tesselate Bezier Strokes"];
  [cmd_encoder setComputePipelineState:compute_pipeline_];

  for (auto stroke : dirty_strokes_) {
    stroke->Tesselate(cmd_encoder);
//    stroke->Tesselate(nil);
  }
  dirty_strokes_.clear();

  [cmd_encoder popDebugGroup];
  [cmd_encoder endEncoding];
  [cmd_buffer commit];
}

class SkeqiStrokeFitter {
 public:
  // TODO: revisit error_threshold_
  SkeqiStrokeFitter(std::shared_ptr<Page2> page)
      : fitter_id(s_next_fitter_id++), page_(page), error_threshold_(0.0004) {}

  void StartStroke(Pt2f pt) {
    stroke_ = page_->NewStroke();
    stroke2_ = page_->NewStroke();
    points_.push_back(pt);
    params_.push_back(0.0);
  }

  void AddSamplePoint(Pt2f pt) {
    float dist = distance(pt, points_.back());
    // TODO: epison constant
    if (dist < 0.000004) return;
    points_.push_back(pt);
    params_.push_back(params_.back() + dist);

    // Recursively compute a list of cubic Bezier segments.
    // TODO: don't recompute stable path segments near the beginning of the stroke.
    size_t end_index = points_.size() - 1;
    Pt2f left_tangent = points_[1] - points_[0];
    Pt2f right_tangent = points_[end_index-1] - points_[end_index];
    FitSampleRange(0, end_index, left_tangent, right_tangent);

    // Compute length of each cubic Bezier segment,
    // TODO: unfinished
//    std::vector<CubicBezier2f>
    for (auto bez : path_) {

//      std::deque

    }

    // TODO: remove... this is just basic sanity-check for Split() given that
    // I don't have unit tests running.
    Stroke2::Path split_path;
    for (auto bez : path_) {
      auto split = bez.Split(0.5);
      split_path.push_back(split.first);
      split_path.push_back(split.second);
    }
    path_ = move(split_path);

    // For each of the segments computed above, compute the total segment length
    // and a arc-length parameterization.  This parameterization is a 1-D cubic
    // Bezier such that an input parameter t in the range [0,1] results in a
    // new parameter t' (also in [0,1]) such that evaluating the original
    // curve-segment at t' returns the point on the segment where the cumulative
    // arc-length to that point is t * total_segment_length.
    ASSERT(!path_.empty());
    stroke_->SetPath(move(path_));
    stroke2_->SetSamplePoints(points_);
  }

  void FinishStroke() {
    stroke_->Finalize();
    stroke2_->Finalize();
    stroke_.reset();
    stroke2_.reset();
    points_.clear();
    params_.clear();
  }

  void CancelStroke() {
    page_->DeleteStroke(stroke_);
    page_->DeleteStroke(stroke2_);
    stroke_.reset();
    stroke2_.reset();
    points_.clear();
    params_.clear();
  }

  // Identify the stroke fitter.
  const int64 fitter_id;

 private:
  void FitSampleRange(
      int start_index, int end_index, Pt2f left_tangent, Pt2f right_tangent);

  shared_ptr<Page2> page_;
  Stroke2Ptr stroke_;
  Stroke2Ptr stroke2_;

  std::vector<Pt2f> points_;
  std::vector<float> params_;
  float error_threshold_;
  Stroke2::Path path_;

  static int64 s_next_fitter_id;
};

int64 SkeqiStrokeFitter::s_next_fitter_id = 1;


// TODO: investigate reparameterization
void SkeqiStrokeFitter::FitSampleRange(
    int start_index, int end_index, Pt2f left_tangent, Pt2f right_tangent) {
  ASSERT(end_index > start_index);
  if (end_index - start_index == 1) {
    // Only two points... use a heuristic.
    // TODO: Double-check this heuristic (perhaps normalization needed?)
    // TODO: Perhaps this segment can be omitted entirely (perhaps blending
    //       endpoints of the adjacent segments.
    CubicBezier2f line;
    line.pts[0] = points_[start_index];
    line.pts[3] = points_[end_index];
    line.pts[1] = line.pts[0] + (left_tangent * 0.25f);
    line.pts[2] = line.pts[3] + (right_tangent * 0.25f);
    ASSERT(!std::isnan(line.pts[0].x));
    ASSERT(!std::isnan(line.pts[0].y));
    ASSERT(!std::isnan(line.pts[1].x));
    ASSERT(!std::isnan(line.pts[1].y));
    ASSERT(!std::isnan(line.pts[2].x));
    ASSERT(!std::isnan(line.pts[2].y));
    ASSERT(!std::isnan(line.pts[3].x));
    ASSERT(!std::isnan(line.pts[3].y));
    path_.push_back(line);
    return;
  }

  // Normalize cumulative length between 0.0 and 1.0.
  float param_shift = -params_[start_index];
  float param_scale = 1.0 / (params_[end_index] + param_shift);

  CubicBezier2f bez = FitCubicBezier2f(
      &(points_[start_index]), end_index - start_index + 1,
      &(params_[start_index]), param_shift, param_scale,
      left_tangent, right_tangent);

  int split_index = (end_index + start_index + 1) / 2;
  float max_error = 0.0;
  for (int i = start_index; i <= end_index; ++i) {
    float t = (params_[i] + param_shift) * param_scale;
    Pt2f diff = points_[i] - bez.Evaluate(t);
    float error = dot(diff, diff);
    if (error > max_error) {
      max_error = error;
      split_index = i;
    }
  }

  // The current fit is good enough... add it to the path and stop recursion.
  if (max_error < error_threshold_) {
    ASSERT(!std::isnan(bez.pts[0].x));
    ASSERT(!std::isnan(bez.pts[0].y));
    ASSERT(!std::isnan(bez.pts[1].x));
    ASSERT(!std::isnan(bez.pts[1].y));
    ASSERT(!std::isnan(bez.pts[2].x));
    ASSERT(!std::isnan(bez.pts[2].y));
    ASSERT(!std::isnan(bez.pts[3].x));
    ASSERT(!std::isnan(bez.pts[3].y));
    path_.push_back(bez);
    return;
  }

  // Error is too large... split into two ranges and fit each.
  ASSERT(split_index > start_index && split_index < end_index);
  Pt2f middle_tangent = points_[split_index + 1] - points_[split_index - 1];
  FitSampleRange(start_index, split_index, left_tangent, middle_tangent * -1.f);
  FitSampleRange(split_index, end_index, middle_tangent, right_tangent);
}


class SkeqiTouchHandler : public ui::TouchHandler {
 public:
  SkeqiTouchHandler(shared_ptr<Page2> page) : page_(page) {}

  void TouchesBegan(const std::vector<ui::Touch>* touches) override {
    // If there are 5 simultanous touches, run tests.
    // TODO: some other way to invoke tests.
    if (fitters_.size() == 4) {
      for (auto& fitter : fitters_) {
        fitter.second->CancelStroke();
      }
      fitters_.clear();
      Qi::RunAllTests();
      return;
    }

    for (auto& touch : *touches) {
      ASSERT(fitters_.find(touch) == fitters_.end());
      auto& fitter = (fitters_[touch] = make_unique<SkeqiStrokeFitter>(page_));
      fitter->StartStroke(GetTouchPosition(touch));
    }
  }

  void TouchesCancelled(const std::vector<ui::Touch>* touches) override {
    for (auto& touch : *touches) {
      auto it = fitters_.find(touch);
      if (it != fitters_.end()) {
        it->second->CancelStroke();
        fitters_.erase(it);
      }
    }
  }

  void TouchesMoved(const std::vector<ui::Touch>* touches) override {
    for (auto& touch : *touches) {
      auto it = fitters_.find(touch);
      if (it == fitters_.end()) {
        // TODO: Used for running tests; see TouchesBegan().
        continue;
      }
      ASSERT(it != fitters_.end());
      it->second->AddSamplePoint(GetTouchPosition(touch));
    }
  }

  void TouchesEnded(const std::vector<ui::Touch>* touches) override {
    for (auto& touch : *touches) {
      auto it = fitters_.find(touch);
      if (it == fitters_.end()) {
        // TODO: Used for running tests; see TouchesBegan().
        continue;
      }
      ASSERT(it != fitters_.end());
      it->second->AddSamplePoint(GetTouchPosition(touch));
      it->second->FinishStroke();
      fitters_.erase(it);
    }
  }

 private:
  Pt2f GetTouchPosition(ui::Touch touch) {
    return Pt2f{static_cast<float>(touch.x),static_cast<float>(touch.y)};
  }

  shared_ptr<Page2> page_;
  std::map<ui::Touch, unique_ptr<SkeqiStrokeFitter>> fitters_;
};


}  // namespace page
}  // namespace qi

/*
#import "CocoaHTTPServer/HTTPServer.h"
#import "CocoaHTTPServer/HTTPConnection.h"
#import "CocoaHTTPServer/HTTPLogging.h"
#import "CocoaLumberjack/DDLog.h"
#import "CocoaLumberjack/DDTTYLogger.h"

// Log levels: off, error, warn, info, verbose
static const int ddLogLevel = LOG_LEVEL_VERBOSE;

// Log levels: off, error, warn, info, verbose
// Other flags: trace
static const int httpLogLevel = HTTP_LOG_LEVEL_WARN; // | HTTP_LOG_FLAG_TRACE;
*/

@implementation Skeqi_iOS {
  std::shared_ptr<qi::page::Page2> page_;
  std::shared_ptr<qi::disk::Cache> cache_;
// TODO(jjosh):  HTTPServer *httpServer;
}

- (id)init {
  self = [super init];
  if (self) {
    page_ = std::make_shared<qi::page::Page2>(self.device, self.metalLibrary);
    cache_ = std::make_shared<qi::disk::SqliteCache>(":memory:");
    auto touch_handler = std::make_unique<qi::page::SkeqiTouchHandler>(page_);
    [self setTouchHandler: move(touch_handler)];

/*
    // Configure our logging framework.
    // To keep things simple and fast, we're just going to log to the Xcode console.
    [DDLog addLogger:[DDTTYLogger sharedInstance]];

    // Initalize our http server
    httpServer = [[HTTPServer alloc] init];

    // Tell the server to broadcast its presence via Bonjour.
    // This allows browsers such as Safari to automatically discover our service.
    //  [httpServer setType:@"_http._tcp."];

    // Note: Clicking the bonjour service in Safari won't work because Safari will use http and not https.
    // Just change the url to https for proper access.

    // Normally there's no need to run our server on any specific port.
    // Technologies like Bonjour allow clients to dynamically discover the server's port at runtime.
    // However, for easy testing you may want force a certain port so you can just hit the refresh button.
    [httpServer setPort:12345];

    // We're going to extend the base HTTPConnection class with our MyHTTPConnection class.
    // This allows us to customize the server for things such as SSL and password-protection.
    [httpServer setConnectionClass:[HTTPConnection class]];

    // Serve files from the standard Sites folder
    NSString *docRoot = [[NSBundle mainBundle] pathForResource:@"skeqi-dbg" ofType:@"bundle"];
//    DDLogInfo(@"Setting document root: %@", docRoot);

    [httpServer setDocumentRoot:docRoot];

    NSError *error = nil;
    if(![httpServer start:&error])
    {
//      DDLogError(@"Error starting HTTP Server: %@", error);
    }
*/
  }
  return self;
}

- (void)update {
  page_->Update(self.metalQueue);
}

- (void)encodeDrawCalls:(id<MTLRenderCommandEncoder>)encoder {
  // TODO: pass a gfx::RenderCommandEncoder to Page.
  page_->EncodeDrawCalls(encoder);
}

@end
