#pragma once

#include <QWidget>
#include <AIS_ViewController.hxx>
//#include <AIS_InteractiveContext.hxx>

//#include <V3d_View.hxx>
//#include <V3d_Viewer.hxx>

class AIS_ViewCube;
class AIS_Manipulator;
class AIS_InteractiveContext;
class V3d_View;
class V3d_Viewer;
class TopoDS_Face;
class TopoDS_Shape;

class Occt3DView : public QWidget, protected AIS_ViewController {
    Q_OBJECT
public:
    explicit Occt3DView(QWidget* parent = nullptr);
    virtual ~Occt3DView() ;

    void init();
    // Return AIS_InteractiveContext
    Handle(AIS_InteractiveContext) Context() const;
    // Return View
    const Handle(V3d_View)& View() const;
    // Return Viewer
    const Handle(V3d_Viewer)& Viewer() const;

    // View Fit
    void FitAll();
    // ClearObjects
    void ClearObjects();

protected:
    // Events
    void initializeGL();//! Initial OpenGL setup.
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    QPaintEngine* paintEngine() const override;

private:
  //! Dump OpenGL info.
  void dumpGlInfo(bool theIsBasic, bool theToPrint);

  //! Request widget paintGL() event.
  void updateView();

  //! Handle view redraw.
  virtual void handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx, const Handle(V3d_View)& theView) override;

private:
    Handle(V3d_Viewer)                  m_viewer{};
    Handle(V3d_View)                    m_view{};
    Handle(AIS_InteractiveContext)      m_context{};
    Handle(AIS_ViewCube)                m_viewCube{};
};
