#ifndef PDFEXPORTDIALOG_H
#define PDFEXPORTDIALOG_H

#include <QDialog>
#include <QColor>
#include "ExportTypes.h"

namespace Ui {
class PdfExportDialog;
}

/**
 * @brief Widget-based dialog for 3D PDF Export settings
 */
class PdfExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit PdfExportDialog(QWidget* parent = nullptr);
    ~PdfExportDialog();

    void setInitialPath(const QString& path);
    ExportRequest getRequest() const;

private slots:
    void onBrowse();
    void onPickBackgroundColor();
    void onToggleAdvanced();

private:
    void setupAdvancedUi();
    static const char* Dup(const QString& value);
    Ui::PdfExportDialog* ui;
    QColor m_backgroundColor;
    QWidget* m_advancedPanel;
};

#endif // PDFEXPORTDIALOG_H
