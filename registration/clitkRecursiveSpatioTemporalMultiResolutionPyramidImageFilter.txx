/*=========================================================================
  Program:   vv                     http://www.creatis.insa-lyon.fr/rio/vv

  Authors belong to: 
  - University of LYON              http://www.universite-lyon.fr/
  - Léon Bérard cancer center       http://www.centreleonberard.fr
  - CREATIS CNRS laboratory         http://www.creatis.insa-lyon.fr

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the copyright notices for more information.

  It is distributed under dual licence

  - BSD        See included LICENSE.txt file
  - CeCILL-B   http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
===========================================================================**/
#ifndef __clitkRecursiveSpatioTemporalMultiResolutionPyramidImageFilter_txx
#define __clitkRecursiveSpatioTemporalMultiResolutionPyramidImageFilter_txx
#include "clitkRecursiveSpatioTemporalMultiResolutionPyramidImageFilter.h"
#include "itkShrinkImageFilter.h"
#include "itkGaussianOperator.h"
#include "itkCastImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkExceptionObject.h"
#include "itkResampleImageFilter.h"
#include "itkShrinkImageFilter.h"
#include "itkIdentityTransform.h"

namespace clitk
{

/**
 * Constructor
 */
template <class TInputImage, class TOutputImage>
RecursiveSpatioTemporalMultiResolutionPyramidImageFilter<TInputImage, TOutputImage>
::RecursiveSpatioTemporalMultiResolutionPyramidImageFilter()
{
  this->Superclass::m_UseShrinkImageFilter = true;
}

/**
 * GenerateData
 */
template <class TInputImage, class TOutputImage>
void
RecursiveSpatioTemporalMultiResolutionPyramidImageFilter<TInputImage, TOutputImage>
::GenerateData()
{

  if( !this->IsScheduleDownwardDivisible( this->GetSchedule() ) )
    {
    // use the Superclass implemenation
    this->Superclass::GenerateData();
    return;
    }

  // Get the input and output pointers
  InputImageConstPointer  inputPtr = this->GetInput();

  // Create caster, smoother and resampleShrink filters
  typedef itk::CastImageFilter<TInputImage, TOutputImage>              CasterType;
  typedef itk::CastImageFilter<TOutputImage, TOutputImage>             CopierType;
  typedef itk::DiscreteGaussianImageFilter<TOutputImage, TOutputImage> SmootherType;

  typedef itk::ImageToImageFilter<TOutputImage,TOutputImage>           ImageToImageType;
  typedef itk::ResampleImageFilter<TOutputImage,TOutputImage>          ResampleShrinkerType;
  typedef itk::ShrinkImageFilter<TOutputImage,TOutputImage>            ShrinkerType;

  typename CasterType::Pointer   caster   = CasterType::New();
  typename CopierType::Pointer   copier   = CopierType::New();
  typename SmootherType::Pointer smoother = SmootherType::New();


  typename ImageToImageType::Pointer shrinkerFilter;
  //
  // only one of these pointers is going to be valid, depending on the
  // value of UseShrinkImageFilter flag
  typename ResampleShrinkerType::Pointer resampleShrinker;
  typename ShrinkerType::Pointer shrinker;

  if(this->GetUseShrinkImageFilter())
    {
    shrinker = ShrinkerType::New();
    shrinkerFilter = shrinker.GetPointer();
    }
  else
    {
    resampleShrinker = ResampleShrinkerType::New();
    typedef itk::LinearInterpolateImageFunction< OutputImageType, double >
      LinearInterpolatorType;
    typename LinearInterpolatorType::Pointer interpolator = 
      LinearInterpolatorType::New();
    typedef itk::IdentityTransform<double,OutputImageType::ImageDimension>
      IdentityTransformType;
    typename IdentityTransformType::Pointer identityTransform =
      IdentityTransformType::New();
    resampleShrinker->SetInterpolator( interpolator );
    resampleShrinker->SetDefaultPixelValue( 0 );
    resampleShrinker->SetTransform(identityTransform);
    shrinkerFilter = resampleShrinker.GetPointer();
    }

  int ilevel;
  unsigned int idim;
  unsigned int factors[ImageDimension];
  double       variance[ImageDimension];

  bool         allOnes;
  OutputImagePointer   outputPtr;
  OutputImagePointer swapPtr;
  typename TOutputImage::RegionType  LPRegion;

  smoother->SetUseImageSpacing( false );
  smoother->SetMaximumError( this->GetMaximumError() );
  shrinkerFilter->SetInput( smoother->GetOutput() );

  
  // recursively compute outputs starting from the last one
  for( ilevel = this->GetNumberOfLevels() - 1; ilevel > -1; ilevel--)
    {

    this->UpdateProgress( 1.0 - static_cast<float>( 1 + ilevel ) /
                          static_cast<float>( this->GetNumberOfLevels() ) );

    // Allocate memory for each output
    outputPtr = this->GetOutput( ilevel );
    outputPtr->SetBufferedRegion( outputPtr->GetRequestedRegion() );
    outputPtr->Allocate();
    
    // cached a copy of the largest possible region
    LPRegion = outputPtr->GetLargestPossibleRegion();

    // Check shrink factors and compute variances
    allOnes = true;
    for( idim = 0; idim < ImageDimension; idim++ )
      {
      if( ilevel == static_cast<int>(this->GetNumberOfLevels()) - 1)
        {
        factors[idim] = this->GetSchedule()[ilevel][idim];
        }
      else
        {
        factors[idim] = this->GetSchedule()[ilevel][idim] /
          this->GetSchedule()[ilevel+1][idim];
        }
      variance[idim] = std::sqr( 0.5 *
                                     static_cast<float>( factors[idim] ) );
      if( factors[idim] != 1 ) 
        { 
        allOnes = false; 
        }
      else
        {
        variance[idim] = 0.0;
        }
      }


    if( allOnes && ilevel == static_cast<int>(this->GetNumberOfLevels()) - 1 )
      {
      // just copy the input over
      caster->SetInput( inputPtr );
      caster->GraftOutput( outputPtr );
      // ensure only the requested region is updated
      caster->UpdateOutputInformation();
      caster->GetOutput()->SetRequestedRegion(outputPtr->GetRequestedRegion());
      caster->GetOutput()->PropagateRequestedRegion();
      caster->GetOutput()->UpdateOutputData();

      swapPtr = caster->GetOutput();


      }
    else if( allOnes )
      {
      // just copy the data over
      copier->SetInput( swapPtr );
      copier->GraftOutput( outputPtr );
      // ensure only the requested region is updated
      copier->GetOutput()->UpdateOutputInformation();
      copier->GetOutput()->SetRequestedRegion(outputPtr->GetRequestedRegion());
      copier->GetOutput()->PropagateRequestedRegion();
      copier->GetOutput()->UpdateOutputData();

      swapPtr = copier->GetOutput();
        
      }
    else
      {
      if( ilevel == static_cast<int>(this->GetNumberOfLevels()) - 1 )
        {
        // use caster -> smoother -> shrinker piepline
        caster->SetInput( inputPtr );
        smoother->SetInput( caster->GetOutput() );
        }
      else
        {
        // use smoother -> shrinker pipeline
        smoother->SetInput( swapPtr );
        }

      smoother->SetVariance( variance );

      //      shrinker->SetShrinkFactors( factors );
      //      shrinker->GraftOutput( outputPtr );
      if(!this->GetUseShrinkImageFilter())
        {
        resampleShrinker->SetOutputParametersFromImage(outputPtr);
        }
      else
        {
        shrinker->SetShrinkFactors(factors);
        }
      shrinkerFilter->GraftOutput(outputPtr);
      shrinkerFilter->Modified();
      // ensure only the requested region is updated
      shrinkerFilter->GetOutput()->UpdateOutputInformation();
      shrinkerFilter->GetOutput()->SetRequestedRegion(outputPtr->GetRequestedRegion());
      shrinkerFilter->GetOutput()->PropagateRequestedRegion();
      shrinkerFilter->GetOutput()->UpdateOutputData();

      swapPtr = shrinkerFilter->GetOutput();

      }

    // graft pipeline output back onto this filter's output
    swapPtr->SetLargestPossibleRegion( LPRegion );
    this->GraftNthOutput( ilevel, swapPtr );

    // disconnect from pipeline to stop cycle
    swapPtr->DisconnectPipeline();

    }

}

/**
 * PrintSelf method
 */
template <class TInputImage, class TOutputImage>
void
RecursiveSpatioTemporalMultiResolutionPyramidImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}


