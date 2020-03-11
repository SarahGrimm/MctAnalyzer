
#ifndef BOUNDINGOBJECTTEMPLATE_H_HEADER_INCLUDED
#define BOUNDINGOBJECTTEMPLATE_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
#include <MitkCTSegmentationExports.h>
#include <mitkPointSet.h>
#include <vtkSelectEnclosedPoints.h>

//##Documentation
//## @brief Data class containing an cylinder
//## @ingroup Data
class MITKCTSEGMENTATION_EXPORT BoundingObjectTemplate : public mitk::BoundingObject
{

private:
	vtkSmartPointer<vtkPolyData> boundingObject;
	double Bounds[6];
	double Length;
	vtkCellLocator *CellLocator;
	vtkIdList      *CellIds;
	vtkGenericCell *Cell;
	double* Center;
public:
  mitkClassMacro(BoundingObjectTemplate, BoundingObject);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual mitk::ScalarType GetVolume() override;
  virtual bool IsInside(const mitk::Point3D& p)  const override;
 
  void setVTKData(vtkPolyData* pd);
 
  
  //virtual void UpdateOutputInformation();
  BoundingObjectTemplate();
  ~BoundingObjectTemplate();
protected:
	vtkSmartPointer<vtkSelectEnclosedPoints> selectEnclosedPoints;
};

#endif /* MITKCONE_H_HEADER_INCLUDED */
