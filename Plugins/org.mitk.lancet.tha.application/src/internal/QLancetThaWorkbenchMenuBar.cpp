#include "QLancetThaWorkbenchMenuBar.h"
#include "ui_qlancetthaworkbenchmenubarform.h"

// Qt
//#include <QMenu>
#include <QSlider>
#include <QWidgetAction>
#include <QResizeEvent>

// berry
#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>
#include <berryWorkbenchPlugin.h>
#include <berryIWorkbenchWindow.h>

// mitk
#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>

// lancet
#include <internal/lancetNamingFormats.h>
#include <lancetIScxmStateMachineHandler.h>
#include <lancetIScxmlStateMachineService.h>
#include <lancetIStateMachineActionProperty.h>

// tha
#include "org_mitk_lancet_tha_application_Activator.h"

struct QLancetThaWorkbenchMenuBar::QLancetThaWorkbenchMenuBarPrivateImp
{
	lancet::IScxmlStateMachineService::Pointer stateMachineService;
};

QLancetThaWorkbenchMenuBar::QLancetThaWorkbenchMenuBar(QWidget* parent)
	: imp(std::make_shared<QLancetThaWorkbenchMenuBarPrivateImp>())
	, ui(new Ui::QLancetThaWorkbenchMenuBarForm)
	, QToolBar(parent)
{
	this->ui->setupUi(this);
}

QLancetThaWorkbenchMenuBar::~QLancetThaWorkbenchMenuBar()
{
}

QMenuBar* QLancetThaWorkbenchMenuBar::GetModuleMenuBar() const
{
	return this->ui->menubarApplication;
}

QToolBar* QLancetThaWorkbenchMenuBar::GetApplicationToolBar() const
{
	return this->ui->toolbarApplication;
}

