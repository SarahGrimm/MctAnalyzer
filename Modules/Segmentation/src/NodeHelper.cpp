#include "NodeHelper.h"

// MITK
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include <mitkNodePredicateDataType.h>
#include <mitkITKImageImport.h>

// QMITK
#include <QmitkIOUtil.h>

// ITK
#include "itkImage.h"


const char* NODE_PROPERTY_BINARY_KEY = "binary";
const char* NODE_PROPERTY_SURFACE_KEY = "Surface";
const char* NODE_PROPERTY_HELPER_KEY = "helper object";
const char* NODE_PROPERTY_VISIBLE = "visible";
const char* NODE_PROPERTY_CLIPPINGPLANE = "clippingPlane";
const char* NODE_PROPERTY_CUTTINGTEMPLATE = "cuttingTemplate";
const char* NODE_PROPERTY_CUTTEDSEGMENT = "cuttedSegment";
const char* NODE_PROPERTY_CLIPPINGPLANEARROW = "arrowClippingplane";
const char* NODE_PROPERTY_SURFACE = "Surface";
const char* NODE_PROPERTY_IMAGE = "Image";
const char* NODE_PROPERTY_PointSet = "PointSet";
const char* NODE_PROPERTY_RESECTIONPLANE = "resectionplane";
const char* NODE_PROPERTY_SKULL = "skull";
const char* NODE_PROPERTY_MANDIBULA = "mandibula";
const char* NODE_PROPERTY_FIBULA = "fibula";
const char* NODE_PROPERTY_VESSEL = "vessel";
const char* NODE_PROPERTY_BOUNDINGOBJECT = "boundingObject";
const char* NODE_PROPERTY_SINGLEPOINTSET = "SinglePointSet";
const char* NODE_PROPERTY_MULTIPLEPOINTSET = "MultiplePointSet";
const char* NODE_PROPERTY_SKIN = "skin";
const char* NODE_PROPERTY_SPLINTPOINTS = "splintpoints";
const char* NODE_PROPERTY_SKINPOINTS = "skinpoints";


mitk::NodePredicateProperty::Pointer NodeHelper::HelperObject = mitk::NodePredicateProperty::New(NODE_PROPERTY_HELPER_KEY);
mitk::NodePredicateNot::Pointer NodeHelper::NoHelperObject = mitk::NodePredicateNot::New(HelperObject);

// may not work, use instead the predicate data type version
mitk::NodePredicateProperty::Pointer NodeHelper::Surface = mitk::NodePredicateProperty::New(NODE_PROPERTY_SURFACE_KEY);
mitk::NodePredicateNot::Pointer NodeHelper::NoSurface = mitk::NodePredicateNot::New(Surface);

mitk::NodePredicateProperty::Pointer NodeHelper::IsBinary = mitk::NodePredicateProperty::New(NODE_PROPERTY_BINARY_KEY, mitk::BoolProperty::New(true));
mitk::NodePredicateNot::Pointer NodeHelper::IsNotBinary = mitk::NodePredicateNot::New(IsBinary);

mitk::NodePredicateProperty::Pointer NodeHelper::isClippingplane = mitk::NodePredicateProperty::New(NODE_PROPERTY_CLIPPINGPLANE, mitk::BoolProperty::New(true));
mitk::NodePredicateNot::Pointer NodeHelper::IsNotClippingplane = mitk::NodePredicateNot::New(isClippingplane);

mitk::NodePredicateProperty::Pointer NodeHelper::isClippingplaneArrow = mitk::NodePredicateProperty::New(NODE_PROPERTY_CLIPPINGPLANEARROW, mitk::BoolProperty::New(true));
mitk::NodePredicateNot::Pointer NodeHelper::IsNotClippingplaneArrow = mitk::NodePredicateNot::New(isClippingplaneArrow);

mitk::NodePredicateProperty::Pointer NodeHelper::IsCuttingTemplate = mitk::NodePredicateProperty::New(NODE_PROPERTY_CUTTINGTEMPLATE, mitk::BoolProperty::New(true));
mitk::NodePredicateNot::Pointer NodeHelper::IsNotCuttingTemplate = mitk::NodePredicateNot::New(IsCuttingTemplate);

