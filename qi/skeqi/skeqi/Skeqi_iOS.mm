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

 private:
  id<MTLRenderCommandEncoder> encoder_;
};

void RenderCommandEncoder_iOS::SetVertexBuffer(
    Buffer* buffer, int offset, int index) {
  ASSERT(dynamic_cast<Buffer_iOS>(buffer));
  id<MTLBuffer> mtl_buffer = static_cast<Buffer_iOS*>(buffer)->GetBuffer();
  [encoder_ setVertexBuffer:mtl_buffer offset:offset atIndex:index];
}

void RenderCommandEncoder_iOS::DrawTriangleStrip(
    int vertex_start, int vertex_count) {
  [encoder_ drawPrimitives:MTLPrimitiveTypeTriangleStrip
           vertexStart:0 vertexCount:vertex_count];
}

}  // namespace port

typedef port::RenderCommandEncoder_iOS RenderCommandEncoder;

}  // namespace gfx
}  // namespace qi



namespace qi {
namespace pen {

class Page;

class Stroke {
 public:
  typedef std::vector<CubicBezier2f> Path;

  int vertex_count;
  int offset;
  gfx::port::Buffer_iOS buffer;

  void Finalize();
  void SetPath(Path&& path);

 private:
  Stroke(Page* page)
      : page_(page), vertex_count_(0), offset_(0), finalized_(false) {}

  void EncodeDrawCalls(gfx::RenderCommandEncoder* encoder);
  void Tesselate();

  struct Vertex {
    float px, py, pz, pw;
    float nx, ny, length;
    unsigned char cb, cg, cr, ca;
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

  shared_ptr<pen::Stroke> NewStroke() {
    // TODO: can't use make_shared because constructor is private... what is best idiom?
    shared_ptr<pen::Stroke> stroke(new pen::Stroke(this));
    strokes_.push_back(stroke);
    return stroke;
  }

  // Compute the number of vertices to use to tesselate each Stoke path segment.
  std::vector<size_t> ComputeVertexCounts(Stroke::Path& path);

  // TODO: use a qi::gfx::RenderCommandEncoder.
  void EncodeDrawCalls(id<MTLRenderCommandEncoder> encoder);

  // TODO: perhaps put all strokes into one shared Buffer.
  void FinalizeStroke(pen::Stroke* stroke) {}

  shared_ptr<gfx::Buffer> NewBuffer(size_t length) { return device_->NewBuffer(length); }

 private:
  shared_ptr<gfx::Device> device_;
  id<MTLRenderPipelineState> pipeline_;
  std::vector<shared_ptr<pen::Stroke>> strokes_;
};


// Stroke method implementations //////////////////////////////////////////////

void Stroke::Finalize() {
  bool was_finalized = finalized_.exchange(true);
  if (!was_finalized) {
    page_->FinalizeStroke(this);
  }
}

void Stroke::SetPath(Path&& path) {
  ASSERT(!finalized_);
  path_ = std::move(path);
  Tesselate();
}

void Stroke::EncodeDrawCalls(gfx::RenderCommandEncoder* encoder) {
  if (vertex_count_ > 0) {
    encoder->SetVertexBuffer(buffer_.get(), offset_, 0);
    encoder->SetVertexBuffer(buffer_.get(), offset_ + 28, 1);
    encoder->DrawTriangleStrip(0, vertex_count_);
  }
}

void Stroke::Tesselate() {
  if (path_.empty()) return;

  std::vector<size_t> vertex_counts = page_->ComputeVertexCounts(path_);
  size_t total_vertex_count = 0;
  for (size_t count : vertex_counts) {
    ASSERT(count == (count % 2 * 2));
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

  // Generate vertices for each Path segment.
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
      std::pair<Pt2f, Pt2f> pt = bez.EvaluatePointAndNormal(i * param_incr);
      verts[i].px = verts[i+1].px = pt.first.x;
      verts[i].py = verts[i+1].py = pt.first.y;
      verts[i].pz = verts[i+1].pz = 0.0;
      verts[i].pw = verts[i+1].pw = 1.0;
      verts[i].nx = pt.second.x;
      verts[i].ny = pt.second.y;
      verts[i+1].nx = -pt.second.x;
      verts[i+1].ny = -pt.second.y;

      // TODO this is a hack since our vertex shader doesn't actually use the specified normals.
      float kWidth = 0.02;
      verts[i].px += verts[i].nx * kWidth;
      verts[i].py += verts[i].ny * kWidth;
      verts[i+1].px += verts[i+1].nx * kWidth;
      verts[i+1].py += verts[i+1].ny * kWidth;

      // TODO length
      verts[i].length = verts[i+1].length = 0.0;
      verts[i].cb = verts[i+1].cb = rgb;
      verts[i].cg = verts[i+1].cg = rgb;
      verts[i].cr = verts[i+1].cr = rgb;
      verts[i].ca = verts[i+1].ca = 1;
    }
    // Bump Stroke::Vertex pointer to tesselate the next segment.
    verts += seg_vert_count;
  }
}


// Page method implementations ////////////////////////////////////////////////

Page::Page(shared_ptr<gfx::Device> device, id<MTLLibrary> library) : device_(device) {
  auto vert_func = [library newFunctionWithName:@"passThroughVertex2"];
  auto frag_func = [library newFunctionWithName:@"passThroughFragment"];

  auto vert_desc = [[MTLVertexDescriptor alloc] init];
  vert_desc.attributes[0].format = MTLVertexFormatFloat4;
  vert_desc.attributes[0].bufferIndex = 0;
  vert_desc.attributes[0].offset = 0;
  vert_desc.attributes[1].format = MTLVertexFormatUChar4Normalized;
  vert_desc.attributes[1].bufferIndex = 1;
  vert_desc.attributes[1].offset = 0;
  vert_desc.layouts[0].stride = sizeof(Stroke::Vertex);
  vert_desc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
  vert_desc.layouts[1].stride = sizeof(Stroke::Vertex);
  vert_desc.layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;

  auto pipeline_desc = [[MTLRenderPipelineDescriptor alloc] init];
  pipeline_desc.vertexDescriptor = vert_desc;
  pipeline_desc.vertexFunction = vert_func;
  pipeline_desc.fragmentFunction = frag_func;
  // TODO: needed if set elsewhere earlier?
  pipeline_desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

  NSError *errors = nil;
  pipeline_ = [library.device newRenderPipelineStateWithDescriptor:pipeline_desc error:&errors];
  ASSERT(pipeline_ && !errors);
}

std::vector<size_t> Page::ComputeVertexCounts(Stroke::Path& path) {
  std::vector<size_t> counts;
  counts.reserve(path.size());
  for (CubicBezier2f bez : path) {
    // TODO: compute a number based on the length of each path segment.
    static const size_t kVertexCount = 200;
    counts.push_back(kVertexCount);
  }
  return counts;
}

void Page::EncodeDrawCalls(id<MTLRenderCommandEncoder> mtl_encoder) {
  [mtl_encoder pushDebugGroup:@"Render Page Strokes"];
  [mtl_encoder setRenderPipelineState:pipeline_];
  gfx::port::RenderCommandEncoder_iOS encoder(mtl_encoder);
  for (auto& stroke : strokes_) {
    stroke->EncodeDrawCalls(&encoder);
  }
  [mtl_encoder popDebugGroup];
}



class SkeqiStrokeFitter {
 public:
  SkeqiStrokeFitter(std::shared_ptr<qi::pen::Page> page)
      : fitter_id(s_next_fitter_id++), page_(page) {}
  ~SkeqiStrokeFitter() {}

