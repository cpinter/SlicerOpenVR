/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QAction>
#include <QDebug>
#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include <QToolBar>
#include <QtPlugin>

// VirtualReality Logic includes
#include <vtkSlicerVirtualRealityLogic.h>

// VirtualReality includes
#include "qSlicerVirtualRealityModule.h"
#include "qSlicerVirtualRealityModuleWidget.h"
#include "qMRMLVirtualRealityView.h"
#include "vtkMRMLVirtualRealityViewNode.h"

#include "qSlicerApplication.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerCamerasModuleLogic.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerVirtualRealityModule, qSlicerVirtualRealityModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerVirtualRealityModulePrivate
{
  Q_DECLARE_PUBLIC(qSlicerVirtualRealityModule);
protected:
  qSlicerVirtualRealityModule* const q_ptr;
public:
  qSlicerVirtualRealityModulePrivate(qSlicerVirtualRealityModule& object);
  virtual ~qSlicerVirtualRealityModulePrivate();

  /// Adds Virtual Reality toolbar to the application GUI (toolbar and menu)
  void addToolBar();

  /// Updated toolbar button states.
  void updateToolBar();

  /// Adds Virtual Reality view widget
  void addViewWidget();

  vtkSlicerVirtualRealityLogic* logic();

  QToolBar* ToolBar;
  QAction* VirtualRealityToggleAction;
  QAction* UpdateViewFromReferenceViewCameraAction;
  QAction* ConfigureAction;
  qMRMLVirtualRealityView* VirtualRealityViewWidget;
};

//-----------------------------------------------------------------------------
// qSlicerVirtualRealityModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModulePrivate::qSlicerVirtualRealityModulePrivate(qSlicerVirtualRealityModule& object)
  : q_ptr(&object)
  , ToolBar(NULL)
  , VirtualRealityToggleAction(NULL)
  , UpdateViewFromReferenceViewCameraAction(NULL)
  , ConfigureAction(NULL)
  , VirtualRealityViewWidget(NULL)
{
}

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModulePrivate::~qSlicerVirtualRealityModulePrivate()
{
  delete this->VirtualRealityViewWidget;
  this->VirtualRealityViewWidget = NULL;
}

