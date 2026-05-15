#pragma once
#include "CadProcessor.h"
#include <QAction>
#include <QMainWindow>
#include <QTextEdit>
#include <QToolBar>
#include <memory>


class ViewerWidget;
class AssemblyTreeWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onImport();
  void onExport();
  void onExportPdf();

private:
  void setupUI();
  void updateButtonStates();
  void appendLog(const QString &message, const QString &level = "INFO");

  // UI Components
  QTextEdit*            m_logDisplay;
  ViewerWidget*         m_viewer;
  AssemblyTreeWidget*   m_tree;
  QAction*              m_importAction;
  QAction*              m_exportAction;
  QAction*              m_exportPdfAction;
  QToolBar*             m_toolBar;

  std::unique_ptr<CADCore::CadProcessor>    m_processor;
  QString                                   m_currentFilePath;
};