/* 
 * GenerateOutputRequestedRegion
 */
template <class TInputImage, class TOutputImage>
void
RecursiveSpatioTemporalMultiResolutionPyramidImageFilter<TInputImage, TOutputImage>
::GenerateOutputRequestedRegion(itk::DataObject * ptr )
{

  // call the superclass's implementation of this method
  Superclass::GenerateOutputRequestedRegion( ptr );

  TOutputImage * refOutputPtr = static_cast<TOutputImage*>( ptr );
  if( !refOutputPtr )
    {
    itkExceptionMacro( << "Could not cast ptr to TOutputImage*." );
    }

  // find the index for this output
  unsigned int refLevel;
  refLevel = refOutputPtr->GetSourceOutputIndex();

  typedef typename TOutputImage::PixelType                 OutputPixelType;
  typedef itk::GaussianOperator<OutputPixelType,ImageDimension> OperatorType;

  OperatorType * oper = new OperatorType;
  oper->SetMaximumError( this->GetMaximumError() );

  typedef typename OutputImageType::SizeType    SizeType;
  typedef typename SizeType::SizeValueType      SizeValueType;
  typedef typename OutputImageType::IndexType   IndexType;
  typedef typename IndexType::IndexValueType    IndexValueType;
  typedef typename OutputImageType::RegionType  RegionType;

  int ilevel, idim;
  unsigned int factors[ImageDimension];
  
  typename TInputImage::SizeType radius;

  RegionType requestedRegion;
  SizeType   requestedSize;
  IndexType  requestedIndex;

  // compute requested regions for lower levels
  for( ilevel = refLevel + 1; ilevel < static_cast<int>(this->GetNumberOfLevels()); 
       ilevel++ )
    {

    requestedRegion = this->GetOutput( ilevel - 1 )->GetRequestedRegion();
    requestedSize = requestedRegion.GetSize();
    requestedIndex = requestedRegion.GetIndex();

    for( idim = 0; idim < static_cast<int>(ImageDimension); idim++ )
      {
      factors[idim] = this->GetSchedule()[ilevel-1][idim] / this->GetSchedule()[ilevel][idim];

      // take into account shrink component
      requestedSize[idim] *= static_cast<SizeValueType>(factors[idim]);
      requestedIndex[idim] *= static_cast<IndexValueType>(factors[idim]);
      
      // take into account smoothing component
      if( factors[idim] > 1 )
        {
        oper->SetDirection( idim );
        oper->SetVariance( std::sqr( 0.5 *
                                         static_cast<float>( factors[idim] ) ) );
        oper->CreateDirectional();
        radius[idim] = oper->GetRadius()[idim];
        }
      else
        {
        radius[idim] = 0;
        }
      }

    requestedRegion.SetSize( requestedSize );
    requestedRegion.SetIndex( requestedIndex );
    requestedRegion.PadByRadius( radius );
    requestedRegion.Crop( this->GetOutput(ilevel)->
                          GetLargestPossibleRegion() );

    this->GetOutput(ilevel)->SetRequestedRegion( requestedRegion );

    }


  // compute requested regions for higher levels
  for( ilevel = refLevel - 1; ilevel > -1; ilevel-- )
    {
    requestedRegion = this->GetOutput( ilevel + 1 )->GetRequestedRegion();
    requestedSize = requestedRegion.GetSize();
    requestedIndex = requestedRegion.GetIndex();

    for( idim = 0; idim < static_cast<int>(ImageDimension); idim++ )
      {

      factors[idim] = this->GetSchedule()[ilevel][idim] / this->GetSchedule()[ilevel+1][idim];
      
      // take into account smoothing component
      if( factors[idim] > 1 )
        {
        oper->SetDirection( idim );
        oper->SetVariance( std::sqr( 0.5 *
                                         static_cast<float>( factors[idim] ) ) );
        oper->CreateDirectional();
        radius[idim] = oper->GetRadius()[idim];
        }
      else
        {
        radius[idim] = 0;
        }

      requestedSize[idim] -= static_cast<SizeValueType>( 
        2 * radius[idim] );
      requestedIndex[idim] += radius[idim];
      
      // take into account shrink component
      requestedSize[idim] = static_cast<SizeValueType>( std::floor(
                                                          static_cast<double>(requestedSize[idim]) / 
                                                          static_cast<double>(factors[idim]) ) );
      if( requestedSize[idim] < 1 )
        {
        requestedSize[idim] = 1;
        }
      requestedIndex[idim] = static_cast<IndexValueType>( std::ceil(
                                                            static_cast<double>(requestedIndex[idim]) /
                                                            static_cast<double>(factors[idim]) ) );

      }

    requestedRegion.SetSize( requestedSize );
    requestedRegion.SetIndex( requestedIndex );
    requestedRegion.Crop( this->GetOutput(ilevel)->
                          GetLargestPossibleRegion() );

    this->GetOutput(ilevel)->SetRequestedRegion( requestedRegion );

    }

  // clean up
  delete oper;

}