  void StartStroke(Pt2f pt) {
    stroke_ = page_->NewStroke();
    points_.push_back(pt);
    params_.push_back(0.0);
  }
  void AddSamplePoint(Pt2f pt) {
    float distance = points_.back().dist(pt);
    points_.push_back(pt);
    params_.push_back(params_.back() + distance);

    // TODO: need to start drawing immediately, not after 4 samples.
    if (points_.size() > 4) {
      // TODO: don't recompute stable path segments near the beginning of the stroke.
      stroke_->SetPath(FitPathToSamplePoints());
    }
  }
  void FinishStroke() {
    stroke_->Finalize();
    stroke_.reset();
    points_.clear();
    params_.clear();
  }

  // Identify the stroke fitter.
  const int64 fitter_id;

 private:
  Stroke::Path FitPathToSamplePoints();

  shared_ptr<pen::Page> page_;
  shared_ptr<pen::Stroke> stroke_;

  std::vector<Pt2f> points_;
  std::vector<float> params_;

  static int64 s_next_fitter_id;
};

int64 SkeqiStrokeFitter::s_next_fitter_id = 1;


Stroke::Path SkeqiStrokeFitter::FitPathToSamplePoints() {
  int num_pts = points_.size();

  Stroke::Path path;

  static const int kSegs = 4;

  int range = num_pts / kSegs;
  int start_index = 0;
  for (int i = 1; i <= kSegs; i++) {
    int end_index = num_pts - (kSegs - i) * range - 1;

    // Normalize cumulative length between 0.0 and 1.0.
    float param_shift = -params_[start_index];
    float param_scale = 1.0 / (params_[end_index] + param_shift);

    CubicBezier2f bez = CubicBezier2f::Fit(
        &(points_[start_index]), end_index - start_index + 1,
        &(params_[start_index]), param_shift, param_scale,
        points_[start_index+1] - points_[start_index], points_[end_index-1] - points_[end_index]);
    path.push_back(bez);

    bez.Print();

    start_index = end_index + 1;
  }

  return path;
}


class SkeqiTouchHandler : public ui::TouchHandler {
 public:
  SkeqiTouchHandler(shared_ptr<pen::Page> page) : page_(page) {}

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

  shared_ptr<pen::Page> page_;
  std::map<ui::Touch, unique_ptr<SkeqiStrokeFitter>> fitters_;
};


}  // namespace pen
}  // namespace qi


@implementation Skeqi_iOS {
  std::shared_ptr<qi::pen::Page> page_;
}

- (id)init {
  self = [super init];
  if (self) {
    page_ = std::make_shared<qi::pen::Page>(self.device, self.metalLibrary);
    auto touch_handler = std::make_unique<qi::pen::SkeqiTouchHandler>(page_);
    [self setTouchHandler: std::move(touch_handler)];
  }
  return self;
}

- (void)encodeDrawCalls:(id<MTLRenderCommandEncoder>)encoder {
  // TODO: pass a gfx::RenderCommandEncoder to Page.
  page_->EncodeDrawCalls(encoder);
}

@end