bool QLancetThaWorkbenchMenuBar::OpenStateWidget(
	berry::SmartPointer<lancet::IScxmlStateMachineState> state)
{
	if (state.IsNull() || state->GetActionProperty().IsNull())
	{
		// Warning
		return false;
	}
	berry::IWorkbenchWindow::Pointer window = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();

	if (window->GetActivePage().IsNull())
	{
		QString defaultPerspId = window->GetWorkbench()->GetPerspectiveRegistry()->GetDefaultPerspective();
		window->GetWorkbench()->ShowPerspective(defaultPerspId, window);
	}
	
	QString editorId = lancet::plugin::ToMitkId(state->GetActionProperty()->GetStateId());
	ctkPluginContext* context = PluginActivator::GetPluginContext();

	ctkServiceReference serviceRef = context->getServiceReference<mitk::IDataStorageService>();
	if (serviceRef)
	{
		mitk::IDataStorageService* dsService = context->getService<mitk::IDataStorageService>(serviceRef);
		if (dsService)
		{
			mitk::IDataStorageReference::Pointer dsRef = dsService->GetDataStorage();
			berry::IEditorInput::Pointer editorInput(new mitk::DataStorageEditorInput(dsRef));


			if (berry::WorkbenchPlugin::GetDefault()->GetEditorRegistry()->FindEditor(editorId).IsNotNull())
			{
				window->GetActivePage()->OpenEditor(editorInput, editorId, true, berry::IWorkbenchPage::MATCH_ID);
			}
			else
			{
				qDebug() << "\033[0;34m" << QString("file(%1) line(%2) func(%3)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)
					<< "Not find target plugin " << editorId;
			}

		}
	}
	qDebug() << "\033[0;34m" << QString("file(%1) line(%2) func(%3)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)
		<< QString("open %1").arg(editorId) << "\033[0m";

	return false;
}

bool QLancetThaWorkbenchMenuBar::CloseStateWidget(
	berry::SmartPointer<lancet::IScxmlStateMachineState> state)
{	
	// 
	if (state.IsNull() || state->GetActionProperty().IsNull() 
		&& state->GetManageStateMachine().IsNotNull())
	{
		// Warning
		return false;
	}

	// Correctness filtering
	// The bug does not exist logically and may be removed later.
	//if (state->GetManageStateMachine()->GetLastActiveState() != state)
	//{
	//	return false;
	//}
	
	berry::IWorkbenchWindow::Pointer window = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();

	if (window->GetActivePage()->GetActiveEditor().IsNotNull())
	{
		qDebug() << "\033[0;34m" << QString("file(%1) line(%2) func(%3)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)
			<< QString("close Editor %1").arg(window->GetActivePage()->GetActiveEditor()->GetPartName()) << "\033[0m";
		window->GetActivePage()->CloseEditor(window->GetActivePage()->GetActiveEditor(), false);
	}
	return true;
}

berry::SmartPointer<lancet::IScxmlStateMachineService> 
QLancetThaWorkbenchMenuBar::GetStateMachineService() const
{
	if (this->imp->stateMachineService.IsNull())
	{
		ctkPluginContext* context = PluginActivator::GetPluginContext();
		ctkServiceReference serviceRef =
			context->getServiceReference<lancet::IScxmlStateMachineService>();
		this->imp->stateMachineService =
			context->getService<lancet::IScxmlStateMachineService>(serviceRef);
	}
	return this->imp->stateMachineService;
}

bool QLancetThaWorkbenchMenuBar::InitializeStateMachineForUi()
{
	auto admin = this->GetStateMachineService()->GetAdminState();
	if (admin.IsNotNull() && !admin->GetSubStateMachines().empty())
	{
		auto name = admin->GetActionProperty()->GetStateUiName();
		for (auto model : admin->GetSubStateMachines())
		{
			if (model.IsNotNull())
			{
				this->InitializeUiOfStateMachine(nullptr, model);
			}
		}

		// handle
		if (admin->GetStateMachineHandler().IsNotNull())
		{
			auto qtStateHandle = admin->GetStateMachineHandler().GetPointer();
			connect(qtStateHandle, SIGNAL(StateEnter(IScxmlStateMachineState*)),
				this, SLOT(onStateMachineModuleEnter(IScxmlStateMachineState*)));
			connect(qtStateHandle, SIGNAL(StateExit(IScxmlStateMachineState*)),
				this, SLOT(onStateMachineModuleExit(IScxmlStateMachineState*)));
		}
	}
	return true;
}

void QLancetThaWorkbenchMenuBar::resizeEvent(QResizeEvent* event)
{
	this->ui->widgetContainer->setMinimumSize(event->size());
	this->ui->widgetContainer->setMaximumSize(event->size());

	return QToolBar::resizeEvent(event);
}

void QLancetThaWorkbenchMenuBar::onStateMachineModuleEnter(IScxmlStateMachineState* state)
{
	qDebug() << "\033[0;34m" << QString("file(%1) line(%2) func(%3)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)
		<< QString("State id : %1").arg(this->GetStateMachineService()->GetAdminState()->GetActiveState()->GetStateId());
	if (state && state->GetStateMachineHandler().IsNotNull())
	{
		state->GetStateMachineHandler()->StartStateMachine();
	}
}

void QLancetThaWorkbenchMenuBar::onStateMachineModuleExit(IScxmlStateMachineState* state)
{
	qDebug() << "\033[0;34m" << QString("file(%1) line(%2) func(%3)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)
		<< QString("State id : %1").arg(this->GetStateMachineService()->GetAdminState()->GetActiveState()->GetStateId());
	if (state && state->GetStateMachineHandler().IsNotNull())
	{
		state->GetStateMachineHandler()->StopStateMachine();
	}
}

void QLancetThaWorkbenchMenuBar::onStateMachineElementEnter(IScxmlStateMachineState* state)
{
	qDebug() << "\033[0;34m" << QString("file(%1) line(%2) func(%3)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__);

	if (state)
	{
		this->OpenStateWidget(IScxmlStateMachineState::Pointer(state));
	}
}

void QLancetThaWorkbenchMenuBar::onStateMachineElementExit(IScxmlStateMachineState* state)
{
	qDebug() << "\033[0;34m" << QString("file(%1) line(%2) func(%3)").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__);
	if (state)
	{
		this->CloseStateWidget(IScxmlStateMachineState::Pointer(state));
	}
}

void QLancetThaWorkbenchMenuBar::on_action_clicked(bool)
{
	QAction* action = dynamic_cast<QAction*>(sender());
	if (action)
	{
		auto variant = action->data();
		if (variant.isNull() || false == variant.isValid()
			|| nullptr == variant.value<lancet::IScxmlStateMachineState*>())
		{
			qDebug() << "Warning ptr is null.";
			return;
		}
		lancet::IScxmlStateMachineState::Pointer state(
			variant.value<lancet::IScxmlStateMachineState*>());

		this->SwitchToSubState(state);
	}
}

void QLancetThaWorkbenchMenuBar::on_pushbtnApplicationIcon_clicked()
{
	if (this->GetStateMachineService().IsNotNull() 
		&& this->GetStateMachineService()->GetAdminState().IsNotNull())
	{
		this->GetStateMachineService()->GetAdminState()->GetStateMachineHandler()->ToTargetState("org_mitk_lancet_tha_medicalrecordmanagement");
	}
}

bool QLancetThaWorkbenchMenuBar::InitializeUiOfStateMachine(QMenu* menu,
	berry::SmartPointer<lancet::IScxmlStateMachineState> state)
{
	if (state.IsNotNull()&& lancet::plugin::IsLancetModuleFormat(state->GetStateId()))
	{
		this->ConnectToElementStateMachine(state);
		// Identify whether the nature of the state is single.
		// A single state is presented by QAction.
		// Multiple states are presented by QMenu.
		
		//if (false == this->IsSigleState(state))
		switch (state->GetStateFeatures())
		{
		case IScxmlStateMachineState::Multiple:
		{
			// Multiple states
			QMenu* multipleMenu = this->InitializeMenu(menu, state, true);

			// Create attached state set objects
			for (auto item : state->GetSubStateMachines())
			{
				// Filter other states.
				if (item.IsNull() || !lancet::plugin::IsLancetModuleFormat(item->GetStateId()))
				{
					continue;
				}
				
				this->InitializeUiOfStateMachine(multipleMenu, item);
			}
		}break;
		case IScxmlStateMachineState::Single:
		{
			// A single
			// Logically, this state has only one recognizable state.
			QAction* action = this->InitializeAction(menu, state);
		}break;
		default:
			break;
		}

		return true;
	}
	return false;
}

QMenu* QLancetThaWorkbenchMenuBar::InitializeMenu(QMenu* menu,
	berry::SmartPointer<lancet::IScxmlStateMachineState> state, bool recursion)
{
	QMenu* retval = 
		menu ? menu->addMenu("empty") : this->ui->menubarApplication->addMenu("empty");

	// setting property
	if (state.IsNotNull())
	{
		QString title = lancet::plugin::GetMitkPluginName(state->GetStateId());
		QString icon = "empty";
		QString object = lancet::plugin::GetMitkPluginName(state->GetStateId());

		if (state->GetActionProperty().IsNotNull())
		{
			title = state->GetActionProperty()->GetStateUiName();
			icon = state->GetActionProperty()->GetStateIcon();
			object = state->GetActionProperty()->GetStateObjectName();
		}
		retval->setTitle(title);
		retval->setIcon(QIcon(icon));
		retval->setObjectName(object);
	}
	return retval;
}

QAction* QLancetThaWorkbenchMenuBar::InitializeAction(QMenu* menu,
	berry::SmartPointer<lancet::IScxmlStateMachineState> state)
{
	QAction* retval = 
		menu ? menu->addAction("empty") : this->ui->menubarApplication->addAction("empty");

	connect(retval, SIGNAL(triggered(bool)), this, SLOT(on_action_clicked(bool)));
	// setting property 
	if (state.IsNotNull())
	{
		auto variant = QVariant::fromValue<lancet::IScxmlStateMachineState*>(state.GetPointer());
		retval->setData(variant);
		QString title = lancet::plugin::GetMitkPluginName(state->GetStateId());
		QString icon = "empty";
		QString object = lancet::plugin::GetMitkPluginName(state->GetStateId());

		if (state->GetActionProperty().IsNotNull())
		{
			title = state->GetActionProperty()->GetStateUiName();
			icon = state->GetActionProperty()->GetStateIcon();
			object = state->GetActionProperty()->GetStateObjectName();
		}
		retval->setText(title);
		retval->setIcon(QIcon(icon));
		retval->setObjectName(object);
	}
	return retval;
}

bool QLancetThaWorkbenchMenuBar::SwitchToSubState(
	berry::SmartPointer<lancet::IScxmlStateMachineState> state)
{
	bool retval = true;
	if (state.IsNotNull() && state->GetStateMachineHandler().IsNotNull())
	{
		if (state->GetManageStateMachine().IsNotNull())
		{
			retval &= SwitchToSubState(state->GetManageStateMachine());

			auto handle = state->GetManageStateMachine()->GetStateMachineHandler();
			if (handle.IsNotNull() && true == retval)
			{
				retval &= handle->ToTargetState(state);
			}
		}
	}
	return retval;
}

void QLancetThaWorkbenchMenuBar::ConnectToElementStateMachine(
	berry::SmartPointer<lancet::IScxmlStateMachineState> state)
{
	if (state.IsNotNull() && state->GetStateMachineHandler().IsNotNull()
		&& !state->GetSubStateMachines().empty())
	{
		if (state->GetActionProperty().IsNotNull())
		{
			qDebug() << state->GetStateId() << "-" << state->GetActionProperty()->GetStateObjectName() << " connect element signals";
		}
		else
		{
			qDebug() << state->GetStateId() << "-" << "unknown" << " connect element signals";
		}
		
		auto qtStateHandle = state->GetStateMachineHandler().GetPointer();
		connect(qtStateHandle, SIGNAL(StateEnter(IScxmlStateMachineState*)),
			this, SLOT(onStateMachineElementEnter(IScxmlStateMachineState*)));
		connect(qtStateHandle, SIGNAL(StateExit(IScxmlStateMachineState*)),
			this, SLOT(onStateMachineElementExit(IScxmlStateMachineState*)));
	}
}
