/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef RecordAndMove_h
#define RecordAndMove_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_RecordAndMoveControls.h"


//#include "kukaRobotDevice.h"
#include "lancetApplyDeviceRegistratioinFilter.h"
#include "lancetApplySurfaceRegistratioinFilter.h"
#include "lancetApplySurfaceRegistratioinStaticImageFilter.h"
#include "lancetKukaRobotDevice.h"
#include "lancetNavigationObjectVisualizationFilter.h"
#include "lancetPathPoint.h"
#include "mitkTrackingDeviceSource.h"
#include "mitkVirtualTrackingDevice.h"
#include "robotRegistration.h"
#include "lancetNavigationSceneFilter.h"
#include <mutex>
// ************************* Ϊʲô�������� *************************************
//include "udpRobotiInfoClient.h"
/**
  \brief RecordAndMove

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class RecordAndMove : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;
  

public slots:
	void UseKuka();
	void UseVega();
	void ShowToolStatus_Vega();

	void Record();
	void ThreadRecord();

	void HandDrive();
	void ThreadHandDrive();
	void StopHandDrive();

	void SetAsTarget();
	void MoveToTarget();

	void CapturePose(bool translationOnly);
	void OnRobotCapture();



protected:
	

  std::array<double, 6> m_Target;

  bool m_ThreadRecord_Flag;

  bool m_ThreadHandDrive_Flag = false;
  std::thread m_ThreadHandDrive_Handler;
  std::thread m_ThreadRecord_Handler;

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  //*********Helper Function****************
  RobotRegistration m_RobotRegistration;

  mitk::NavigationData::Pointer GetNavigationDataInRef(mitk::NavigationData::Pointer nd,
	  mitk::NavigationData::Pointer nd_ref);

  Ui::RecordAndMoveControls m_Controls;

  lancet::KukaRobotDevice_New::Pointer m_KukaTrackingDevice;
  //vega trackingDeviceSource
  mitk::TrackingDeviceSource::Pointer m_VegaSource;
  //kuka trackingDeviceSource
  mitk::TrackingDeviceSource::Pointer m_KukaSource;

  lancet::NavigationObjectVisualizationFilter::Pointer m_KukaVisualizer;
  lancet::NavigationObjectVisualizationFilter::Pointer m_VegaVisualizer;
  lancet::ApplyDeviceRegistratioinFilter::Pointer m_KukaApplyRegistrationFilter;
  QTimer* m_KukaVisualizeTimer{ nullptr };
  QTimer* m_VegaVisualizeTimer{ nullptr };
  mitk::NavigationToolStorage::Pointer m_KukaToolStorage;
  mitk::NavigationToolStorage::Pointer m_VegaToolStorage;

  //filter test
  mitk::VirtualTrackingDevice::Pointer m_VirtualDevice1;
  mitk::VirtualTrackingDevice::Pointer m_VirtualDevice2;
  mitk::TrackingDeviceSource::Pointer m_VirtualDevice1Source;
  mitk::TrackingDeviceSource::Pointer m_VirtualDevice2Source;
  lancet::NavigationObjectVisualizationFilter::Pointer m_VirtualDevice1Visualizer;
  lancet::NavigationObjectVisualizationFilter::Pointer m_VirtualDevice2Visualizer;
  QTimer* m_VirtualDevice1Timer{ nullptr };
  QTimer* m_VirtualDevice2Timer{ nullptr };
  mitk::NavigationToolStorage::Pointer m_VirtualDevice1ToolStorage;
  mitk::NavigationToolStorage::Pointer m_VirtualDevice2ToolStorage;
  QTimer* m_ToolStatusTimer{ nullptr }; //<<< tracking timer that updates the status widgets

  std::vector<mitk::NavigationData::Pointer> m_VegaNavigationData;
  std::vector<mitk::NavigationData::Pointer> m_KukaNavigationData;

  //fri test
  // lancet::FriManager m_FriManager;
  std::thread m_friThread;
  mitk::AffineTransform3D::Pointer m_ProbeRealTimePose;
  mitk::AffineTransform3D::Pointer m_ProbeInitPose;
  mitk::AffineTransform3D::Pointer T_probe2robotEndRF;
  bool m_KeepUpdateFriTransform{ true };
  bool preciousHandGuiding_select = false;
  bool m_handGuidingOn = true;
  //double m_offset[3]{ 0,0,0 };

  //use navigation scene filter
  lancet::NavigationSceneFilter::Pointer m_NavigationSceneFilter;
  NavigationScene::Pointer m_NavigationScene;

  //robot registration
  unsigned int m_IndexOfRobotCapture{ 0 };
  std::array<vtkMatrix4x4*, 10> m_AutoPoses{};
  mitk::AffineTransform3D::Pointer m_RobotRegistrationMatrix;//
};

#endif // RecordAndMove_h