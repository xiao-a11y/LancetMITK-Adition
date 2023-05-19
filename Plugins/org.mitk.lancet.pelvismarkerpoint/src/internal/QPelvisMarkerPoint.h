/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QPelvisMarkerPoint_h
#define QPelvisMarkerPoint_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QPelvisMarkerPointControls.h"

namespace mitk
{
	class NavigationData;
}

namespace lancet { class IDevicesAdministrationService; }
namespace lancet::spatial_fitting
{
	class PelvisCheckPointModel;
}

/**
  \brief QPelvisMarkerPoint

  \warning  This class is not yet documented. Use "git blame" and ask the author 
	          to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QPelvisMarkerPoint : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QPelvisMarkerPoint();
  virtual ~QPelvisMarkerPoint();
protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing();

  void ConnectQtWidgets();

  void UpdateUiForService();

	berry::SmartPointer<lancet::spatial_fitting::PelvisCheckPointModel> GetServiceModel() const;

	static lancet::IDevicesAdministrationService* GetDevicesService();
protected slots:

  void on_pushButtonPelvisCheckpoint_clicked();
  
  void on_toolCollector_fail(int);

	void on_toolCollector_complete(mitk::NavigationData*);

	void on_toolCollector_step(int, mitk::NavigationData*);
private:

	void InitializeTrackingToolsWidget();
private:
  Ui::QPelvisMarkerPointControls m_Controls;
};

#endif // QPelvisMarkerPoint_h
