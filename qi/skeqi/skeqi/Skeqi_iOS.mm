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
namespace pen {

class Page;

class Stroke {
 public:
  typedef std::vector<CubicBezier2f> Path;

  void Finalize();
  void SetPath(Path&& path);
  void SetSamplePoints(const std::vector<Pt2f>& points);

 private:
  Stroke(Page* page)
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
  Page* page_;
  shared_ptr<gfx::Buffer> buffer_;
  int vertex_count_;
  int offset_;
  std::atomic_bool finalized_;

  friend class Page;
};


class Page {
 public:
  Page(shared_ptr<gfx::Device> device, id<MTLLibrary> library);

  shared_ptr<Stroke> NewStroke() {
    // TODO: need better way to clear strokes.
    if (strokes_.size() > 20) {
      strokes_.clear();
      dirty_strokes_.clear();
    }

    // TODO: can't use make_shared because constructor is private... what is best idiom?
    shared_ptr<Stroke> stroke(new Stroke(this));
    strokes_.push_back(stroke);
    return stroke;
  }

  // Compute the number of vertices to use to tesselate each Stoke path segment.
  std::vector<size_t> ComputeVertexCounts(Stroke::Path& path);

  // TODO: use a qi::gfx::RenderCommandEncoder.
  void EncodeDrawCalls(id<MTLRenderCommandEncoder> encoder);

  // TODO: perhaps put all strokes into one shared Buffer.
  void FinalizeStroke(Stroke* stroke) {}

  shared_ptr<gfx::Buffer> NewBuffer(size_t length) { return device_->NewBuffer(length); }

  // Pre-render update.
  void Update(id<MTLCommandQueue>queue);

 private:
  void SetupRenderPipeline(id<MTLLibrary> library);
  void SetupComputePipeline(id<MTLLibrary> library);

  shared_ptr<gfx::Device> device_;
  id<MTLRenderPipelineState> render_pipeline_;
  id<MTLComputePipelineState> compute_pipeline_;
  std::vector<shared_ptr<Stroke>> strokes_;
  std::unordered_set<Stroke*> dirty_strokes_;

  friend class Stroke;
};


// Stroke method implementations //////////////////////////////////////////////

void Stroke::Finalize() {
  bool was_finalized = finalized_.exchange(true);
  if (!was_finalized) {
    page_->FinalizeStroke(this);

    if (!path_.empty()) {
      std::cerr << "STROKE (" << path_.size() << " segments):" << std::endl;
      for (int i = 0; i < path_.size(); ++i) {
        std::cerr << "       " << path_[i] << std::endl;
      }
    }

    // TODO(jjosh):
    qi::pen::DoProtoTest();
  }
}

void Stroke::SetPath(Path&& path) {
  ASSERT(!finalized_);
  path_ = std::move(path);
  page_->dirty_strokes_.insert(this);
}

void Stroke::SetSamplePoints(const std::vector<Pt2f>& points) {
  ASSERT(!finalized_);
  path_.clear();
  size_t buffer_size = points.size() * sizeof(Vertex);
  if (!buffer_ || buffer_->GetLength() < buffer_size) {
    buffer_ = page_->NewBuffer(buffer_size * 1.2 + 200);
  }
  auto verts = reinterpret_cast<Stroke::Vertex*>(buffer_->GetContents());
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

void Stroke::EncodeDrawCalls(gfx::RenderCommandEncoder* encoder) {
  if (vertex_count_ > 0) {
    encoder->SetVertexBuffer(buffer_.get(), offset_, 0);
    if (path_.empty()) {
      encoder->DrawPoints(0, vertex_count_);
    } else {
      encoder->DrawTriangleStrip(0, vertex_count_);
    }
  }
}

void Stroke::Tesselate(id<MTLComputeCommandEncoder>encoder) {
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
      // TODO: avoid static_cast<>
      auto buf = static_cast<gfx::port::Buffer_iOS*>(buffer_.get());
      [encoder setBuffer:buf->GetBuffer() offset:offset atIndex:1];

      MTLSize threadgroup_size = MTLSizeMake(vertex_counts[i] / 2, 1, 1);
      MTLSize threadgroups = MTLSizeMake(1, 1, 1);
      [encoder dispatchThreadgroups:threadgroups
               threadsPerThreadgroup: threadgroup_size];

      offset += vertex_counts[i] * sizeof(Stroke::Vertex);
    }
    return;
  }

  // Use CPU to generate vertices for each Path segment.
  auto verts = reinterpret_cast<Stroke::Vertex*>(buffer_->GetContents());
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
    // Bump Stroke::Vertex pointer to tesselate the next segment.
    verts += seg_vert_count;
  }
}


// Page method implementations ////////////////////////////////////////////////

Page::Page(shared_ptr<gfx::Device> device, id<MTLLibrary> library) : device_(device) {
  SetupRenderPipeline(library);
  SetupComputePipeline(library);
}