/** 
 * GenerateInputRequestedRegion
 */
template <class TInputImage, class TOutputImage>
void
RecursiveSpatioTemporalMultiResolutionPyramidImageFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion()
{

  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // get pointers to the input and output
  InputImagePointer  inputPtr = 
    const_cast< InputImageType *>( this->GetInput() );
  if ( !inputPtr )
    {
    itkExceptionMacro( << "Input has not been set." );
    }

  // compute baseIndex and baseSize
  typedef typename OutputImageType::SizeType    SizeType;
  typedef typename SizeType::SizeValueType      SizeValueType;
  typedef typename OutputImageType::IndexType   IndexType;
  typedef typename IndexType::IndexValueType    IndexValueType;
  typedef typename OutputImageType::RegionType  RegionType;

  unsigned int refLevel = this->GetNumberOfLevels() - 1;
  SizeType baseSize = this->GetOutput(refLevel)->GetRequestedRegion().GetSize();
  IndexType baseIndex = this->GetOutput(refLevel)->GetRequestedRegion().GetIndex();
  RegionType baseRegion;

  unsigned int idim;
  for( idim = 0; idim < ImageDimension; idim++ )
    {
    unsigned int factor = this->GetSchedule()[refLevel][idim];
    baseIndex[idim] *= static_cast<IndexValueType>( factor );
    baseSize[idim] *= static_cast<SizeValueType>( factor );
    }
  baseRegion.SetIndex( baseIndex );
  baseRegion.SetSize( baseSize );

  // compute requirements for the smoothing part
  typedef typename TOutputImage::PixelType                 OutputPixelType;
  typedef itk::GaussianOperator<OutputPixelType,ImageDimension> OperatorType;

  OperatorType *oper = new OperatorType;

  typename TInputImage::SizeType radius;

  RegionType inputRequestedRegion = baseRegion;
  refLevel = 0;

  for( idim = 0; idim < TInputImage::ImageDimension; idim++ )
    {
    oper->SetDirection(idim);
    oper->SetVariance( std::sqr( 0.5 * static_cast<float>(
                                       this->GetSchedule()[refLevel][idim] ) ) );
    oper->SetMaximumError( this->GetMaximumError() );
    oper->CreateDirectional();
    radius[idim] = oper->GetRadius()[idim];
    if( this->GetSchedule()[refLevel][idim] <= 1 )
      {
      radius[idim] = 0;
      }
      
    }
  delete oper;

  inputRequestedRegion.PadByRadius( radius );

  // make sure the requested region is within the largest possible
  inputRequestedRegion.Crop( inputPtr->GetLargestPossibleRegion() );
  
  // set the input requested region
  inputPtr->SetRequestedRegion( inputRequestedRegion );

}


} // namespace clitk

#endif
