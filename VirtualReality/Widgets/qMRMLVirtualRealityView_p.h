/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLVirtualRealityView_p_h
#define __qMRMLVirtualRealityView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// VTK includes
//#include <vtkOpenGLFramebufferObject.h>
#include <vtkEventData.h>
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLVirtualRealityView.h"

// Qt includes
#include <QTimer>

class QLabel;
class vtkLightCollection;
class vtkMRMLCameraNode;
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLVirtualRealityViewNode;
class vtkObject;
class vtkVirtualRealityViewInteractorStyle;
class vtkTimerLog;

//-----------------------------------------------------------------------------
class qMRMLVirtualRealityViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLVirtualRealityView);
protected:
  qMRMLVirtualRealityView* const q_ptr;
public:
  qMRMLVirtualRealityViewPrivate(qMRMLVirtualRealityView& object);
  ~qMRMLVirtualRealityViewPrivate();

  virtual void init();

  void setMRMLScene(vtkMRMLScene* scene);
  void startVirtualReality();
  void stopVirtualReality();

  double desiredUpdateRate();
  double stillUpdateRate();

public slots:
  void updateWidgetFromMRML();
  void doOpenVirtualReality();

protected:
  void updateTransformNodeWithControllerPose(vtkEventDataDevice device);
  void createRenderWindow();
  void destroyRenderWindow();

  vtkSlicerCamerasModuleLogic* CamerasLogic;

  vtkSmartPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagerGroup;
  vtkWeakPointer<vtkMRMLScene> MRMLScene;
  vtkWeakPointer<vtkMRMLVirtualRealityViewNode> MRMLVirtualRealityViewNode;
  vtkSmartPointer<vtkOpenVRRenderer> Renderer;
  vtkSmartPointer<vtkOpenVRRenderWindow> RenderWindow;
  vtkSmartPointer<vtkOpenVRRenderWindowInteractor> Interactor;
  vtkSmartPointer<vtkVirtualRealityViewInteractorStyle> InteractorStyle;
  vtkSmartPointer<vtkOpenVRCamera> Camera;
  vtkSmartPointer<vtkLightCollection> Lights;

  vtkSmartPointer<vtkTimerLog> LastViewUpdateTime;
  double LastViewDirection[3];
  double LastViewUp[3];
  double LastViewPosition[3];

  QTimer VirtualRealityLoopTimer;
};

#endif