void Page::SetupRenderPipeline(id<MTLLibrary> library) {
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

void Page::SetupComputePipeline(id<MTLLibrary> library) {
  auto kernel_func = [library newFunctionWithName:@"strokeBezierTesselate"];
  NSError *errors = nil;
  compute_pipeline_ =[library.device newComputePipelineStateWithFunction:kernel_func error:&errors];
  ASSERT(compute_pipeline_ && !errors);
}

std::vector<size_t> Page::ComputeVertexCounts(Stroke::Path& path) {
  std::vector<size_t> counts;
  counts.reserve(path.size());
  for (CubicBezier2f bez : path) {
    // TODO: compute a number based on the length of each path segment.
    static const size_t kVertexCount = 128;
    counts.push_back(kVertexCount);
  }
  return counts;
}

void Page::EncodeDrawCalls(id<MTLRenderCommandEncoder> mtl_encoder) {
  [mtl_encoder pushDebugGroup:@"Render Page Strokes"];
  [mtl_encoder setRenderPipelineState:render_pipeline_];
  gfx::port::RenderCommandEncoder_iOS encoder(mtl_encoder);
  for (auto& stroke : strokes_) {
    stroke->EncodeDrawCalls(&encoder);
  }
  [mtl_encoder popDebugGroup];
}

void Page::Update(id<MTLCommandQueue>queue) {
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
  SkeqiStrokeFitter(std::shared_ptr<Page> page)
      : fitter_id(s_next_fitter_id++), page_(page), error_threshold_(0.0002) {}

  void StartStroke(Pt2f pt) {
    stroke_ = page_->NewStroke();
    stroke2_ = page_->NewStroke();
    points_.push_back(pt);
    params_.push_back(0.0);
  }
  void AddSamplePoint(Pt2f pt) {
    float dist = distance(pt, points_.back());
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
    Stroke::Path split_path;
    for (auto bez : path_) {
      auto split = bez.Split(0.5);
      split_path.push_back(split.first);
      split_path.push_back(split.second);
    }
    path_ = std::move(split_path);

    // For each of the segments computed above, compute the total segment length
    // and a arc-length parameterization.  This parameterization is a 1-D cubic
    // Bezier such that an input parameter t in the range [0,1] results in a
    // new parameter t' (also in [0,1]) such that evaluating the original
    // curve-segment at t' returns the point on the segment where the cumulative
    // arc-length to that point is t * total_segment_length.
    ASSERT(!path_.empty());
    stroke_->SetPath(std::move(path_));
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

  // Identify the stroke fitter.
  const int64 fitter_id;

 private:
  void FitSampleRange(
      int start_index, int end_index, Pt2f left_tangent, Pt2f right_tangent);

  shared_ptr<Page> page_;
  shared_ptr<Stroke> stroke_;
  shared_ptr<Stroke> stroke2_;

  std::vector<Pt2f> points_;
  std::vector<float> params_;
  float error_threshold_;
  Stroke::Path path_;

  static int64 s_next_fitter_id;
};

int64 SkeqiStrokeFitter::s_next_fitter_id = 1;


// TODO: investigate reparameterization
void SkeqiStrokeFitter::FitSampleRange(
    int start_index, int end_index, Pt2f left_tangent, Pt2f right_tangent) {
  ASSERT(end_index > start_index);

  if (end_index - start_index == 1) {
    // Only two points... use a heuristic.
    // TODO: double-check this heuristic.
    CubicBezier2f line;
    line.pts[0] = points_[start_index];
    line.pts[1] = line.pts[0] + (left_tangent * 0.25f);
    line.pts[2] = line.pts[3] + (right_tangent * 0.25f);
    line.pts[3] = points_[end_index];
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
    path_.push_back(bez);
    return;
  }

  // Error is too large... split into two ranges and fit each.
  ASSERT(split_index > start_index && split_index < end_index);
  Pt2f middle_tangent = points_[split_index + 1] - points_[split_index - 1];
  FitSampleRange(start_index, split_index, left_tangent, middle_tangent * -1.0f);
  FitSampleRange(split_index, end_index, middle_tangent, right_tangent);
}


class SkeqiTouchHandler : public ui::TouchHandler {
 public:
  SkeqiTouchHandler(shared_ptr<Page> page) : page_(page) {}

  void TouchesBegan(const std::vector<ui::Touch>* touches) override {
    for (auto touch : *touches) {
      ASSERT(fitters_.find(touch) == fitters_.end());
      auto& fitter = (fitters_[touch] = make_unique<SkeqiStrokeFitter>(page_));
      fitter->StartStroke(GetTouchPosition(touch));
    }
  }

  void TouchesCancelled(const std::vector<ui::Touch>* touches) override {
    ASSERT(false);  // not implemented
  }

  void TouchesMoved(const std::vector<ui::Touch>* touches) override {
    for (auto touch : *touches) {
      auto it = fitters_.find(touch);
      ASSERT(it != fitters_.end());
      it->second->AddSamplePoint(GetTouchPosition(touch));
    }
  }

  void TouchesEnded(const std::vector<ui::Touch>* touches) override {
    for (auto touch : *touches) {
      auto it = fitters_.find(touch);
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

  shared_ptr<Page> page_;
  std::map<ui::Touch, unique_ptr<SkeqiStrokeFitter>> fitters_;
};


}  // namespace pen
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
  std::shared_ptr<qi::pen::Page> page_;
// TODO(jjosh):  HTTPServer *httpServer;
}

- (id)init {
  self = [super init];
  if (self) {
    page_ = std::make_shared<qi::pen::Page>(self.device, self.metalLibrary);
    auto touch_handler = std::make_unique<qi::pen::SkeqiTouchHandler>(page_);
    [self setTouchHandler: std::move(touch_handler)];

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