//-----------------------------------------------------------------------------
vtkSlicerVirtualRealityLogic* qSlicerVirtualRealityModulePrivate::logic()
{
  Q_Q(qSlicerVirtualRealityModule);
  return vtkSlicerVirtualRealityLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModulePrivate::addViewWidget()
{
  Q_Q(qSlicerVirtualRealityModule);

  if (this->VirtualRealityViewWidget != NULL)
  {
    return;
  }

  this->VirtualRealityViewWidget = new qMRMLVirtualRealityView();
  this->VirtualRealityViewWidget->setObjectName(QString("VirtualRealityWidget"));

  qSlicerAbstractCoreModule* camerasModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Cameras");
  if (camerasModule)
  {
    vtkSlicerCamerasModuleLogic* camerasLogic = vtkSlicerCamerasModuleLogic::SafeDownCast(camerasModule->logic());
    this->VirtualRealityViewWidget->setCamerasLogic(camerasLogic);
  }
  else
  {
    qWarning() << "Cameras module is not found";
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModulePrivate::addToolBar()
{
  Q_Q(qSlicerVirtualRealityModule);

  QMainWindow* mainWindow = qSlicerApplication::application()->mainWindow();
  if (mainWindow == NULL)
  {
    qDebug("qSlicerVirtualRealityModulePrivate::addToolBar: no main window is available, toolbar is not added");
    return;
  }

  if (!this->ToolBar)
  {
    this->ToolBar = new QToolBar;
    this->ToolBar->setWindowTitle(QObject::tr("Virtual Reality"));
    this->ToolBar->setObjectName("VirtualRealityToolBar");

    this->VirtualRealityToggleAction = this->ToolBar->addAction(QObject::tr("Show scene in virtual reality."));
    this->VirtualRealityToggleAction->setIcon(QIcon(":/Icons/VirtualRealityHeadset.png"));
    this->VirtualRealityToggleAction->setCheckable(true);
    QObject::connect(this->VirtualRealityToggleAction, SIGNAL(toggled(bool)),
      q, SLOT(enableVirtualReality(bool)));

    this->UpdateViewFromReferenceViewCameraAction = this->ToolBar->addAction(QObject::tr("Set virtual reality view to match reference view."));
    this->UpdateViewFromReferenceViewCameraAction->setIcon(QIcon(":/Icons/ViewCenter.png"));
    QObject::connect(this->UpdateViewFromReferenceViewCameraAction, SIGNAL(triggered()),
      q, SLOT(updateViewFromReferenceViewCamera()));

    this->ToolBar->addSeparator();

    this->ConfigureAction = this->ToolBar->addAction(QObject::tr("Configure virtual reality settings."));
    this->ConfigureAction->setIcon(QIcon(":/Icons/Small/SlicerConfigure.png"));
    QObject::connect(this->ConfigureAction, SIGNAL(triggered()),
      q, SLOT(switchToVirtualRealityModule()));

    mainWindow->addToolBar(this->ToolBar);
  }

  // Add toolbar show/hide option to menu
  foreach(QMenu* toolBarMenu, mainWindow->findChildren<QMenu*>())
  {
    if (toolBarMenu->objectName() == QString("WindowToolBarsMenu"))
    {
      toolBarMenu->addAction(this->ToolBar->toggleViewAction());
      break;
    }
  }

  // Main window takes care of saving and restoring toolbar geometry and state.
  // However, when state is restored the virtual reality toolbar was not created yet.
  // We need to restore the main window state again, now, that the Sequences toolbar is available.
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
  {
    mainWindow->restoreState(settings.value("windowState").toByteArray());
  }
  settings.endGroup();
}


//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModulePrivate::updateToolBar()
{
  Q_Q(qSlicerVirtualRealityModule);

  if (!this->VirtualRealityToggleAction)
  {
    return;
  }

  vtkMRMLVirtualRealityViewNode* vrViewNode = this->logic()->GetVirtualRealityViewNode();

  bool wasBlocked = this->VirtualRealityToggleAction->blockSignals(true);
  if (!vrViewNode)
  {
    this->VirtualRealityToggleAction->setChecked(false);
    this->VirtualRealityToggleAction->setIcon(QIcon(":/Icons/VirtualRealityHeadset.png"));
  }
  else
  {
    this->VirtualRealityToggleAction->setChecked(vrViewNode->GetVisibility() && vrViewNode->GetActive());
    if (vrViewNode->HasError())
    {
      this->VirtualRealityToggleAction->setIcon(QIcon(":/Icons/VirtualRealityHeadsetError.png"));
    }
    else
    {
      this->VirtualRealityToggleAction->setIcon(QIcon(":/Icons/VirtualRealityHeadset.png"));
    }
  }
  this->VirtualRealityToggleAction->blockSignals(wasBlocked);

  this->UpdateViewFromReferenceViewCameraAction->setEnabled(this->VirtualRealityToggleAction->isChecked());
}

//-----------------------------------------------------------------------------
// qSlicerVirtualRealityModule methods

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModule::qSlicerVirtualRealityModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVirtualRealityModulePrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModule::~qSlicerVirtualRealityModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVirtualRealityModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerVirtualRealityModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVirtualRealityModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerVirtualRealityModule::icon() const
{
  return QIcon(":/Icons/VirtualReality.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVirtualRealityModule::categories() const
{
  return QStringList() << "Virtual Reality";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVirtualRealityModule::dependencies() const
{
  return QStringList() << "Cameras";
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModule::setup()
{
  Q_D(qSlicerVirtualRealityModule);
  this->Superclass::setup();
  d->addToolBar();
  d->addViewWidget();
  // If virtual reality logic is modified it indicates that the view node may changed
  qvtkConnect(logic(), vtkCommand::ModifiedEvent, this, SLOT(onViewNodeModified()));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerVirtualRealityModule::createWidgetRepresentation()
{
  return new qSlicerVirtualRealityModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVirtualRealityModule::createLogic()
{
  return vtkSlicerVirtualRealityLogic::New();
}

//-----------------------------------------------------------------------------
QToolBar* qSlicerVirtualRealityModule::toolBar()
{
  Q_D(qSlicerVirtualRealityModule);
  return d->ToolBar;
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModule::setToolBarVisible(bool visible)
{
  Q_D(qSlicerVirtualRealityModule);
  d->ToolBar->setVisible(visible);
}

//-----------------------------------------------------------------------------
bool qSlicerVirtualRealityModule::isToolBarVisible()
{
  Q_D(qSlicerVirtualRealityModule);
  return d->ToolBar->isVisible();
}

// --------------------------------------------------------------------------
void qSlicerVirtualRealityModule::onViewNodeModified()
{
  Q_D(qSlicerVirtualRealityModule);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->logic()->GetVirtualRealityViewNode();

  // Update view node in view widget
  if (d->VirtualRealityViewWidget != NULL)
  {
    vtkMRMLVirtualRealityViewNode* oldVrViewNode = d->VirtualRealityViewWidget->mrmlVirtualRealityViewNode();
    if (oldVrViewNode != vrViewNode)
    {
      d->logic()->SetDefaultReferenceView();
    }
    d->VirtualRealityViewWidget->setMRMLVirtualRealityViewNode(vrViewNode);
  }

  // Update toolbar
  d->updateToolBar();
}

// --------------------------------------------------------------------------
void qSlicerVirtualRealityModule::enableVirtualReality(bool enable)
{
  Q_D(qSlicerVirtualRealityModule);
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  d->logic()->SetVirtualRealityActive(enable);
  QApplication::restoreOverrideCursor();
}

// --------------------------------------------------------------------------
void qSlicerVirtualRealityModule::updateViewFromReferenceViewCamera()
{
  Q_D(qSlicerVirtualRealityModule);
  if (d->VirtualRealityViewWidget == NULL)
  {
    return;
  }
  d->VirtualRealityViewWidget->updateViewFromReferenceViewCamera();
}

// --------------------------------------------------------------------------
void qSlicerVirtualRealityModule::switchToVirtualRealityModule()
{
  Q_D(qSlicerVirtualRealityModule);
  if (!qSlicerApplication::application()
    || !qSlicerApplication::application()->moduleManager())
  {
    return;
  }
  qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("VirtualReality");
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (!module)
  {
    return;
  }
  moduleWithAction->action()->trigger();
}

// --------------------------------------------------------------------------
qMRMLVirtualRealityView* qSlicerVirtualRealityModule::viewWidget()
{
  Q_D(qSlicerVirtualRealityModule);
  return d->VirtualRealityViewWidget;
}