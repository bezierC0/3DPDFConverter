#ifndef PDFEXPORTDIALOG_H
#define PDFEXPORTDIALOG_H

#include <QDialog>
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

private:
    Ui::PdfExportDialog* ui;
};

#endif // PDFEXPORTDIALOG_H
