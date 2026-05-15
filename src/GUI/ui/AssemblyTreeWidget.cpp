#include "AssemblyTreeWidget.h"
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDataStd_Name.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TDF_Label.hxx>


AssemblyTreeWidget::AssemblyTreeWidget(QWidget* parent) 
    : QTreeWidget(parent) {
    setHeaderLabel("Assembly Structure");
}

void AssemblyTreeWidget::updateTree(const Handle(TDocStd_Document)& doc) {
    clear();
    if (doc.IsNull()) return;

    Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
    
    TDF_LabelSequence freeShapes;
    shapeTool->GetFreeShapes(freeShapes);

    for (Standard_Integer i = 1; i <= freeShapes.Length(); i++) {
        TDF_Label label = freeShapes.Value(i);
        QTreeWidgetItem* item = new QTreeWidgetItem(this);
        item->setText(0, getLabelName(label, shapeTool));
        traverseShapes(label, item, shapeTool);
    }
}

void AssemblyTreeWidget::traverseShapes(const TDF_Label& label, QTreeWidgetItem* parentItem, const Handle(XCAFDoc_ShapeTool)& shapeTool) {
    TDF_LabelSequence components;
    if (shapeTool->GetComponents(label, components)) {
        for (Standard_Integer i = 1; i <= components.Length(); i++) {
            TDF_Label compLabel = components.Value(i);
            TDF_Label refLabel;
            if (shapeTool->GetReferredShape(compLabel, refLabel)) {
                 QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
                 // We like to show Instance Name (from component) if available, else Reference Name
                 QString name = getLabelName(compLabel, shapeTool);
                 if (name.isEmpty() || name == "Shape") {
                     name = getLabelName(refLabel, shapeTool);
                 }
                 item->setText(0, name);
                 traverseShapes(refLabel, item, shapeTool);
            }
        }
    }
}

QString AssemblyTreeWidget::getLabelName(const TDF_Label& label, const Handle(XCAFDoc_ShapeTool)& shapeTool) {
    Handle(TDataStd_Name) name;
    if (label.FindAttribute(TDataStd_Name::GetID(), name)) {
        TCollection_ExtendedString extStr = name->Get();
        // Use fromUtf16 for cross-platform compatibility (TCollection_ExtendedString is UTF-16)
        return QString::fromUtf16((const ushort*)extStr.ToExtString());
    }
    return "Shape";
}