mitk::NodePredicateProperty::Pointer NodeHelper::IsCuttedSegment = mitk::NodePredicateProperty::New(NODE_PROPERTY_CUTTEDSEGMENT, mitk::BoolProperty::New(true));
mitk::NodePredicateNot::Pointer NodeHelper::IsNotCuttedSegment = mitk::NodePredicateNot::New(IsCuttedSegment);

mitk::NodePredicateDataType::Pointer NodeHelper::isSurface = mitk::NodePredicateDataType::New(NODE_PROPERTY_SURFACE);
mitk::NodePredicateNot::Pointer NodeHelper::isNoSurface = mitk::NodePredicateNot::New(isSurface);

mitk::NodePredicateDataType::Pointer NodeHelper::isBinary = mitk::NodePredicateDataType::New(NODE_PROPERTY_BINARY_KEY);

mitk::NodePredicateDataType::Pointer NodeHelper::isPointSet = mitk::NodePredicateDataType::New(NODE_PROPERTY_PointSet);

mitk::NodePredicateDataType::Pointer NodeHelper::isImage = mitk::NodePredicateDataType::New(NODE_PROPERTY_IMAGE);

mitk::NodePredicateProperty::Pointer NodeHelper::isResectionplane = mitk::NodePredicateProperty::New(NODE_PROPERTY_RESECTIONPLANE, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isSkull = mitk::NodePredicateProperty::New(NODE_PROPERTY_SKULL, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isMandibula = mitk::NodePredicateProperty::New(NODE_PROPERTY_MANDIBULA, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isFibula = mitk::NodePredicateProperty::New(NODE_PROPERTY_FIBULA, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isVessel = mitk::NodePredicateProperty::New(NODE_PROPERTY_VESSEL, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isBoundingobject = mitk::NodePredicateProperty::New(NODE_PROPERTY_BOUNDINGOBJECT, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isSinglePointSet = mitk::NodePredicateProperty::New(NODE_PROPERTY_SINGLEPOINTSET, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isMultiplePointSet = mitk::NodePredicateProperty::New(NODE_PROPERTY_MULTIPLEPOINTSET, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isSkin = mitk::NodePredicateProperty::New(NODE_PROPERTY_SKIN, mitk::BoolProperty::New(true));

mitk::NodePredicateProperty::Pointer NodeHelper::isSplintpoints = mitk::NodePredicateProperty::New(NODE_PROPERTY_SPLINTPOINTS);

mitk::NodePredicateProperty::Pointer NodeHelper::isSkinpoints = mitk::NodePredicateProperty::New(NODE_PROPERTY_SKINPOINTS);

mitk::DataNode::Pointer NodeHelper::BuildBaseDataNode(mitk::BaseData::Pointer baseData, bool binary, bool volumeRendering, const char* name, float opacity, mitk::ColorProperty* color){
	mitk::DataNode::Pointer newNode = mitk::DataNode::New();
	if (baseData)
		newNode->SetData(baseData);

	// set some properties
	newNode->SetProperty("binary", mitk::BoolProperty::New(binary));
	newNode->SetName(name);
	newNode->SetProperty("color", color);
	newNode->SetProperty("volumerendering", mitk::BoolProperty::New(volumeRendering));
	newNode->SetOpacity(opacity);
	return newNode;
}

bool NodeHelper::checkIsSurface(const mitk::DataNode::Pointer node){
	mitk::NodePredicateDataType::Pointer predicate = mitk::NodePredicateDataType::New(NODE_PROPERTY_SURFACE);
	return predicate->CheckNode(node);
}

bool NodeHelper::checkIsSurface(const mitk::DataNode *node){
	mitk::NodePredicateDataType::Pointer predicate = mitk::NodePredicateDataType::New(NODE_PROPERTY_SURFACE);
	return predicate->CheckNode(node);
}

bool NodeHelper::checkIsImage(const mitk::DataNode::Pointer node){
	mitk::NodePredicateDataType::Pointer predicate = mitk::NodePredicateDataType::New(NODE_PROPERTY_IMAGE);
	return predicate->CheckNode(node);
}

bool NodeHelper::checkIsImage(const mitk::DataNode *node){
	mitk::NodePredicateDataType::Pointer predicate = mitk::NodePredicateDataType::New(NODE_PROPERTY_IMAGE);
	return predicate->CheckNode(node);
}

void NodeHelper::setCrosshairVisibility(bool visibility, mitk::DataStorage::Pointer dateStorage){
	mitk::DataNode::Pointer widgetNode = dateStorage->GetNamedNode("Widgets");

	typedef itk::VectorContainer<unsigned int, mitk::DataNode::Pointer> VectorContainerType;
	VectorContainerType::ConstPointer vc = dateStorage->GetDerivations(widgetNode, NULL, true);

	VectorContainerType::ConstIterator stdPlane = vc->Begin();

	while (stdPlane != vc->End()){
		stdPlane->Value()->SetVisibility(visibility);
		stdPlane++;
	}
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

std::vector<mitk::Surface*> NodeHelper::getSurfaces(const mitk::DataStorage::Pointer dataStorage, const char *predicate, bool predicateFlag){
	std::vector<mitk::Surface*> surfaces;
	mitk::NodePredicateProperty::Pointer storagePredicate = mitk::NodePredicateProperty::New(predicate, mitk::BoolProperty::New(predicateFlag));
	mitk::DataStorage::SetOfObjects::ConstPointer allSurfaces = dataStorage->GetSubset(storagePredicate);
	for (mitk::DataStorage::SetOfObjects::ConstIterator itAllSurfaces = allSurfaces->Begin(); itAllSurfaces != allSurfaces->End(); itAllSurfaces++)
	{
		mitk::Surface *surface = dynamic_cast<mitk::Surface*>(itAllSurfaces.Value()->GetData());
		surfaces.push_back(surface);
	}
	return surfaces;
}

mitk::DataNode::Pointer NodeHelper::CreateSurfaceNode(mitk::Surface* surface, const char* name){
	mitk::DataNode::Pointer surfaceNode = BuildBaseDataNode(surface, false, false, name, 1.0, mitk::ColorProperty::New(0.8745, 0.80392, 0.407843));

	surfaceNode->SetProperty("material.ambientCoefficient", mitk::FloatProperty::New(0.01));
	//surfaceNode->ReplaceProperty("material.interpolation", mitk::StringProperty::New("Gouraud"));
	surfaceNode->SetProperty("material.specularCoefficient", mitk::FloatProperty::New(0.7));
	return surfaceNode;
}

mitk::DataNode::Pointer NodeHelper::CreateSegmentatioNode(mitk::Image* segmentation, const char* name){
	return BuildBaseDataNode(segmentation, true, true, name, 0.67, mitk::ColorProperty::New(0.8745, 0.80392, 0.407843));
}

mitk::DataNode::Pointer NodeHelper::CreateImageNode(mitk::Image* rawImage, const char* name){
	return BuildBaseDataNode(rawImage, false, false, name, 1.0, mitk::ColorProperty::New(1, 1, 1));
}

mitk::DataNode::Pointer NodeHelper::ImportSegmentation(const std::string path){
	mitk::BaseData::Pointer segmentation = mitk::IOUtil::Load(path).front();
	mitk::DataNode::Pointer node;
	if (segmentation){
		node = BuildBaseDataNode(segmentation, true, true, "", 1.0, mitk::ColorProperty::New(1, 1, 1));
		std::size_t lastSeperator = path.find_last_of('/');
		std::string file = path.substr(lastSeperator + 1);
		std::size_t formatDot = file.find_last_of(".");
		file = file.substr(0, formatDot);
		node->SetName(file);
		MITK_INFO << "Segmentation '" << file << "' was successful loaded";
	}
	else{
		MITK_ERROR << "Segmentation with path '" << path << "' could not be loaded successfully";
	}
	return node;
}

mitk::DataNode::Pointer NodeHelper::BuildHelperNode(mitk::BaseData::Pointer baseData, char* name, float opacity, mitk::ColorProperty* color){
	mitk::DataNode::Pointer newNode = NodeHelper::BuildBaseDataNode(baseData, false, false, name, opacity, color);
	
	// set special properties
	newNode->SetProperty("helper object", mitk::BoolProperty::New(true));
	return newNode;
}
