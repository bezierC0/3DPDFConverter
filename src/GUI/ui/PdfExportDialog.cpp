#include "PdfExportDialog.h"
#include "ui_PdfExportDialog.h"
#include <QFileDialog>
#include <cstring>

PdfExportDialog::PdfExportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PdfExportDialog)
{
    ui->setupUi(this);
    connect(ui->btnBrowse, &QPushButton::clicked, this, &PdfExportDialog::onBrowse);
}

PdfExportDialog::~PdfExportDialog() {
    delete ui;
}

void PdfExportDialog::setInitialPath(const QString& path) {
    ui->editPath->setText(path);
}

void PdfExportDialog::onBrowse() {
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Select Output PDF"), ui->editPath->text(), tr("PDF Files (*.pdf)"));
    if (!fileName.isEmpty()) {
        ui->editPath->setText(fileName);
    }
}

ExportRequest PdfExportDialog::getRequest() const {
    ExportRequest req = {};
    
    // Path
    QString path = ui->editPath->text();
    req.pdfFilePath = strdup(path.toUtf8().constData());

    // Mesh
    req.meshSettings.deflection = ui->spinDeflection->value();
    req.meshSettings.angle = 0.5; // Default
    req.meshSettings.relativeMesh = false;

    // PDF
    req.pdfSettings.pageSize = strdup(ui->comboPageSize->currentText().toUtf8().constData());
    req.pdfSettings.defaultLighting = strdup(ui->comboLighting->currentText().toUtf8().constData());
    req.pdfSettings.defaultView = strdup(ui->comboView->currentText().toUtf8().constData());
    req.pdfSettings.backgroundColorRGB = 0xFFFFFF; // White default

    return req;
}
