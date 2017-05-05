/**
  * QRCapture.cpp
  * A capture object for QRScanner.
**/

#include "QRCapture.h"
#include "database/checkin.h"
#include "ui_camera.h"
#include "videosettings.h"
#include "imagesettings.h"
#include "database/user.h"
#include <iostream>

#include <QMediaService>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QMediaMetaData>

#include <QMessageBox>
#include <QPalette>
#include <QtGlobal>
#include <QtWidgets>
#include <QMainWindow>

#include "QRScanner.h"

Q_DECLARE_METATYPE(QCameraInfo)

Camera::Camera(QWidget *parent, Activity* currentActivity) :
QMainWindow(parent),
ui(new Ui::Camera),
camera(0),
imageCapture(0),
mediaRecorder(0),
isCapturingImage(false),
applicationExiting(false)
{
    ui->setupUi(this);
    current = currentActivity;
	//Camera devices:

	QActionGroup *videoDevicesGroup = new QActionGroup(this);
	videoDevicesGroup->setExclusive(true);
	foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
		QAction *videoDeviceAction = new QAction(cameraInfo.description(), videoDevicesGroup);
		videoDeviceAction->setCheckable(true);
		videoDeviceAction->setData(QVariant::fromValue(cameraInfo));
		if (cameraInfo == QCameraInfo::defaultCamera()) videoDeviceAction->setChecked(true);

		ui->menuDevices->addAction(videoDeviceAction);
	}

	connect(videoDevicesGroup, SIGNAL(triggered(QAction*)), SLOT(updateCameraDevice(QAction*)));

	setCamera(QCameraInfo::defaultCamera());
}

Camera::~Camera()
{
	delete mediaRecorder;
	delete imageCapture;
	delete camera;
}

void Camera::setCamera(const QCameraInfo &cameraInfo)
{
	delete imageCapture;
	delete mediaRecorder;
	delete camera;

	camera = new QCamera(cameraInfo);

	connect(camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(updateCameraState(QCamera::State)));
	connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));

	mediaRecorder = new QMediaRecorder(camera);
	connect(mediaRecorder, SIGNAL(stateChanged(QMediaRecorder::State)), this, SLOT(updateRecorderState(QMediaRecorder::State)));

	imageCapture = new QCameraImageCapture(camera);
	connect(mediaRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateRecordTime()));
	connect(mediaRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(displayRecorderError()));

	mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));

	camera->setViewfinder(ui->viewfinder);

	updateCameraState(camera->state());
	updateLockStatus(camera->lockStatus(), QCamera::UserRequest);
	updateRecorderState(mediaRecorder->state());

	connect(imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
	connect(imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
	connect(imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
	connect(imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this,
	SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));

	connect(camera, SIGNAL(lockStatusChanged(QCamera::LockStatus,QCamera::LockChangeReason)),
	this, SLOT(updateLockStatus(QCamera::LockStatus,QCamera::LockChangeReason)));

	updateCaptureMode();
	camera->start();
}

void Camera::keyPressEvent(QKeyEvent * event)
{
	if (event->isAutoRepeat())
	return;

	switch (event->key()) {
		case Qt::Key_CameraFocus:
			displayViewfinder();
			camera->searchAndLock();
			event->accept();
			break;
		case Qt::Key_Camera:
			if (camera->captureMode() == QCamera::CaptureStillImage) {
				takeImage();
			} else {
				if (mediaRecorder->state() == QMediaRecorder::RecordingState) stop();
				else record();
			}
			event->accept();
			break;
	}
}

void Camera::keyReleaseEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat()) return;

	switch (event->key()) {
		case Qt::Key_CameraFocus:
			camera->unlock();
			break;
	}
}

void Camera::updateRecordTime()
{
	QString str = QString("Recorded %1 sec").arg(mediaRecorder->duration()/1000);
	ui->statusbar->showMessage(str);
}

void Camera::processCapturedImage(int requestId, const QImage& img)
{
	Q_UNUSED(requestId);
	QImage scaledImage = img.scaled(ui->viewfinder->size(),
			Qt::KeepAspectRatio,
			Qt::SmoothTransformation);

	ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

	// Display captured image for 4 seconds.
    // displayCapturedImage();

	// process QR
	QRScanner scan;
    QString result = scan.decode(img);
	qInfo(result.toUtf8().data());
	if (result==QString("")) {
		QMessageBox::warning(this, tr("Error"), QString("No QR symbols found."));
	}
    else {
        // do SOMETHING with the data...
        User* theUser = User::getUserWithUUID(result.toUtf8().data());
        std::cout << theUser->getUUID() << std::endl;
        Checkin *c = Checkin::createCheckin(theUser->getUserId(), current->getId());
        current->addCheckins(c);
        this->close();
    }
}

