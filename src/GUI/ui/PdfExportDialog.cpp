#include "PdfExportDialog.h"
#include "ui_PdfExportDialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <cstdlib>
#include <cstring>

namespace {
MaterialSettings DefaultMaterialSettings() {
    MaterialSettings settings = {};
    settings.ambient[0] = 0.2; settings.ambient[1] = 0.2; settings.ambient[2] = 0.2; settings.ambient[3] = 1.0;
    settings.diffuse[0] = 0.6; settings.diffuse[1] = 0.65; settings.diffuse[2] = 0.8; settings.diffuse[3] = 1.0;
    settings.emissive[0] = 0.0; settings.emissive[1] = 0.0; settings.emissive[2] = 0.0; settings.emissive[3] = 1.0;
    settings.specular[0] = 0.8; settings.specular[1] = 0.8; settings.specular[2] = 0.8; settings.specular[3] = 1.0;
    settings.alpha = 1.0;
    settings.shininess = 32.0;
    return settings;
}
}

PdfExportDialog::PdfExportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PdfExportDialog)
    , m_backgroundColor(Qt::white)
    , m_advancedPanel(nullptr) {
    ui->setupUi(this);
    connect(ui->btnBrowse, &QPushButton::clicked, this, &PdfExportDialog::onBrowse);
    setupAdvancedUi();
}

PdfExportDialog::~PdfExportDialog() {
    delete ui;
}

const char* PdfExportDialog::Dup(const QString& value) {
    return _strdup(value.toUtf8().constData());
}

