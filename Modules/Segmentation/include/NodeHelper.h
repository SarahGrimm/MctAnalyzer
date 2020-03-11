/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef NodeHelper_h
#define NodeHelper_h

#include <MitkCTSegmentationExports.h>
#include <mitkBaseData.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkSurface.h>
#include <mitkImage.h>

/**
 * \brief This class offers an interface for some mitk::NodePredicateProperty objects and mitk::DataNode objects.
 */
class MITKCTSEGMENTATION_EXPORT NodeHelper
{
public:

	/**
	 * \brief	Builds a base data node.
	 *
	 * \param	baseData	   	Like mitk::Surface or mitk::Image
	 * \param	binary		   	true to binary.
	 * \param	volumeRendering	true to volume rendering.
	 * \param 	name        	If non-null, the name.
	 * \param	opacity		   	The opacity.
	 * \param   color  	        If non-null, the color.
	 *
	 * \return	A mitk::DataNode::Pointer.
	 */
	static mitk::DataNode::Pointer BuildBaseDataNode(mitk::BaseData::Pointer baseData, bool binary, bool volumeRendering, const char* name, float opacity, mitk::ColorProperty* color);

	/**
	* \brief	Checks if the given datanode represent a mitk::Surface
	*
	* \param	dataNode	   	mitk::DataNode
	*
	* \return	bool
	*/
	static bool checkIsSurface(const mitk::DataNode::Pointer);

	/**
	* \brief	Checks if the given datanode represent a mitk::Surface
	*
	* \param	dataNode	   	mitk::DataNode
	*
	* \return	bool
	*/
	static bool checkIsSurface(const mitk::DataNode*);

	/**
	* \brief	Checks if the given datanode represent a mitk::Image
	*
	* \param	dataNode	   	mitk::DataNode
	*
	* \return	bool
	*/
	static bool checkIsImage(const mitk::DataNode::Pointer);

	/**
	* \brief	Checks if the given datanode represent a mitk::Image
	*
	* \param	dataNode	   	mitk::DataNode
	*
	* \return	bool
	*/
	static bool checkIsImage(const mitk::DataNode*);

	/** \brief	Predicates that a given node has the helper object property. */
	static mitk::NodePredicateProperty::Pointer HelperObject;

	/** \brief	Predicates that a given node has not the helper object property. */
	static mitk::NodePredicateNot::Pointer NoHelperObject;

	/** \brief	Predicates that a given node has the surface property. May not work, please use isSurface instead. */
	static mitk::NodePredicateProperty::Pointer Surface;

	/** \brief	Predicates that a given node has not the surface property. May not work, please use isNoSurface instead. */
	static mitk::NodePredicateNot::Pointer NoSurface;

	/** \brief	Predicates that a given node has the binary property with a true value. */
	static mitk::NodePredicateProperty::Pointer IsBinary;
	
	/** \brief	Predicates that a given node has the binary property with a false value. */
	static mitk::NodePredicateNot::Pointer IsNotBinary;

	/** \brief	Predicates that a given node has the clippingplane property with a true value. */
	static mitk::NodePredicateProperty::Pointer isClippingplane;

	/** \brief	Predicates that a given node has the clippingplane property with a false value. */
	static mitk::NodePredicateNot::Pointer IsNotClippingplane;

	/** \brief	Predicates that a given node has the clippingplane-arrow property with a true value. */
	static mitk::NodePredicateProperty::Pointer isClippingplaneArrow;

	/** \brief	Predicates that a given node has the clippingplane-arrow property with a false value. */
	static mitk::NodePredicateNot::Pointer IsNotClippingplaneArrow;

	/** \brief	Predicates that a given node has the cuttingTemplate property with a true value. */
	static mitk::NodePredicateProperty::Pointer IsCuttingTemplate;

	/** \brief	Predicates that a given node has the cuttingTemplate property with a false value. */
	static mitk::NodePredicateNot::Pointer IsNotCuttingTemplate;

	/** \brief	Predicates that a given node has the CuttedSegment property with a true value. */
	static mitk::NodePredicateProperty::Pointer IsCuttedSegment;

	/** \brief	Predicates that a given node has the CuttedSegment property with a false value. */
	static mitk::NodePredicateNot::Pointer IsNotCuttedSegment;

	/** \brief	Predicates that a given node has a surface as data. */
	static mitk::NodePredicateDataType::Pointer isSurface;

	/** \brief	Predicates that a given node has not surface as data. */
	static mitk::NodePredicateNot::Pointer isNoSurface;

