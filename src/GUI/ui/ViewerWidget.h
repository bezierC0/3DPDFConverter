#pragma once

#include <QWidget>
#include <Standard_Handle.hxx>

class Occt3DView;
class QVBoxLayout;
class AIS_InteractiveContext;
class V3d_View;

class ViewerWidget : public QWidget {
    Q_OBJECT
public:
    explicit ViewerWidget(QWidget* parent = nullptr);
    virtual ~ViewerWidget() = default;

    Handle(AIS_InteractiveContext) Context() const;
    Handle(V3d_View) View() const;

    void FitAll();
    void ClearObjects();

private:
    Occt3DView*         m_occtView;
    QVBoxLayout*        m_layout;
};
