#ifndef GREYVALUECUTTER_H_HEADER_INCLUDED_C10B22CD
#define GREYVALUECUTTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include <MitkCTSegmentationExports.h>
#include "mitkImageToImageFilter.h"
#include "mitkBoundingObject.h"
#include "mitkImageTimeSelector.h"
#include "itkImage.h"



class MITKCTSEGMENTATION_EXPORT GreyValueCutter : public mitk::ImageToImageFilter
{
public:
	mitkClassMacro(GreyValueCutter, ImageToImageFilter);
	itkFactorylessNewMacro(Self) itkCloneMacro(Self)

	void SetBoundingObject(const mitk::BoundingObject *boundingObject);
	const mitk::BoundingObject *GetBoundingObject() const;
	//##Description
	//## @brief Set for inside pixels, used when m_UseInsideValue is @a true
	itkSetMacro(InsideValue, mitk::ScalarType);
	itkGetMacro(InsideValue, mitk::ScalarType);
	//##Description
	//## @brief Set value for outside pixels, used when m_AutoOutsideValue is \a false
	itkSetMacro(OutsideValue, mitk::ScalarType);
	itkGetMacro(OutsideValue, mitk::ScalarType);
	itkSetMacro(UseInsideValue, bool);
	itkGetMacro(UseInsideValue, bool);
	itkBooleanMacro(UseInsideValue);
	//##Description
	//## @brief If set to \a true the minimum of the ouput pixel type is
	//## used as outside value.
	itkSetMacro(AutoOutsideValue, bool);
	itkGetMacro(AutoOutsideValue, bool);
	itkBooleanMacro(AutoOutsideValue);

	itkGetMacro(InsidePixelCount, unsigned int);
	itkGetMacro(OutsidePixelCount, unsigned int);

	itkSetMacro(UseWholeInputRegion, bool);
	itkGetMacro(UseWholeInputRegion, bool);

protected:
	GreyValueCutter();
	virtual ~GreyValueCutter();

	virtual const mitk::PixelType GetOutputPixelType();

	virtual void GenerateInputRequestedRegion() override;
	virtual void GenerateOutputInformation() override;
	virtual void GenerateData() override;

	template <typename TPixel, unsigned int VImageDimension, typename TOutputPixel>
	friend void CutImageWithOutputTypeSelect(itk::Image<TPixel, VImageDimension> *inputItkImage,
		GreyValueCutter *cutter,
		int boTimeStep,
		TOutputPixel *dummy);
	template <typename TPixel, unsigned int VImageDimension, typename TOutputPixel>
	friend void CutImageWithOutputTypeSelect(itk::VectorImage<TPixel, VImageDimension> *inputItkImage,
		GreyValueCutter *cutter,
		int boTimeStep,
		TOutputPixel *dummy);
	template <typename TPixel, unsigned int VImageDimension>
	friend void CutImage(itk::Image<TPixel, VImageDimension> *itkImage,
		GreyValueCutter *cutter,
		int boTimeStep);
	template <typename TPixel, unsigned int VImageDimension>
	friend void CutImage(itk::VectorImage<TPixel, VImageDimension> *itkImage,
		GreyValueCutter *cutter,
		int boTimeStep);
	virtual void ComputeData(mitk::Image *input3D, int boTimeStep);

	//##Description
	//## @brief BoundingObject that will be cut
	mitk::BoundingObject::Pointer m_BoundingObject;
	//##Description
	//## @brief Value for inside pixels, used when m_UseInsideValue is @a true
	//##
	//## \sa m_UseInsideValue
	mitk::ScalarType m_InsideValue;
	//##Description
	//## @brief Value for outside pixels (default: 0)
	//##
	//## Used only if m_AutoOutsideValue is \a false.
	mitk::ScalarType m_OutsideValue;
	//##Description
	//## @brief If \a true the minimum of the ouput pixel type is
	//## used as outside value (default: \a false)
	bool m_AutoOutsideValue;
	//##Description
	//## @brief Use m_InsideValue for inside pixels (default: \a false)
	//##
	//## If @a true, pixels that are inside m_BoundingObject
	//## will get m_InsideValue in the cutting process
	//## If @a false, they keep their original value.
	//## \sa m_InsideValue
	bool m_UseInsideValue;

	unsigned int m_OutsidePixelCount;
	unsigned int m_InsidePixelCount;

	unsigned long int valueSum;

	//##Description
	//## @brief Region of input needed for cutting
	mitk::SlicedData::RegionType m_InputRequestedRegion;

	//##Description
	//## @brief Time when Header was last initialized
	itk::TimeStamp m_TimeOfHeaderInitialization;

	mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
	mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

	bool m_UseWholeInputRegion;
};


#endif
