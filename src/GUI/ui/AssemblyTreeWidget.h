#pragma once
#include <QTreeWidget>
#include <TDocStd_Document.hxx>


class XCAFDoc_ShapeTool;
class TDF_Label;

class AssemblyTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit AssemblyTreeWidget(QWidget* parent = nullptr);


    void updateTree(const Handle(TDocStd_Document)& doc);

private:
    void traverseShapes(const TDF_Label& label, QTreeWidgetItem* parentItem, const Handle(XCAFDoc_ShapeTool)& shapeTool);
    QString getLabelName(const TDF_Label& label, const Handle(XCAFDoc_ShapeTool)& shapeTool);
};
