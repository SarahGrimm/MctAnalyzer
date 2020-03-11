#include "boundingObjectTemplate.h"
#include "vtkLinearTransform.h"
#include "mitkNumericTypes.h"
#include "vtkConeSource.h"
#include <vtkDelaunay3D.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkDelaunay2D.h>
#include <vtkGeometryFilter.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkButterflySubdivisionFilter.h>
#include <vtkCenterOfMass.h>
#include <vtkMassProperties.h>
#include "vtkCellLocator.h"
#include "vtkGenericCell.h"
#include "vtkMath.h"


BoundingObjectTemplate::BoundingObjectTemplate()
: BoundingObject()
{
	this->selectEnclosedPoints = vtkSmartPointer<vtkSelectEnclosedPoints>::New();
}

BoundingObjectTemplate::~BoundingObjectTemplate()
{
	
	this->selectEnclosedPoints->Complete();
}



void BoundingObjectTemplate::setVTKData(vtkPolyData* pd) {
	MITK_WARN << pd->GetNumberOfCells();
	boundingObject = pd;
	pd->GetBounds(this->Bounds);
	this->Length = pd->GetLength();
	
		this->CellLocator = vtkCellLocator::New();
	
	this->CellIds = vtkIdList::New();
	this->Cell = vtkGenericCell::New();

	MITK_WARN << "LENGTH" << this->Length;

	// Set up structures for acceleration ray casting
	this->CellLocator->SetDataSet(pd);
	this->CellLocator->BuildLocator();


	vtkSmartPointer<vtkCenterOfMass> centerOfMassFilter =
		vtkSmartPointer<vtkCenterOfMass>::New();

	centerOfMassFilter->SetInputData(pd);

	centerOfMassFilter->SetUseScalarsAsWeights(false);
	centerOfMassFilter->Update();


	this->Center = centerOfMassFilter->GetCenter();

	//std::cout << "Center of mass is " << center[0] << " " << center[1] << " " << center[2] << std::endl;

	SetVtkPolyData(pd);
}

bool BoundingObjectTemplate::IsInside(const mitk::Point3D& worldPoint) const
{
	double point[3];
	
	point[0] = worldPoint[0];
	point[1] = worldPoint[1];
	point[2] = worldPoint[2];
	//MITK_WARN << "Hallo";
	/*this->selectEnclosedPoints->Initialize(boundingObject);
	
	

	bool hi =  this->selectEnclosedPoints->IsInsideSurface(point);*/
	if (point[0] < this->Bounds[0] || point[0] > this->Bounds[1] ||
		point[1] < this->Bounds[2] || point[1] > this->Bounds[3] ||
		point[2] < this->Bounds[4] || point[2] > this->Bounds[5])
	{
		return 0;
	}

	//  Perform in/out by shooting random rays. Multiple rays are fired
	//  to improve accuracy of the result.
	//
	//  The variable iterNumber counts the number of rays fired and is
	//  limited by the defined variable VTK_MAX_ITER.
	//
	//  The variable deltaVotes keeps track of the number of votes for
	//  "in" versus "out" of the surface.  When deltaVotes > 0, more votes
	//  have counted for "in" than "out".  When deltaVotes < 0, more votes
	//  have counted for "out" than "in".  When the delta_vote exceeds or
	//  equals the defined variable VTK_VOTE_THRESHOLD, then the
	//  appropriate "in" or "out" status is returned.
	//
	double rayMag, ray[3], xray[3], t, pcoords[3], xint[3];
	int i, numInts, iterNumber, deltaVotes, subId;
	vtkIdType idx, numCells;
	double tol = 0.001*this->Length;

	
		//  Define a random ray to fire.
	
		// The ray must be appropriately sized wrt the bounding box. (It has to go
		// all the way through the bounding box.)
		

		// Retrieve the candidate cells from the locator
		this->CellLocator->FindCellsAlongLine(point, this->Center, tol, this->CellIds);

		// Intersect the line with each of the candidate cells
		numInts = 0;
		numCells = this->CellIds->GetNumberOfIds();
		for (idx = 0; idx < numCells; idx++)
		{
			boundingObject->GetCell(this->CellIds->GetId(idx), this->Cell);
			if (this->Cell->IntersectWithLine(point, this->Center, tol, t, xint, pcoords, subId))
			{
				numInts++;
			}
		} //for all candidate cells

		// Count the result
	
			if (numInts == 0){
				++deltaVotes;
			}
			else{
				--deltaVotes;
			}
			
		
	 //try another ray

	//   If the number of votes is positive, the point is inside
	//
	return (deltaVotes < 0 ? 0 : 1);
}



mitk::ScalarType BoundingObjectTemplate::GetVolume()
{

	vtkSmartPointer<vtkMassProperties> centerOfMassFilter =
		vtkSmartPointer<vtkMassProperties>::New();

	centerOfMassFilter->SetInputData(GetVtkPolyData());
	centerOfMassFilter->Update();

	return centerOfMassFilter->GetVolume();
	
}
