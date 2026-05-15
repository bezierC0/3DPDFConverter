#include "CadProcessor.h"

// OpenCASCADE Includes
#include <BRepCheck_Analyzer.hxx>
#include <BRepCheck_Result.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GProp_GProps.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IGESCAFControl_Writer.hxx>
#include <Interface_Static.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeExtend.hxx>
#include <ShapeFix_Shape.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TDataStd_Name.hxx>
#include <TDocStd_Document.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#include <algorithm>
#include <filesystem>
#include <sstream>

namespace CADCore {

struct CadProcessor::Implementation {
  TopoDS_Shape m_currentShape;
  Handle(TDocStd_Document) m_doc;

  // XCAF Tools
  Handle(XCAFDoc_ColorTool) m_colorTool;
  Handle(XCAFDoc_ShapeTool) m_shapeTool;
  Handle(XCAFDoc_LayerTool) m_layerTool;
  Handle(XCAFDoc_MaterialTool) m_materialTool;

  Implementation() {
    // Initialize XCAF application
    if (XCAFApp_Application::GetApplication().IsNull()) {
      // Just ensuring it's initialized
    }
  }
};

CadProcessor::CadProcessor()
    : m_impl(std::make_unique<Implementation>()), m_hasLoadedFile(false) {}

CadProcessor::~CadProcessor() = default;

bool CadProcessor::HasLoadedFile() const { return m_hasLoadedFile; }

std::string CadProcessor::GetCurrentFilePath() const {
  return m_currentFilePath;
}

Handle(TDocStd_Document) CadProcessor::GetDocument() const {
  return m_impl->m_doc;
}

bool CadProcessor::ImportFile(const std::string &filePath) {
  bool success = false;
  std::filesystem::path p(filePath);
  std::string ext = p.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Reset old data
  m_impl->m_currentShape.Nullify();
  if (!m_impl->m_doc.IsNull())
    m_impl->m_doc->Close();
  m_impl->m_doc.Nullify();

  if (ext == ".step" || ext == ".stp") {
    success = ReadSTEP(filePath);
  } else if (ext == ".iges" || ext == ".igs") {
    success = ReadIGES(filePath);
  } else if (ext == ".brep") {
    success = ReadBREP(filePath);
  }

  if (success) {
    m_currentFilePath = filePath;
    m_hasLoadedFile = true;
  }
  return success;
}

bool CadProcessor::ReadSTEP(const std::string &filePath) {
  Handle(TDocStd_Application) app = XCAFApp_Application::GetApplication();
  app->NewDocument("MDTV-XCAF", m_impl->m_doc);

  STEPCAFControl_Reader reader;
  reader.SetColorMode(true);
  reader.SetNameMode(true);
  reader.SetLayerMode(true);
  reader.SetPropsMode(true);

  IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
  if (status != IFSelect_RetDone)
    return false;

  if (!reader.Transfer(m_impl->m_doc))
    return false;

  m_impl->m_shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_impl->m_doc->Main());
  m_impl->m_colorTool = XCAFDoc_DocumentTool::ColorTool(m_impl->m_doc->Main());
  m_impl->m_layerTool = XCAFDoc_DocumentTool::LayerTool(m_impl->m_doc->Main());
  m_impl->m_materialTool =
      XCAFDoc_DocumentTool::MaterialTool(m_impl->m_doc->Main());

  TDF_LabelSequence freeShapes;
  m_impl->m_shapeTool->GetFreeShapes(freeShapes);

  BRep_Builder B;
  TopoDS_Compound compound;
  B.MakeCompound(compound);

  for (Standard_Integer i = 1; i <= freeShapes.Length(); i++) {
    TopoDS_Shape shape = m_impl->m_shapeTool->GetShape(freeShapes.Value(i));
    B.Add(compound, shape);
  }

  m_impl->m_currentShape = compound;
  return !m_impl->m_currentShape.IsNull();
}

