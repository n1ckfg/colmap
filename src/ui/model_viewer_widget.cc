// Copyright (c) 2018, ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of the ETH Zurich and UNC Chapel Hill nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Johannes L. Schoenberger (jsch at inf.ethz.ch)

#include "ui/model_viewer_widget.h"

#include "ui/main_window.h"

#define POINT_SELECTED_R 0
#define POINT_SELECTED_G 1
#define POINT_SELECTED_B 0
#define IMAGE_R 1
#define IMAGE_G 0.1
#define IMAGE_B 0
#define IMAGE_A 0.6
#define IMAGE_SELECTED_R 1
#define IMAGE_SELECTED_G 0
#define IMAGE_SELECTED_B 1
#define IMAGE_SELECTED_A 0.6
#define SELECTION_BUFFER_IMAGE 0
#define SELECTION_BUFFER_POINT 1

#define GRID_RGBA 0.2, 0.2, 0.2, 0.6
#define X_AXIS_RGBA 0.9, 0, 0, 0.5
#define Y_AXIS_RGBA 0, 0.9, 0, 0.5
#define Z_AXIS_RGBA 0, 0, 0.9, 0.5

namespace colmap {
namespace {

// Generate unique index from RGB color in the range [0, 256^3].
inline size_t RGBToIndex(const uint8_t r, const uint8_t g, const uint8_t b) {
  return static_cast<size_t>(r) + static_cast<size_t>(g) * 256 +
         static_cast<size_t>(b) * 65536;
}

// Derive color from unique index, generated by `RGBToIndex`.
inline void IndexToRGB(const size_t index, float& r, float& g, float& b) {
  r = ((index & 0x000000FF) >> 0) / 255.0f;
  g = ((index & 0x0000FF00) >> 8) / 255.0f;
  b = ((index & 0x00FF0000) >> 16) / 255.0f;
}

void BuildImageModel(const Image& image, const Camera& camera,
                     const float image_size, const float r, const float g,
                     const float b, const float a, LinePainter::Data& line1,
                     LinePainter::Data& line2, LinePainter::Data& line3,
                     LinePainter::Data& line4, LinePainter::Data& line5,
                     LinePainter::Data& line6, LinePainter::Data& line7,
                     LinePainter::Data& line8, TrianglePainter::Data& triangle1,
                     TrianglePainter::Data& triangle2) {
  // Generate camera dimensions in OpenGL (world) coordinate space
  const float image_width = image_size * camera.Width() / 1024.0f;
  const float image_height =
      image_width * static_cast<float>(camera.Height()) / camera.Width();
  const float image_extent = std::max(image_width, image_height);
  const float camera_extent = std::max(camera.Width(), camera.Height());
  const float camera_extent_world =
      static_cast<float>(camera.ImageToWorldThreshold(camera_extent));
  const float focal_length = 2.0f * image_extent / camera_extent_world;

  const Eigen::Matrix<float, 3, 4> inv_proj_matrix =
      image.InverseProjectionMatrix().cast<float>();

  // Projection center, top-left, top-right, bottom-right, bottom-left corners

  const Eigen::Vector3f pc = inv_proj_matrix.rightCols<1>();
  const Eigen::Vector3f tl =
      inv_proj_matrix *
      Eigen::Vector4f(-image_width, image_height, focal_length, 1);
  const Eigen::Vector3f tr =
      inv_proj_matrix *
      Eigen::Vector4f(image_width, image_height, focal_length, 1);
  const Eigen::Vector3f br =
      inv_proj_matrix *
      Eigen::Vector4f(image_width, -image_height, focal_length, 1);
  const Eigen::Vector3f bl =
      inv_proj_matrix *
      Eigen::Vector4f(-image_width, -image_height, focal_length, 1);

  // Lines from sensor corners to projection center

  line1.point1 = PointPainter::Data(pc(0), pc(1), pc(2), 0.8f * r, g, b, 1);
  line1.point2 = PointPainter::Data(tl(0), tl(1), tl(2), 0.8f * r, g, b, 1);

  line2.point1 = PointPainter::Data(pc(0), pc(1), pc(2), 0.8f * r, g, b, 1);
  line2.point2 = PointPainter::Data(tr(0), tr(1), tr(2), 0.8f * r, g, b, 1);

  line3.point1 = PointPainter::Data(pc(0), pc(1), pc(2), 0.8f * r, g, b, 1);
  line3.point2 = PointPainter::Data(br(0), br(1), br(2), 0.8f * r, g, b, 1);

  line4.point1 = PointPainter::Data(pc(0), pc(1), pc(2), 0.8f * r, g, b, 1);
  line4.point2 = PointPainter::Data(bl(0), bl(1), bl(2), 0.8f * r, g, b, 1);

  line5.point1 = PointPainter::Data(tl(0), tl(1), tl(2), 0.8f * r, g, b, 1);
  line5.point2 = PointPainter::Data(tr(0), tr(1), tr(2), 0.8f * r, g, b, 1);

  line6.point1 = PointPainter::Data(tr(0), tr(1), tr(2), 0.8f * r, g, b, 1);
  line6.point2 = PointPainter::Data(br(0), br(1), br(2), 0.8f * r, g, b, 1);

  line7.point1 = PointPainter::Data(br(0), br(1), br(2), 0.8f * r, g, b, 1);
  line7.point2 = PointPainter::Data(bl(0), bl(1), bl(2), 0.8f * r, g, b, 1);

  line8.point1 = PointPainter::Data(bl(0), bl(1), bl(2), 0.8f * r, g, b, 1);
  line8.point2 = PointPainter::Data(tl(0), tl(1), tl(2), 0.8f * r, g, b, 1);

  // Sensor rectangle

  triangle1.point1 = PointPainter::Data(tl(0), tl(1), tl(2), r, g, b, a);
  triangle1.point2 = PointPainter::Data(tr(0), tr(1), tr(2), r, g, b, a);
  triangle1.point3 = PointPainter::Data(bl(0), bl(1), bl(2), r, g, b, a);

  triangle2.point1 = PointPainter::Data(bl(0), bl(1), bl(2), r, g, b, a);
  triangle2.point2 = PointPainter::Data(tr(0), tr(1), tr(2), r, g, b, a);
  triangle2.point3 = PointPainter::Data(br(0), br(1), br(2), r, g, b, a);
}

}  // namespace

ModelViewerWidget::ModelViewerWidget(QWidget* parent, OptionManager* options)
    : QOpenGLWidget(parent),
      options_(options),
      point_viewer_widget_(new PointViewerWidget(parent, this, options)),
      image_viewer_widget_(
          new DatabaseImageViewerWidget(parent, this, options)),
      movie_grabber_widget_(new MovieGrabberWidget(parent, this)),
      mouse_is_pressed_(false),
      focus_distance_(kInitFocusDistance),
      selected_image_id_(kInvalidImageId),
      selected_point3D_id_(kInvalidPoint3DId),
      coordinate_grid_enabled_(true),
      near_plane_(kInitNearPlane) {
  bg_color_[0] = 1.0f;
  bg_color_[1] = 1.0f;
  bg_color_[2] = 1.0f;

  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setSamples(4);
  format.setProfile(QSurfaceFormat::CoreProfile);
#ifdef DEBUG
  format.setOption(QSurfaceFormat::DebugContext);
#endif
  setFormat(format);
  QSurfaceFormat::setDefaultFormat(format);

  SetPointColormap(new PointColormapPhotometric());

  image_size_ = static_cast<float>(devicePixelRatio() * image_size_);
  point_size_ = static_cast<float>(devicePixelRatio() * point_size_);
}

void ModelViewerWidget::initializeGL() {
  initializeOpenGLFunctions();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  SetupPainters();
  SetupView();
}

void ModelViewerWidget::paintGL() {
  glClearColor(bg_color_[0], bg_color_[1], bg_color_[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const QMatrix4x4 pmv_matrix = projection_matrix_ * model_view_matrix_;

  // Model view matrix for center of view
  QMatrix4x4 model_view_center_matrix = model_view_matrix_;
  const Eigen::Vector4f rot_center =
      QMatrixToEigen(model_view_matrix_).inverse() *
      Eigen::Vector4f(0, 0, -focus_distance_, 1);
  model_view_center_matrix.translate(rot_center(0), rot_center(1),
                                     rot_center(2));

  // Coordinate system
  if (coordinate_grid_enabled_) {
    const QMatrix4x4 pmvc_matrix =
        projection_matrix_ * model_view_center_matrix;
    coordinate_axes_painter_.Render(pmv_matrix, width(), height(), 2);
    coordinate_grid_painter_.Render(pmvc_matrix, width(), height(), 1);
  }

  // Points
  point_painter_.Render(pmv_matrix, point_size_);
  point_connection_painter_.Render(pmv_matrix, width(), height(), 1);

  // Images
  image_line_painter_.Render(pmv_matrix, width(), height(), 1);
  image_triangle_painter_.Render(pmv_matrix);
  image_connection_painter_.Render(pmv_matrix, width(), height(), 1);

  // Movie grabber cameras
  movie_grabber_path_painter_.Render(pmv_matrix, width(), height(), 1.5);
  movie_grabber_line_painter_.Render(pmv_matrix, width(), height(), 1);
  movie_grabber_triangle_painter_.Render(pmv_matrix);
}

void ModelViewerWidget::resizeGL(int width, int height) {
  glViewport(0, 0, width, height);
  ComposeProjectionMatrix();
  UploadCoordinateGridData();
}

void ModelViewerWidget::ReloadReconstruction() {
  CHECK_NOTNULL(reconstruction);

  cameras = reconstruction->Cameras();
  points3D = reconstruction->Points3D();
  reg_image_ids = reconstruction->RegImageIds();

  images.clear();
  for (const image_t image_id : reg_image_ids) {
    images[image_id] = reconstruction->Image(image_id);
  }

  statusbar_status_label->setText(QString().sprintf(
      "%d Images - %d Points", static_cast<int>(reg_image_ids.size()),
      static_cast<int>(points3D.size())));

  Upload();
}

void ModelViewerWidget::ClearReconstruction() {
  cameras.clear();
  images.clear();
  points3D.clear();
  reg_image_ids.clear();
  reconstruction = nullptr;
  Upload();
}

int ModelViewerWidget::GetProjectionType() const {
  return options_->render->projection_type;
}

void ModelViewerWidget::SetPointColormap(PointColormapBase* colormap) {
  point_colormap_.reset(colormap);
}

void ModelViewerWidget::UpdateMovieGrabber() {
  UploadMovieGrabberData();
  update();
}

void ModelViewerWidget::EnableCoordinateGrid() {
  coordinate_grid_enabled_ = true;
  update();
}

void ModelViewerWidget::DisableCoordinateGrid() {
  coordinate_grid_enabled_ = false;
  update();
}

void ModelViewerWidget::ChangeFocusDistance(const float delta) {
  if (delta == 0.0f) {
    return;
  }
  const float prev_focus_distance = focus_distance_;
  float diff = delta * ZoomScale() * kFocusSpeed;
  focus_distance_ -= diff;
  if (focus_distance_ < kMinFocusDistance) {
    focus_distance_ = kMinFocusDistance;
    diff = prev_focus_distance - focus_distance_;
  } else if (focus_distance_ > kMaxFocusDistance) {
    focus_distance_ = kMaxFocusDistance;
    diff = prev_focus_distance - focus_distance_;
  }
  const Eigen::Matrix4f vm_mat = QMatrixToEigen(model_view_matrix_).inverse();
  const Eigen::Vector3f tvec(0, 0, diff);
  const Eigen::Vector3f tvec_rot = vm_mat.block<3, 3>(0, 0) * tvec;
  model_view_matrix_.translate(tvec_rot(0), tvec_rot(1), tvec_rot(2));
  ComposeProjectionMatrix();
  UploadCoordinateGridData();
  update();
}

void ModelViewerWidget::ChangeNearPlane(const float delta) {
  if (delta == 0.0f) {
    return;
  }
  near_plane_ *= (1.0f + delta / 100.0f * kNearPlaneScaleSpeed);
  near_plane_ = std::max(kMinNearPlane, std::min(kMaxNearPlane, near_plane_));
  ComposeProjectionMatrix();
  UploadCoordinateGridData();
  update();
}

void ModelViewerWidget::ChangePointSize(const float delta) {
  if (delta == 0.0f) {
    return;
  }
  point_size_ *= (1.0f + delta / 100.0f * kPointScaleSpeed);
  point_size_ = std::max(kMinPointSize, std::min(kMaxPointSize, point_size_));
  update();
}

void ModelViewerWidget::RotateView(const float x, const float y,
                                   const float prev_x, const float prev_y) {
  if (x - prev_x == 0 && y - prev_y == 0) {
    return;
  }

  // Rotation according to the Arcball method "ARCBALL: A User Interface for
  // Specifying Three-Dimensional Orientation Using a Mouse", Ken Shoemake,
  // University of Pennsylvania, 1992.

  // Determine Arcball vector on unit sphere.
  const Eigen::Vector3f u = PositionToArcballVector(x, y);
  const Eigen::Vector3f v = PositionToArcballVector(prev_x, prev_y);

  // Angle between vectors.
  const float angle = 2.0f * std::acos(std::min(1.0f, u.dot(v)));

  const float kMinAngle = 1e-3f;
  if (angle > kMinAngle) {
    const Eigen::Matrix4f vm_mat = QMatrixToEigen(model_view_matrix_).inverse();

    // Rotation axis.
    Eigen::Vector3f axis = vm_mat.block<3, 3>(0, 0) * v.cross(u);
    axis = axis.normalized();
    // Center of rotation is current focus.
    const Eigen::Vector4f rot_center =
        vm_mat * Eigen::Vector4f(0, 0, -focus_distance_, 1);
    // First shift to rotation center, then rotate and shift back.
    model_view_matrix_.translate(rot_center(0), rot_center(1), rot_center(2));
    model_view_matrix_.rotate(RadToDeg(angle), axis(0), axis(1), axis(2));
    model_view_matrix_.translate(-rot_center(0), -rot_center(1),
                                 -rot_center(2));
    update();
  }
}

void ModelViewerWidget::TranslateView(const float x, const float y,
                                      const float prev_x, const float prev_y) {
  if (x - prev_x == 0 && y - prev_y == 0) {
    return;
  }

  Eigen::Vector3f tvec(x - prev_x, prev_y - y, 0.0f);

  if (options_->render->projection_type ==
      RenderOptions::ProjectionType::PERSPECTIVE) {
    tvec *= ZoomScale();
  } else if (options_->render->projection_type ==
             RenderOptions::ProjectionType::ORTHOGRAPHIC) {
    tvec *= 2.0f * OrthographicWindowExtent() / height();
  }

  const Eigen::Matrix4f vm_mat = QMatrixToEigen(model_view_matrix_).inverse();

  const Eigen::Vector3f tvec_rot = vm_mat.block<3, 3>(0, 0) * tvec;
  model_view_matrix_.translate(tvec_rot(0), tvec_rot(1), tvec_rot(2));

  update();
}

void ModelViewerWidget::ChangeCameraSize(const float delta) {
  if (delta == 0.0f) {
    return;
  }
  image_size_ *= (1.0f + delta / 100.0f * kImageScaleSpeed);
  image_size_ = std::max(kMinImageSize, std::min(kMaxImageSize, image_size_));
  UploadImageData();
  UploadMovieGrabberData();
  update();
}

void ModelViewerWidget::ResetView() {
  SetupView();
  Upload();
}

QMatrix4x4 ModelViewerWidget::ModelViewMatrix() const {
  return model_view_matrix_;
}

void ModelViewerWidget::SetModelViewMatrix(const QMatrix4x4& matrix) {
  model_view_matrix_ = matrix;
  update();
}

void ModelViewerWidget::SelectObject(const int x, const int y) {
  makeCurrent();

  // Ensure that anti-aliasing does not change the colors of objects.
  glDisable(GL_MULTISAMPLE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Upload data in selection mode (one color per object).
  UploadImageData(true);
  UploadPointData(true);

  // Render in selection mode, with larger points to improve selection accuracy.
  const QMatrix4x4 pmv_matrix = projection_matrix_ * model_view_matrix_;
  image_triangle_painter_.Render(pmv_matrix);
  point_painter_.Render(pmv_matrix, 2 * point_size_);

  const int scaled_x = devicePixelRatio() * x;
  const int scaled_y = devicePixelRatio() * (height() - y - 1);

  QOpenGLFramebufferObjectFormat fbo_format;
  fbo_format.setSamples(0);
  QOpenGLFramebufferObject fbo(1, 1, fbo_format);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFramebufferObject());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.handle());
  glBlitFramebuffer(scaled_x, scaled_y, scaled_x + 1, scaled_y + 1, 0, 0, 1, 1,
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  fbo.bind();
  std::array<uint8_t, 3> color;
  glReadPixels(0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, color.data());
  fbo.release();

  const size_t index = RGBToIndex(color[0], color[1], color[2]);

  if (index < selection_buffer_.size()) {
    const char buffer_type = selection_buffer_[index].second;
    if (buffer_type == SELECTION_BUFFER_IMAGE) {
      selected_image_id_ = static_cast<image_t>(selection_buffer_[index].first);
      selected_point3D_id_ = kInvalidPoint3DId;
      ShowImageInfo(selected_image_id_);
    } else if (buffer_type == SELECTION_BUFFER_POINT) {
      selected_image_id_ = kInvalidImageId;
      selected_point3D_id_ = selection_buffer_[index].first;
      ShowPointInfo(selection_buffer_[index].first);
    } else {
      selected_image_id_ = kInvalidImageId;
      selected_point3D_id_ = kInvalidPoint3DId;
      image_viewer_widget_->hide();
    }
  } else {
    selected_image_id_ = kInvalidImageId;
    selected_point3D_id_ = kInvalidPoint3DId;
    image_viewer_widget_->hide();
  }

  // Re-enable, since temporarily disabled above.
  glEnable(GL_MULTISAMPLE);

  selection_buffer_.clear();

  UploadPointData();
  UploadImageData();
  UploadPointConnectionData();
  UploadImageConnectionData();

  update();
}

void ModelViewerWidget::SelectMoviewGrabberView(const size_t view_idx) {
  selected_movie_grabber_view_ = view_idx;
  UploadMovieGrabberData();
  update();
}

QImage ModelViewerWidget::GrabImage() {
  makeCurrent();

  DisableCoordinateGrid();

  paintGL();

  const int scaled_width = static_cast<int>(devicePixelRatio() * width());
  const int scaled_height = static_cast<int>(devicePixelRatio() * height());

  QOpenGLFramebufferObjectFormat fbo_format;
  fbo_format.setSamples(0);
  QOpenGLFramebufferObject fbo(scaled_width, scaled_height, fbo_format);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFramebufferObject());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.handle());
  glBlitFramebuffer(0, 0, scaled_width, scaled_height, 0, 0, scaled_width,
                    scaled_height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                    GL_NEAREST);

  fbo.bind();
  QImage image(scaled_width, scaled_height, QImage::Format_RGB888);
  glReadPixels(0, 0, scaled_width, scaled_height, GL_RGB, GL_UNSIGNED_BYTE,
               image.bits());
  fbo.release();

  EnableCoordinateGrid();

  return image.mirrored();
}

void ModelViewerWidget::GrabMovie() { movie_grabber_widget_->show(); }

void ModelViewerWidget::ShowPointInfo(const point3D_t point3D_id) {
  point_viewer_widget_->Show(point3D_id);
}

void ModelViewerWidget::ShowImageInfo(const image_t image_id) {
  image_viewer_widget_->ShowImageWithId(image_id);
}

float ModelViewerWidget::PointSize() const { return point_size_; }

float ModelViewerWidget::ImageSize() const { return image_size_; }

void ModelViewerWidget::SetPointSize(const float point_size) {
  point_size_ = point_size;
}

void ModelViewerWidget::SetImageSize(const float image_size) {
  image_size_ = image_size;
  UploadImageData();
}

void ModelViewerWidget::SetBackgroundColor(const float r, const float g,
                                           const float b) {
  bg_color_[0] = r;
  bg_color_[1] = g;
  bg_color_[2] = b;
  update();
}

void ModelViewerWidget::mousePressEvent(QMouseEvent* event) {
  if (mouse_press_timer_.isActive()) {  // Select objects (2. click)
    mouse_is_pressed_ = false;
    mouse_press_timer_.stop();
    selection_buffer_.clear();
    SelectObject(event->pos().x(), event->pos().y());
  } else {  // Set timer to remember 1. click
    mouse_press_timer_.setSingleShot(true);
    mouse_press_timer_.start(kDoubleClickInterval);
    mouse_is_pressed_ = true;
    prev_mouse_pos_ = event->pos();
  }
  event->accept();
}

void ModelViewerWidget::mouseReleaseEvent(QMouseEvent* event) {
  mouse_is_pressed_ = false;
  event->accept();
}

void ModelViewerWidget::mouseMoveEvent(QMouseEvent* event) {
  if (mouse_is_pressed_) {
    if (event->buttons() & Qt::RightButton ||
        (event->buttons() & Qt::LeftButton &&
         event->modifiers() & Qt::ControlModifier)) {
      TranslateView(event->pos().x(), event->pos().y(), prev_mouse_pos_.x(),
                    prev_mouse_pos_.y());
    } else if (event->buttons() & Qt::LeftButton) {
      RotateView(event->pos().x(), event->pos().y(), prev_mouse_pos_.x(),
                 prev_mouse_pos_.y());
    }
  }
  prev_mouse_pos_ = event->pos();
  event->accept();
}

void ModelViewerWidget::wheelEvent(QWheelEvent* event) {
  if (event->modifiers() & Qt::ControlModifier) {
    ChangePointSize(event->delta());
  } else if (event->modifiers() & Qt::AltModifier) {
    ChangeCameraSize(event->delta());
  } else if (event->modifiers() & Qt::ShiftModifier) {
    ChangeNearPlane(event->delta());
  } else {
    ChangeFocusDistance(event->delta());
  }
  event->accept();
}

void ModelViewerWidget::SetupPainters() {
  makeCurrent();

  coordinate_axes_painter_.Setup();
  coordinate_grid_painter_.Setup();

  point_painter_.Setup();
  point_connection_painter_.Setup();

  image_line_painter_.Setup();
  image_triangle_painter_.Setup();
  image_connection_painter_.Setup();

  movie_grabber_path_painter_.Setup();
  movie_grabber_line_painter_.Setup();
  movie_grabber_triangle_painter_.Setup();
}

void ModelViewerWidget::SetupView() {
  point_size_ = kInitPointSize;
  image_size_ = kInitImageSize;
  focus_distance_ = kInitFocusDistance;
  model_view_matrix_.setToIdentity();
  model_view_matrix_.translate(0, 0, -focus_distance_);
  model_view_matrix_.rotate(225, 1, 0, 0);
  model_view_matrix_.rotate(-45, 0, 1, 0);
}

void ModelViewerWidget::Upload() {
  point_colormap_->Prepare(cameras, images, points3D, reg_image_ids);

  ComposeProjectionMatrix();

  UploadPointData();
  UploadImageData();
  UploadMovieGrabberData();
  UploadPointConnectionData();
  UploadImageConnectionData();

  update();
}

void ModelViewerWidget::UploadCoordinateGridData() {
  makeCurrent();

  const float scale = ZoomScale();

  // View center grid
  std::vector<LinePainter::Data> grid_data(3);

  grid_data[0].point1 = PointPainter::Data(-20 * scale, 0, 0, GRID_RGBA);
  grid_data[0].point2 = PointPainter::Data(20 * scale, 0, 0, GRID_RGBA);

  grid_data[1].point1 = PointPainter::Data(0, -20 * scale, 0, GRID_RGBA);
  grid_data[1].point2 = PointPainter::Data(0, 20 * scale, 0, GRID_RGBA);

  grid_data[2].point1 = PointPainter::Data(0, 0, -20 * scale, GRID_RGBA);
  grid_data[2].point2 = PointPainter::Data(0, 0, 20 * scale, GRID_RGBA);

  coordinate_grid_painter_.Upload(grid_data);

  // Coordinate axes
  std::vector<LinePainter::Data> axes_data(3);

  axes_data[0].point1 = PointPainter::Data(0, 0, 0, X_AXIS_RGBA);
  axes_data[0].point2 = PointPainter::Data(50 * scale, 0, 0, X_AXIS_RGBA);

  axes_data[1].point1 = PointPainter::Data(0, 0, 0, Y_AXIS_RGBA);
  axes_data[1].point2 = PointPainter::Data(0, 50 * scale, 0, Y_AXIS_RGBA);

  axes_data[2].point1 = PointPainter::Data(0, 0, 0, Z_AXIS_RGBA);
  axes_data[2].point2 = PointPainter::Data(0, 0, 50 * scale, Z_AXIS_RGBA);

  coordinate_axes_painter_.Upload(axes_data);
}

void ModelViewerWidget::UploadPointData(const bool selection_mode) {
  makeCurrent();

  std::vector<PointPainter::Data> data;

  // Assume we want to display the majority of points
  data.reserve(points3D.size());

  const size_t min_track_len =
      static_cast<size_t>(options_->render->min_track_len);

  if (selected_image_id_ == kInvalidImageId &&
      images.count(selected_image_id_) == 0) {
    for (const auto& point3D : points3D) {
      if (point3D.second.Error() <= options_->render->max_error &&
          point3D.second.Track().Length() >= min_track_len) {
        PointPainter::Data painter_point;
        painter_point.x = static_cast<float>(point3D.second.XYZ(0));
        painter_point.y = static_cast<float>(point3D.second.XYZ(1));
        painter_point.z = static_cast<float>(point3D.second.XYZ(2));
        if (selection_mode) {
          const size_t index = selection_buffer_.size();
          selection_buffer_.push_back(
              std::make_pair(point3D.first, SELECTION_BUFFER_POINT));
          IndexToRGB(index, painter_point.r, painter_point.g, painter_point.b);
        } else if (point3D.first == selected_point3D_id_) {
          painter_point.r = POINT_SELECTED_R;
          painter_point.g = POINT_SELECTED_G;
          painter_point.b = POINT_SELECTED_B;
        } else {
          const Eigen::Vector3f& rgb =
              point_colormap_->ComputeColor(point3D.first, point3D.second);
          painter_point.r = rgb(0);
          painter_point.g = rgb(1);
          painter_point.b = rgb(2);
        }
        painter_point.a = 1;
        data.push_back(painter_point);
      }
    }
  } else {  // Image selected
    const auto& selected_image = images[selected_image_id_];
    for (const auto& point3D : points3D) {
      if (point3D.second.Error() <= options_->render->max_error &&
          point3D.second.Track().Length() >= min_track_len) {
        PointPainter::Data painter_point;
        painter_point.x = static_cast<float>(point3D.second.XYZ(0));
        painter_point.y = static_cast<float>(point3D.second.XYZ(1));
        painter_point.z = static_cast<float>(point3D.second.XYZ(2));
        if (selection_mode) {
          const size_t index = selection_buffer_.size();
          selection_buffer_.push_back(
              std::make_pair(point3D.first, SELECTION_BUFFER_POINT));
          IndexToRGB(index, painter_point.r, painter_point.g, painter_point.b);
        } else if (selected_image.HasPoint3D(point3D.first)) {
          painter_point.r = IMAGE_SELECTED_R;
          painter_point.g = IMAGE_SELECTED_G;
          painter_point.b = IMAGE_SELECTED_B;
        } else if (point3D.first == selected_point3D_id_) {
          painter_point.r = POINT_SELECTED_R;
          painter_point.g = POINT_SELECTED_G;
          painter_point.b = POINT_SELECTED_B;
        } else {
          const Eigen::Vector3f& rgb =
              point_colormap_->ComputeColor(point3D.first, point3D.second);
          painter_point.r = rgb(0);
          painter_point.g = rgb(1);
          painter_point.b = rgb(2);
        }
        painter_point.a = 1;
        data.push_back(painter_point);
      }
    }
  }

  point_painter_.Upload(data);
}

void ModelViewerWidget::UploadPointConnectionData() {
  makeCurrent();

  std::vector<LinePainter::Data> line_data;

  if (selected_point3D_id_ == kInvalidPoint3DId) {
    // No point selected, so upload empty data
    point_connection_painter_.Upload(line_data);
    return;
  }

  const auto& point3D = points3D[selected_point3D_id_];

  // 3D point position
  LinePainter::Data line;
  line.point1 = PointPainter::Data(
      static_cast<float>(point3D.XYZ(0)), static_cast<float>(point3D.XYZ(1)),
      static_cast<float>(point3D.XYZ(2)), POINT_SELECTED_R, POINT_SELECTED_G,
      POINT_SELECTED_B, 0.8);

  // All images in which 3D point is observed
  for (const auto& track_el : point3D.Track().Elements()) {
    const Image& conn_image = images[track_el.image_id];
    const Eigen::Vector3f conn_proj_center =
        conn_image.ProjectionCenter().cast<float>();
    line.point2 = PointPainter::Data(conn_proj_center(0), conn_proj_center(1),
                                     conn_proj_center(2), POINT_SELECTED_R,
                                     POINT_SELECTED_G, POINT_SELECTED_B, 1);
    line_data.push_back(line);
  }

  point_connection_painter_.Upload(line_data);
}

void ModelViewerWidget::UploadImageData(const bool selection_mode) {
  makeCurrent();

  std::vector<LinePainter::Data> line_data;
  line_data.reserve(8 * reg_image_ids.size());

  std::vector<TrianglePainter::Data> triangle_data;
  triangle_data.reserve(2 * reg_image_ids.size());

  for (const image_t image_id : reg_image_ids) {
    const Image& image = images[image_id];
    const Camera& camera = cameras[image.CameraId()];

    float r, g, b, a;
    if (selection_mode) {
      const size_t index = selection_buffer_.size();
      selection_buffer_.push_back(
          std::make_pair(image_id, SELECTION_BUFFER_IMAGE));
      IndexToRGB(index, r, g, b);
      a = 1;
    } else {
      if (image_id == selected_image_id_) {
        r = IMAGE_SELECTED_R;
        g = IMAGE_SELECTED_G;
        b = IMAGE_SELECTED_B;
        a = IMAGE_SELECTED_A;
      } else {
        r = IMAGE_R;
        g = IMAGE_G;
        b = IMAGE_B;
        a = IMAGE_A;
      }
    }

    LinePainter::Data line1, line2, line3, line4, line5, line6, line7, line8;
    TrianglePainter::Data triangle1, triangle2;
    BuildImageModel(image, camera, image_size_, r, g, b, a, line1, line2, line3,
                    line4, line5, line6, line7, line8, triangle1, triangle2);

    // Lines are not colored with the indexed color in selection mode, so do not
    // show them, so they do not block the selection process
    if (!selection_mode) {
      line_data.push_back(line1);
      line_data.push_back(line2);
      line_data.push_back(line3);
      line_data.push_back(line4);
      line_data.push_back(line5);
      line_data.push_back(line6);
      line_data.push_back(line7);
      line_data.push_back(line8);
    }

    triangle_data.push_back(triangle1);
    triangle_data.push_back(triangle2);
  }

  image_line_painter_.Upload(line_data);
  image_triangle_painter_.Upload(triangle_data);
}

void ModelViewerWidget::UploadImageConnectionData() {
  makeCurrent();

  std::vector<LinePainter::Data> line_data;
  std::vector<image_t> image_ids;

  if (selected_image_id_ != kInvalidImageId) {
    // Show connections to selected images
    image_ids.push_back(selected_image_id_);
  } else if (options_->render->image_connections) {
    // Show all connections
    image_ids = reg_image_ids;
  } else {  // Disabled, so upload empty data
    image_connection_painter_.Upload(line_data);
    return;
  }

  for (const image_t image_id : image_ids) {
    const Image& image = images.at(image_id);

    const Eigen::Vector3f proj_center = image.ProjectionCenter().cast<float>();

    // Collect all connected images
    std::unordered_set<image_t> conn_image_ids;

    for (const Point2D& point2D : image.Points2D()) {
      if (point2D.HasPoint3D()) {
        const Point3D& point3D = points3D[point2D.Point3DId()];
        for (const auto& track_elem : point3D.Track().Elements()) {
          conn_image_ids.insert(track_elem.image_id);
        }
      }
    }

    // Center image
    LinePainter::Data line;
    line.point1 = PointPainter::Data(proj_center(0), proj_center(1),
                                     proj_center(2), IMAGE_SELECTED_R,
                                     IMAGE_SELECTED_G, IMAGE_SELECTED_B, 0.8);

    // All connected images
    for (const image_t conn_image_id : conn_image_ids) {
      const Image& conn_image = images[conn_image_id];
      const Eigen::Vector3f conn_proj_center =
          conn_image.ProjectionCenter().cast<float>();
      line.point2 = PointPainter::Data(conn_proj_center(0), conn_proj_center(1),
                                       conn_proj_center(2), IMAGE_SELECTED_R,
                                       IMAGE_SELECTED_G, IMAGE_SELECTED_B, 0.8);
      line_data.push_back(line);
    }
  }

  image_connection_painter_.Upload(line_data);
}

void ModelViewerWidget::UploadMovieGrabberData() {
  makeCurrent();

  std::vector<LinePainter::Data> path_data;
  path_data.reserve(movie_grabber_widget_->views.size());

  std::vector<LinePainter::Data> line_data;
  line_data.reserve(4 * movie_grabber_widget_->views.size());

  std::vector<TrianglePainter::Data> triangle_data;
  triangle_data.reserve(2 * movie_grabber_widget_->views.size());

  if (movie_grabber_widget_->views.size() > 0) {
    const Image& image0 = movie_grabber_widget_->views[0];
    Eigen::Vector3f prev_proj_center = image0.ProjectionCenter().cast<float>();

    for (size_t i = 1; i < movie_grabber_widget_->views.size(); ++i) {
      const Image& image = movie_grabber_widget_->views[i];
      const Eigen::Vector3f curr_proj_center =
          image.ProjectionCenter().cast<float>();
      LinePainter::Data path;
      path.point1 = PointPainter::Data(prev_proj_center(0), prev_proj_center(1),
                                       prev_proj_center(2), 0, 1, 1, 0.8);
      path.point2 = PointPainter::Data(curr_proj_center(0), curr_proj_center(1),
                                       curr_proj_center(2), 0, 1, 1, 0.8);
      path_data.push_back(path);
      prev_proj_center = curr_proj_center;
    }

    // Setup dummy camera with same settings as current OpenGL viewpoint.
    const size_t kDefaultImageWdith = 2048;
    const size_t kDefaultImageHeight = 1536;
    const double focal_length =
        -2 * tan(DegToRad(kFieldOfView) / 2.0) * kDefaultImageWdith;
    Camera camera;
    camera.InitializeWithId(SimplePinholeCameraModel::model_id, focal_length,
                            kDefaultImageWdith, kDefaultImageHeight);

    // Build all camera models
    for (size_t i = 0; i < movie_grabber_widget_->views.size(); ++i) {
      const Image& image = movie_grabber_widget_->views[i];
      float r, g, b, a;
      if (i == selected_movie_grabber_view_) {
        r = IMAGE_SELECTED_R;
        g = IMAGE_SELECTED_G;
        b = IMAGE_SELECTED_B;
        a = 1;
      } else {
        r = 0;
        g = 1;
        b = 1;
        a = IMAGE_A;
      }

      LinePainter::Data line1, line2, line3, line4, line5, line6, line7, line8;
      TrianglePainter::Data triangle1, triangle2;
      BuildImageModel(image, camera, image_size_, r, g, b, a, line1, line2,
                      line3, line4, line5, line6, line7, line8, triangle1,
                      triangle2);

      line_data.push_back(line1);
      line_data.push_back(line2);
      line_data.push_back(line3);
      line_data.push_back(line4);
      line_data.push_back(line5);
      line_data.push_back(line6);
      line_data.push_back(line7);
      line_data.push_back(line8);

      triangle_data.push_back(triangle1);
      triangle_data.push_back(triangle2);
    }
  }

  movie_grabber_path_painter_.Upload(path_data);
  movie_grabber_line_painter_.Upload(line_data);
  movie_grabber_triangle_painter_.Upload(triangle_data);
}

void ModelViewerWidget::ComposeProjectionMatrix() {
  projection_matrix_.setToIdentity();
  if (options_->render->projection_type ==
      RenderOptions::ProjectionType::PERSPECTIVE) {
    projection_matrix_.perspective(kFieldOfView, AspectRatio(), near_plane_,
                                   kFarPlane);
  } else if (options_->render->projection_type ==
             RenderOptions::ProjectionType::ORTHOGRAPHIC) {
    const float extent = OrthographicWindowExtent();
    projection_matrix_.ortho(-AspectRatio() * extent, AspectRatio() * extent,
                             -extent, extent, near_plane_, kFarPlane);
  }
}

float ModelViewerWidget::ZoomScale() const {
  // "Constant" scale factor w.r.t. zoom-level.
  return 2.0f * std::tan(static_cast<float>(DegToRad(kFieldOfView)) / 2.0f) *
         std::abs(focus_distance_) / height();
}

float ModelViewerWidget::AspectRatio() const {
  return static_cast<float>(width()) / static_cast<float>(height());
}

float ModelViewerWidget::OrthographicWindowExtent() const {
  return std::tan(DegToRad(kFieldOfView) / 2.0f) * focus_distance_;
}

Eigen::Vector3f ModelViewerWidget::PositionToArcballVector(
    const float x, const float y) const {
  Eigen::Vector3f vec(2.0f * x / width() - 1, 1 - 2.0f * y / height(), 0.0f);
  const float norm2 = vec.squaredNorm();
  if (norm2 <= 1.0f) {
    vec.z() = std::sqrt(1.0f - norm2);
  } else {
    vec = vec.normalized();
  }
  return vec;
}

}  // namespace colmap
