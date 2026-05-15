#include "ViewerWidget.h"
#include "Occt3DView.h"
#include <QVBoxLayout>
#include <AIS_ViewCube.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>

ViewerWidget::ViewerWidget(QWidget* parent) : QWidget(parent) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0); // No margins for full view

    m_occtView = new Occt3DView(this);
    m_layout->addWidget(m_occtView);

}

Handle(AIS_InteractiveContext) ViewerWidget::Context() const {
    return m_occtView->Context();
}

Handle(V3d_View) ViewerWidget::View() const {
    return m_occtView->View();
}

void ViewerWidget::FitAll() {
    m_occtView->FitAll();
}

void ViewerWidget::ClearObjects() {
    m_occtView->ClearObjects();
}