bool CadProcessor::ReadIGES(const std::string &filePath) {
  Handle(TDocStd_Application) app = XCAFApp_Application::GetApplication();
  app->NewDocument("MDTV-XCAF", m_impl->m_doc);

  IGESCAFControl_Reader reader;
  reader.SetColorMode(true);
  reader.SetNameMode(true);
  reader.SetLayerMode(true);

  IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
  if (status != IFSelect_RetDone)
    return false;

  if (!reader.Transfer(m_impl->m_doc))
    return false;

  m_impl->m_shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_impl->m_doc->Main());
  m_impl->m_colorTool = XCAFDoc_DocumentTool::ColorTool(m_impl->m_doc->Main());
  m_impl->m_layerTool = XCAFDoc_DocumentTool::LayerTool(m_impl->m_doc->Main());
  m_impl->m_materialTool =
      XCAFDoc_DocumentTool::MaterialTool(m_impl->m_doc->Main());

  TDF_LabelSequence freeShapes;
  m_impl->m_shapeTool->GetFreeShapes(freeShapes);

  BRep_Builder B;
  TopoDS_Compound compound;
  B.MakeCompound(compound);

  for (Standard_Integer i = 1; i <= freeShapes.Length(); i++) {
    TopoDS_Shape shape = m_impl->m_shapeTool->GetShape(freeShapes.Value(i));
    B.Add(compound, shape);
  }
  m_impl->m_currentShape = compound;
  return !m_impl->m_currentShape.IsNull();
}

bool CadProcessor::ReadBREP(const std::string &filePath) {
  BRep_Builder builder;
  if (!BRepTools::Read(m_impl->m_currentShape, filePath.c_str(), builder)) {
    return false;
  }

  Handle(TDocStd_Application) app = XCAFApp_Application::GetApplication();
  app->NewDocument("MDTV-XCAF", m_impl->m_doc);
  m_impl->m_shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_impl->m_doc->Main());
  m_impl->m_colorTool = XCAFDoc_DocumentTool::ColorTool(m_impl->m_doc->Main());
  m_impl->m_layerTool = XCAFDoc_DocumentTool::LayerTool(m_impl->m_doc->Main());
  m_impl->m_materialTool =
      XCAFDoc_DocumentTool::MaterialTool(m_impl->m_doc->Main());

  m_impl->m_shapeTool->AddShape(m_impl->m_currentShape);

  return true;
}


bool CadProcessor::ExportFile(const std::string &filePath) {
  bool success = false;
  std::filesystem::path p(filePath);
  std::string ext = p.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == ".step" || ext == ".stp") {
    success = WriteSTEP(filePath);
  } else if (ext == ".iges" || ext == ".igs") {
    success = WriteIGES(filePath);
  } else if (ext == ".brep") {
    success = WriteBREP(filePath);
  }
  return success;
}

bool CadProcessor::WriteSTEP(const std::string &filePath) {
  STEPCAFControl_Writer writer;
  writer.SetColorMode(true);
  writer.SetNameMode(true);
  writer.SetLayerMode(true);
  writer.SetPropsMode(true);

  // Ensure we are transferring the doc
  if (!writer.Transfer(m_impl->m_doc, STEPControl_AsIs))
    return false;

  IFSelect_ReturnStatus status = writer.Write(filePath.c_str());
  return (status == IFSelect_RetDone);
}

bool CadProcessor::WriteIGES(const std::string &filePath) {
  IGESCAFControl_Writer writer;
  writer.SetColorMode(true);
  writer.SetNameMode(true);
  writer.SetLayerMode(true);

  if (!writer.Transfer(m_impl->m_doc))
    return false;

  return writer.Write(filePath.c_str());
}

bool CadProcessor::WriteBREP(const std::string &filePath) {
  return BRepTools::Write(m_impl->m_currentShape, filePath.c_str());
}

} // namespace CADCore