void PdfExportDialog::setupAdvancedUi() {
    auto* root = qobject_cast<QVBoxLayout*>(layout());
    if (!root) return;

    auto* basicRow = new QHBoxLayout();
    auto* pickColorBtn = new QPushButton(tr("Background Color"), this);
    pickColorBtn->setObjectName("btnBgColor");
    auto* advancedBtn = new QPushButton(tr("Advanced Settings"), this);
    advancedBtn->setObjectName("btnAdvanced");
    basicRow->addWidget(pickColorBtn);
    basicRow->addStretch();
    basicRow->addWidget(advancedBtn);
    root->insertLayout(1, basicRow);

    auto* panel = new QGroupBox(tr("Advanced"), this);
    panel->setCheckable(false);
    panel->setVisible(false);
    auto* form = new QFormLayout(panel);

    auto* projection = new QComboBox(panel);
    projection->setObjectName("comboProjection");
    projection->addItems({ "Perspective", "Orthographic" });
    form->addRow(tr("Projection"), projection);

    auto* fov = new QDoubleSpinBox(panel);
    fov->setObjectName("spinFov");
    fov->setRange(1.0, 179.0);
    fov->setValue(30.0);
    form->addRow(tr("FOV (deg)"), fov);

    auto* roll = new QDoubleSpinBox(panel);
    roll->setObjectName("spinRoll");
    roll->setRange(-180.0, 180.0);
    roll->setValue(0.0);
    form->addRow(tr("Roll (deg)"), roll);

    auto* radius = new QDoubleSpinBox(panel);
    radius->setObjectName("spinRadius");
    radius->setRange(1.0, 1000000.0);
    radius->setValue(200.0);
    form->addRow(tr("Orbit Radius"), radius);

    auto* compression = new QDoubleSpinBox(panel);
    compression->setObjectName("spinCompression");
    compression->setRange(0.0, 1.0);
    compression->setSingleStep(0.1);
    compression->setValue(0.0);
    form->addRow(tr("PRC Compression"), compression);

    auto* granularity = new QDoubleSpinBox(panel);
    granularity->setObjectName("spinGranularity");
    granularity->setRange(0.0, 1.0);
    granularity->setSingleStep(0.1);
    granularity->setValue(0.0);
    form->addRow(tr("PRC Granularity"), granularity);

    auto* keepTemp = new QCheckBox(tr("Keep temp PRC"), panel);
    keepTemp->setObjectName("checkKeepTempPrc");
    form->addRow(keepTemp);

    root->insertWidget(2, panel);
    m_advancedPanel = panel;

    connect(pickColorBtn, &QPushButton::clicked, this, &PdfExportDialog::onPickBackgroundColor);
    connect(advancedBtn, &QPushButton::clicked, this, &PdfExportDialog::onToggleAdvanced);
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

void PdfExportDialog::onPickBackgroundColor() {
    const QColor selected = QColorDialog::getColor(m_backgroundColor, this, tr("Select Background Color"));
    if (selected.isValid()) {
        m_backgroundColor = selected;
    }
}

void PdfExportDialog::onToggleAdvanced() {
    if (m_advancedPanel) {
        m_advancedPanel->setVisible(!m_advancedPanel->isVisible());
        adjustSize();
    }
}

ExportRequest PdfExportDialog::getRequest() const {
    ExportRequest req = {};
    req.apiVersion = 2;
    req.keepTempPrc = false;

    QString path = ui->editPath->text();
    req.pdfFilePath = Dup(path);

    req.meshSettings.qualityPreset = MESH_QUALITY_CUSTOM;
    req.meshSettings.deflection = ui->spinDeflection->value();
    req.meshSettings.angle = 0.5;
    req.meshSettings.relativeMesh = false;
    req.meshSettings.creaseAngle = 25.8419;

    req.prcSettings.compression = 0.0;
    req.prcSettings.granularity = 0.0;
    req.prcSettings.closed = false;
    req.prcSettings.tess = false;
    req.prcSettings.doBreak = true;
    req.prcSettings.noBreak = false;

    req.materialSettings = DefaultMaterialSettings();
    req.lineStyleSettings.lineWidth = 1.0;
    req.lineStyleSettings.lineColorRGB = 0x333333;
    req.lineStyleSettings.lineAlpha = 1.0;

    req.pdfSettings.pageSize = Dup(ui->comboPageSize->currentText());
    req.pdfSettings.customPageWidthPt = 0.0;
    req.pdfSettings.customPageHeightPt = 0.0;
    req.pdfSettings.annotLeftPt = 50.0;
    req.pdfSettings.annotBottomPt = 100.0;
    req.pdfSettings.annotRightPt = 545.0;
    req.pdfSettings.annotTopPt = 792.0;
    req.pdfSettings.backgroundColorRGB = (static_cast<uint32_t>(m_backgroundColor.red()) << 16) |
                                         (static_cast<uint32_t>(m_backgroundColor.green()) << 8) |
                                         static_cast<uint32_t>(m_backgroundColor.blue());
    req.pdfSettings.defaultLighting = Dup(ui->comboLighting->currentText());
    req.pdfSettings.defaultView = Dup(ui->comboView->currentText());
    req.pdfSettings.projectionMode = PDF_PROJ_PERSPECTIVE;
    req.pdfSettings.viewPreset = PDF_VIEW_ISO;
    req.pdfSettings.renderMode = PDF_RENDER_SOLID;
    req.pdfSettings.fovDeg = 30.0;
    req.pdfSettings.rollDeg = 0.0;
    req.pdfSettings.orbitRadius = 200.0;
    req.pdfSettings.cameraToCenter[0] = 0.0;
    req.pdfSettings.cameraToCenter[1] = 0.0;
    req.pdfSettings.cameraToCenter[2] = 1.0;
    req.pdfSettings.keepTempPrc = false;

    if (m_advancedPanel) {
        if (auto* projection = m_advancedPanel->findChild<QComboBox*>("comboProjection")) {
            req.pdfSettings.projectionMode = (projection->currentIndex() == 1) ? PDF_PROJ_ORTHOGRAPHIC : PDF_PROJ_PERSPECTIVE;
        }
        if (auto* fov = m_advancedPanel->findChild<QDoubleSpinBox*>("spinFov")) {
            req.pdfSettings.fovDeg = fov->value();
        }
        if (auto* roll = m_advancedPanel->findChild<QDoubleSpinBox*>("spinRoll")) {
            req.pdfSettings.rollDeg = roll->value();
        }
        if (auto* radius = m_advancedPanel->findChild<QDoubleSpinBox*>("spinRadius")) {
            req.pdfSettings.orbitRadius = radius->value();
        }
        if (auto* compression = m_advancedPanel->findChild<QDoubleSpinBox*>("spinCompression")) {
            req.prcSettings.compression = compression->value();
        }
        if (auto* granularity = m_advancedPanel->findChild<QDoubleSpinBox*>("spinGranularity")) {
            req.prcSettings.granularity = granularity->value();
        }
        if (auto* keepTemp = m_advancedPanel->findChild<QCheckBox*>("checkKeepTempPrc")) {
            req.keepTempPrc = keepTemp->isChecked();
            req.pdfSettings.keepTempPrc = req.keepTempPrc;
        }
    }

    req.pdfMetadata.title = Dup("3DPDF Export");
    req.pdfMetadata.author = Dup("3DPDFConverter");
    req.pdfMetadata.subject = Dup("CAD to 3D PDF");
    req.pdfMetadata.keywords = Dup("3DPDF,PRC,CAD");

    return req;
}
