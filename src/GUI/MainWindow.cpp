#include "MainWindow.h"
#include "ui/ViewerWidget.h"
#include "ui/AssemblyTreeWidget.h"
#include <QApplication>
#include "ui/PdfExportDialog.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>
#include <QSplitter>
#include <cstdlib>

#include "ExportFacade.h"

#include <AIS_InteractiveContext.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs_AISObject.hxx>
#include <TDF_LabelSequence.hxx>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_processor(std::make_unique<CADCore::CadProcessor>()) {
  setupUI();
  updateButtonStates();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
  setWindowTitle("3DPDFConverter");
  setWindowIcon(QIcon(":/images/images/logo.png"));
  resize(1024, 768);

  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Toolbar
  m_toolBar = addToolBar("Main Toolbar");
  m_toolBar->setMovable(false);

  m_importAction = m_toolBar->addAction(QIcon(), "Import", this, &MainWindow::onImport);
  m_exportAction = m_toolBar->addAction(QIcon(), "Export CAD", this, &MainWindow::onExport);
  m_exportPdfAction = m_toolBar->addAction(QIcon(), "Export 3D PDF", this, &MainWindow::onExportPdf);

  // Icons
  m_importAction->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
  m_exportAction->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  m_exportPdfAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

  // Splitter Setup
  QSplitter *hSplitter = new QSplitter(Qt::Horizontal, centralWidget);
  
  // Left: Tree
  m_tree = new AssemblyTreeWidget(hSplitter);
  hSplitter->addWidget(m_tree);

  // Right: Viewer + Log
  QSplitter *vSplitter = new QSplitter(Qt::Vertical, hSplitter);
  
  m_viewer = new ViewerWidget(vSplitter);
  vSplitter->addWidget(m_viewer);

  m_logDisplay = new QTextEdit(vSplitter);
  m_logDisplay->setReadOnly(true);
  m_logDisplay->setFontFamily("Courier New");
  vSplitter->addWidget(m_logDisplay);

  // Set initial sizes
  vSplitter->setStretchFactor(0, 1); 
  vSplitter->setStretchFactor(1, 0);
  // 4 : 1
  vSplitter->setSizes(QList<int>() << 800 << 200);

  hSplitter->addWidget(vSplitter);
  hSplitter->setStretchFactor(0, 1);
  hSplitter->setStretchFactor(1, 3);

  mainLayout->addWidget(hSplitter);

  // Initial Welcome Message
  m_logDisplay->setText("Please click 'Import' to load a CAD file.\n\n");

  // Status Bar
  statusBar()->showMessage("Ready");
}

void MainWindow::updateButtonStates() {
  bool hasFile = m_processor->HasLoadedFile();
  m_exportAction->setEnabled(hasFile);
  m_exportPdfAction->setEnabled(hasFile);
}

void MainWindow::appendLog(const QString &message, const QString &level) {
  m_logDisplay->append(message);
  QScrollBar *sb = m_logDisplay->verticalScrollBar();
  sb->setValue(sb->maximum());
  QApplication::processEvents();
}

void MainWindow::onImport() {
  QString fileName = QFileDialog::getOpenFileName(
      this, "Import CAD File", "",
      "CAD Files (*.step *.stp *.iges *.igs *.brep);;STEP (*.step *.stp);;IGES "
      "(*.iges *.igs);;BREP (*.brep)");
  if (fileName.isEmpty())
    return;

  appendLog("\nLoading file: " + fileName);
  statusBar()->showMessage("Loading " + fileName + "...");
  QApplication::setOverrideCursor(Qt::WaitCursor);

  bool success = m_processor->ImportFile(fileName.toStdString());

  if (success) {
    m_currentFilePath = fileName;
    updateButtonStates();
    statusBar()->showMessage("Loaded: " + fileName);
    appendLog("Import Successful.");

    // Update Tree
    Handle(TDocStd_Document) doc = m_processor->GetDocument();
    m_tree->updateTree(doc);

    // Update Viewer
    m_viewer->ClearObjects();
    if (!doc.IsNull()) {
        Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
        TDF_LabelSequence freeShapes;
        shapeTool->GetFreeShapes(freeShapes);
        
        for (Standard_Integer i = 1; i <= freeShapes.Length(); i++) {
            TDF_Label label = freeShapes.Value(i);
            Handle(XCAFPrs_AISObject) prs = new XCAFPrs_AISObject(label);
            m_viewer->Context()->Display(prs, false);
        }
        m_viewer->FitAll();
    }
  } else {
    appendLog("Import Failed!", "ERROR");
    statusBar()->showMessage("Import Failed");
  }

  QApplication::restoreOverrideCursor();
}

void MainWindow::onExport() {
  QString fileName = QFileDialog::getSaveFileName(
      this, "Export CAD File", "",
      "STEP (*.step *.stp);;IGES (*.iges *.igs);;BREP (*.brep)");
  if (fileName.isEmpty())
    return;

  appendLog("\n========== Export Report ==========");
  appendLog("Output file: " + fileName);
  statusBar()->showMessage("Exporting...");
  QApplication::setOverrideCursor(Qt::WaitCursor);

  bool success = m_processor->ExportFile(fileName.toStdString());

  QApplication::restoreOverrideCursor();

  if (success) {
    statusBar()->showMessage("Export Successful");
    appendLog("Status:  Export successful");
    appendLog("===================================");
  } else {
    statusBar()->showMessage("Export Failed");
    appendLog("Status: Export failed");
  }
}

void MainWindow::onExportPdf() {
    if (m_currentFilePath.isEmpty()) return;

    PdfExportDialog dialog(this);
    QString suggestedPdf = m_currentFilePath;
    suggestedPdf.replace(".step", ".pdf").replace(".stp", ".pdf").replace(".iges", ".pdf").replace(".igs", ".pdf");
    dialog.setInitialPath(suggestedPdf);

    if (dialog.exec() != QDialog::Accepted)
        return;

    ExportRequest request = dialog.getRequest();
    QByteArray stepBytes = m_currentFilePath.toUtf8();
    request.stepFilePath = stepBytes.constData();

    appendLog("\n========== 3D PDF Export ==========");
    appendLog("Input file: " + m_currentFilePath);
    appendLog("Output file: " + QString(request.pdfFilePath));
    statusBar()->showMessage("Exporting PDF...");
    QApplication::setOverrideCursor(Qt::WaitCursor);

    ExportResult result;
    ExportStepToPdf(request, &result);

    QApplication::restoreOverrideCursor();

    // Free strdup'ed strings from getRequest
    free((void*)request.pdfFilePath);
    free((void*)request.pdfSettings.pageSize);
    free((void*)request.pdfSettings.defaultLighting);
    free((void*)request.pdfSettings.defaultView);
    free((void*)request.pdfMetadata.title);
    free((void*)request.pdfMetadata.author);
    free((void*)request.pdfMetadata.subject);
    free((void*)request.pdfMetadata.keywords);

    if (result.code == RESULT_SUCCESS) {
        statusBar()->showMessage("PDF Export Successful");
        appendLog("Status: Export successful");
        appendLog("===================================");
    } else {
        statusBar()->showMessage("PDF Export Failed");
        appendLog(QString("Status: Export failed: %1").arg(result.errorMessage), "ERROR");
    }
}
