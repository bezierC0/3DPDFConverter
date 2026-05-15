#include "Occt3DView.h"
// C++

// Qt
#include <QWidget>
#include <QApplication>
#include <QResizeEvent>

// Occt
#include <AIS_ViewCube.hxx>
#include <AIS_AnimationCamera.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <WNT_Window.hxx>


// Map Qt buttons bitmask to virtual keys.
static Aspect_VKeyMouse qtMouseButtons2VKeys(Qt::MouseButtons theButtons)
{
    Aspect_VKeyMouse aButtons = Aspect_VKeyMouse_NONE;
    if ((theButtons & Qt::LeftButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_LeftButton;
    }
    if ((theButtons & Qt::MiddleButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_MiddleButton;
    }
    if ((theButtons & Qt::RightButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_RightButton;
    }
    return aButtons;
}

// Map Qt mouse modifiers bitmask to virtual keys.
static Aspect_VKeyFlags qtMouseModifiers2VKeys(Qt::KeyboardModifiers theModifiers)
{
    Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;
    if ((theModifiers & Qt::ShiftModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_SHIFT;
    }
    if ((theModifiers & Qt::ControlModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_CTRL;
    }
    if ((theModifiers & Qt::AltModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_ALT;
    }
    return aFlags;
}



Occt3DView::Occt3DView(QWidget* parent) 
    : QWidget(parent) 
{
    init();

    // Setup some defaults
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_NativeWindow); // Essential for direct rendering
    setMouseTracking(true); // Important for mouse move events
}

Occt3DView::~Occt3DView()
{
    Handle(Aspect_DisplayConnection) aDisp = m_viewer->Driver()->GetDisplayConnection();

    // release OCCT viewer
    m_context->RemoveAll(false);
    m_context.Nullify();
    m_view->Remove();
    m_view.Nullify();
    m_viewer.Nullify();

    aDisp.Nullify();
}

void Occt3DView::init() {
    if (!m_view.IsNull()) {
        return;
    }

    // 1. Create a 3D Viewer
    Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
    Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(displayConnection, false);

    m_viewer = new V3d_Viewer(graphicDriver);
    m_viewer->SetDefaultBackgroundColor(Quantity_NOC_BLACK);
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    m_viewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
    m_viewer->SetRectangularGridValues(0.0,0.0,10.0,10.0,0.0);

    // 2. Create a 3D View
    m_view = m_viewer->CreateView();
    m_view->SetImmediateUpdate(false);
    m_view->SetBackgroundColor(Quantity_Color(Quantity_NOC_GRAY50)); // Lighter gray


    // 3. Create an Interactive Context
    m_context = new AIS_InteractiveContext(m_viewer);

    // 4. Create View Cube (Navigation)
    m_viewCube = new AIS_ViewCube();
    m_viewCube->TransformPersistence()->SetOffset2d(Graphic3d_Vec2i(100, 150));

    

    // show Rendering info(Optional)
#ifndef __APPLE__
    m_view->ChangeRenderingParams().NbMsaaSamples = 4; // warning - affects performance
#endif
    m_view->ChangeRenderingParams().ToShowStats = true;

    Handle(AIS_AnimationCamera) anim = new AIS_AnimationCamera("ViewCubeAnim", m_view);
    m_viewCube->SetViewAnimation(anim);
    m_viewCube->SetFixedAnimationLoop(false);
    m_viewCube->SetAutoStartAnimation(true);
    
    // Display it
    m_context->Display(m_viewCube, Standard_False);

    initializeGL();
}

Handle(AIS_InteractiveContext) Occt3DView::Context() const {
    return m_context;
}

const Handle(V3d_View)& Occt3DView::View() const {
    return m_view;
}

const Handle(V3d_Viewer)& Occt3DView::Viewer() const {
    return m_viewer;
}

void Occt3DView::FitAll() {
    if (!m_view.IsNull()) {
        m_view->FitAll();
        m_view->Redraw();
    }
}

void Occt3DView::ClearObjects() {
    if (!m_context.IsNull()) {
        m_context->RemoveAll(false);
        // Resurrect the ViewCube
        if (!m_viewCube.IsNull()) {
            m_context->Display(m_viewCube, false);
        }
    }
}

void Occt3DView::paintEvent(QPaintEvent*) {
    if (m_view.IsNull()) {
        init();
    }
    if (m_view.IsNull()) {
        return;
    }
    
    // Flush events (handle rotation, panning, zoom logic)
    FlushViewEvents(m_context, m_view, true);
    
    // Workaround: Force resize check to ensure view matches window size
    m_view->MustBeResized();
    m_view->Redraw();
}

void Occt3DView::resizeEvent(QResizeEvent*) {
    if (!m_view.IsNull()) {
        m_view->MustBeResized();
    }
}

void Occt3DView::mousePressEvent(QMouseEvent* theEvent)
{
    const Graphic3d_Vec2i aPnt(theEvent->pos().x(), theEvent->pos().y());
    const Aspect_VKeyMouse aButtons = qtMouseButtons2VKeys(theEvent->buttons());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(theEvent->modifiers());

    if (!m_view.IsNull() && UpdateMouseButtons(aPnt, aButtons, aFlags, false))
    {
        update();
    }
}

void Occt3DView::mouseReleaseEvent(QMouseEvent* theEvent)
{
    const Graphic3d_Vec2i aPnt(theEvent->pos().x(), theEvent->pos().y());
    const Aspect_VKeyMouse aButtons = qtMouseButtons2VKeys(theEvent->buttons());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(theEvent->modifiers());

    if (!m_view.IsNull() && UpdateMouseButtons(aPnt, aButtons, aFlags, false))
    {
        update();
    }
}

void Occt3DView::mouseMoveEvent(QMouseEvent* theEvent)
{
    const Graphic3d_Vec2i aPnt(theEvent->pos().x(), theEvent->pos().y());
    const Aspect_VKeyMouse aButtons = qtMouseButtons2VKeys(theEvent->buttons());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(theEvent->modifiers());

    if (!m_view.IsNull() && UpdateMousePosition(aPnt, aButtons, aFlags, false))
    {
        update();
    }
}

void Occt3DView::wheelEvent(QWheelEvent* theEvent)
{
    const Graphic3d_Vec2i aPnt(theEvent->position().x(), theEvent->position().y());
    
    if (!m_view.IsNull() && UpdateZoom(Aspect_ScrollDelta(aPnt, theEvent->angleDelta().y() / 8.0)))
    {
        update();
    }
}

QPaintEngine* Occt3DView::paintEngine() const {
    return nullptr;
}

// #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// bool Occt3DView::nativeEventFilter(const QByteArray& theEventType, void* theMsg, qintptr* )
// #else
// bool Occt3DView::nativeEventFilter(const QByteArray& theEventType, void* theMsg, long* )
// #endif
// {
// //   if (Occt3DView::qtHandleNativeEvent(*this, m_view, theEventType, theMsg))
// //   {
// //     updateView();
// //     return true;
// //   }
//   return false;
// }

void Occt3DView::dumpGlInfo(bool theIsBasic, bool theToPrint){
}

void Occt3DView::updateView(){
    QWidget::update();
}

void Occt3DView::handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx, const Handle(V3d_View)& theView) {
    AIS_ViewController::handleViewRedraw(theCtx, theView);
    if (myToAskNextFrame) updateView(); // ask more frames for animation
}

void Occt3DView::initializeGL()
{
    if (m_view.IsNull())
    {
        return;
    }

    // Determine the window handle
    Aspect_Handle aWindowHandle = (Aspect_Handle)winId();

    // Create the OCCT window wrapper
    Handle(WNT_Window) aWindow = new WNT_Window(aWindowHandle);
    m_view->SetWindow(aWindow);

    if (!aWindow->IsMapped())
    {
        aWindow->Map();
    }

    // Force resize to sync dimensions
    m_view->MustBeResized();
}