void Camera::configureCaptureSettings()
{
	switch (camera->captureMode()) {
	case QCamera::CaptureStillImage:
		configureImageSettings();
		break;
	case QCamera::CaptureVideo:
		configureVideoSettings();
		break;
	default:
		break;
	}
}

void Camera::configureVideoSettings()
{
	VideoSettings settingsDialog(mediaRecorder);

	settingsDialog.setAudioSettings(audioSettings);
	settingsDialog.setVideoSettings(videoSettings);
	settingsDialog.setFormat(videoContainerFormat);

	if (settingsDialog.exec()) {
		audioSettings = settingsDialog.audioSettings();
		videoSettings = settingsDialog.videoSettings();
		videoContainerFormat = settingsDialog.format();

		mediaRecorder->setEncodingSettings(
		audioSettings,
		videoSettings,
		videoContainerFormat);
	}
}

void Camera::configureImageSettings()
{
	ImageSettings settingsDialog(imageCapture);

	settingsDialog.setImageSettings(imageSettings);

	if (settingsDialog.exec()) {
		imageSettings = settingsDialog.imageSettings();
		imageCapture->setEncodingSettings(imageSettings);
	}
}

void Camera::record()
{
	mediaRecorder->record();
	updateRecordTime();
}

void Camera::pause()
{
	mediaRecorder->pause();
}

void Camera::stop()
{
	mediaRecorder->stop();
}

void Camera::setMuted(bool muted)
{
	mediaRecorder->setMuted(muted);
}

void Camera::toggleLock()
{
	switch (camera->lockStatus()) {
	case QCamera::Searching:
	case QCamera::Locked:
		camera->unlock();
		break;
	case QCamera::Unlocked:
		camera->searchAndLock();
	}
}

void Camera::updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason)
{
	return; // nothing to do here
}

void Camera::takeImage()
{
	isCapturingImage = true;
	imageCapture->capture();
}

void Camera::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
	Q_UNUSED(id);
	Q_UNUSED(error);
	QMessageBox::warning(this, tr("Image Capture Error"), errorString);
	isCapturingImage = false;
}

void Camera::startCamera()
{
	camera->start();
}

void Camera::stopCamera()
{
	camera->stop();
}

void Camera::updateCaptureMode()
{
	int tabIndex = 0;
	QCamera::CaptureModes captureMode = tabIndex == 0 ? QCamera::CaptureStillImage : QCamera::CaptureVideo;

	if (camera->isCaptureModeSupported(captureMode))
	camera->setCaptureMode(captureMode);
}

void Camera::updateCameraState(QCamera::State state)
{
	switch (state) {
	case QCamera::ActiveState:
		ui->actionStartCamera->setEnabled(false);
		ui->actionStopCamera->setEnabled(true);
		ui->actionSettings->setEnabled(true);
		break;
	case QCamera::UnloadedState:
	case QCamera::LoadedState:
		ui->actionStartCamera->setEnabled(true);
		ui->actionStopCamera->setEnabled(false);
		ui->actionSettings->setEnabled(false);
	}
}

void Camera::updateRecorderState(QMediaRecorder::State state)
{
	return; // nothing to do here
}

void Camera::setExposureCompensation(int index)
{
	camera->exposure()->setExposureCompensation(index*0.5);
}

void Camera::displayRecorderError()
{
	QMessageBox::warning(this, tr("Capture error"), mediaRecorder->errorString());
}

void Camera::displayCameraError()
{
	QMessageBox::warning(this, tr("Camera error"), camera->errorString());
}

void Camera::updateCameraDevice(QAction *action)
{
	setCamera(qvariant_cast<QCameraInfo>(action->data()));
}

void Camera::displayViewfinder()
{
	ui->stackedWidget->setCurrentIndex(0);
}

void Camera::displayCapturedImage()
{
	ui->stackedWidget->setCurrentIndex(1);
}

void Camera::readyForCapture(bool ready)
{
	ui->takeImageButton->setEnabled(ready);
}

void Camera::imageSaved(int id, const QString &fileName)
{
	Q_UNUSED(id);
	Q_UNUSED(fileName);
	isCapturingImage = false;
	if (applicationExiting)
	close();
}

void Camera::closeEvent(QCloseEvent *event)
{
	if (isCapturingImage) {
		setEnabled(false);
		applicationExiting = true;
	event->ignore();
	} else {
		event->accept();
	}
}