	/** \brief	Predicates that a given node has the binary property. */
	static mitk::NodePredicateDataType::Pointer isBinary;

	/** \brief	Predicates that a given node has a pointSet as data. */
	static mitk::NodePredicateDataType::Pointer isPointSet;

	/** \brief	Predicates that a given node has a image as data. */
	static mitk::NodePredicateDataType::Pointer isImage;

	/** \brief	Predicates that a given node has the resectionplane property with true value. */
	static mitk::NodePredicateProperty::Pointer isResectionplane;

	/** \brief	Predicates that a given node has the skull property with true value. */
	static mitk::NodePredicateProperty::Pointer isSkull;

	/** \brief	Predicates that a given node has the mandibula property with true value. */
	static mitk::NodePredicateProperty::Pointer isMandibula;

	/** \brief	Predicates that a given node has the fibula property with true value. */
	static mitk::NodePredicateProperty::Pointer isFibula;

	/** \brief	Predicates that a given node has the vessel property with true value. */
	static mitk::NodePredicateProperty::Pointer isVessel;

	/** \brief	Predicates that a given node has the boundingobject property with true value. */
	static mitk::NodePredicateProperty::Pointer isBoundingobject;

	/** \brief	Predicates that a given node has the SinglePointSet property with true value. */
	static mitk::NodePredicateProperty::Pointer isSinglePointSet;

	/** \brief	Predicates that a given node has the MultiplePointSet property with true value. */
	static mitk::NodePredicateProperty::Pointer isMultiplePointSet;

	/** \brief	Predicates that a given node has the skin property with true value. */
	static mitk::NodePredicateProperty::Pointer isSkin;

	/** \brief	Predicates that a given node has the splinpoints property with true value. */
	static mitk::NodePredicateProperty::Pointer isSplintpoints;

	/** \brief	Predicates that a given node has the skinpoints property with true value. */
	static mitk::NodePredicateProperty::Pointer isSkinpoints;

	/**
	 *  Enables or disables the three crosshair planes in all render windows.
	 *
	 * \param	visibility 	if visible or not
	 * \param	dateStorage	The date storage.
	 */
	static void NodeHelper::setCrosshairVisibility(bool visibility, mitk::DataStorage::Pointer dateStorage);

	/**
	 *	Returns a list of all Surfaces from the datastorage for the given predicate.
	 *
	 * \param dataStorage	the dataStorage with all dataNodes
	 * \param predicate		for filtering the storage
	 * \param predicateFlag	if predicate is true or false
	*/
	static std::vector<mitk::Surface*> NodeHelper::getSurfaces(const mitk::DataStorage::Pointer dataStorage, const char *predicate, bool predicateFlag);

	/**
	* \brief Creates a mitk::DataNode with the given mitk::Surface reference and set surface specific properties.
	*
	* \param surface	the surface to display
	* \param name		the name
	*/
	static mitk::DataNode::Pointer CreateSurfaceNode(mitk::Surface* surface, const char* name);

	/**
	* \brief Creates a mitk::DataNode with the given mitk::Image reference and set segmentation specific properties like binary = true.
	*
	* \param segmentation	the segmentation as binary image to display
	* \param name			the name
	*/
	static mitk::DataNode::Pointer CreateSegmentatioNode(mitk::Image* segmentation, const char* name);

	/**
	* \brief Creates a mitk::DataNode with the given mitk::Image reference and set image specific properties.
	*
	* \param rawImage	the image as gray value image to display
	* \param name		the name
	*/
	static mitk::DataNode::Pointer CreateImageNode(mitk::Image* rawImage, const char* name);

	/**
	 * \brief	Imports the segmentation data given by the absolute file path.
	 *
	 * \param	path Full pathname of the file.
	 *
	 * \return	A mitk::DataNode::Pointer which contains the segmentation or NULL if it was not possible to read the data given by the path
	 */
	static mitk::DataNode::Pointer ImportSegmentation(const std::string path);

	/**
	 * \brief	Builds a mitk::DataNode which has the NodeHelper::HelperObject property
	 *
	 * \param	baseData like a mitk::PointSet
	 * \param   name 	If non-null, the name.
	 * \param	opacity		 	The opacity.
	 * \param 	color	If non-null, the color.
	 *
	 * \return	A mitk::DataNode::Pointer.
	 */
	static mitk::DataNode::Pointer BuildHelperNode(mitk::BaseData::Pointer baseData, char* name, float opacity, mitk::ColorProperty* color);


private:

};

#endif