/*=========================================================================
  Program:   vv                     http://www.creatis.insa-lyon.fr/rio/vv

  Authors belong to: 
  - University of LYON              http://www.universite-lyon.fr/
  - Léon Bérard cancer center       http://oncora1.lyon.fnclcc.fr
  - CREATIS CNRS laboratory         http://www.creatis.insa-lyon.fr

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the copyright notices for more information.

  It is distributed under dual licence

  - BSD        See included LICENSE.txt file
  - CeCILL-B   http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
  ======================================================================-====*/

#ifndef CLITKEXTRACTLYMPHSTATIONSFILTER_H
#define CLITKEXTRACTLYMPHSTATIONSFILTER_H

// clitk
#include "clitkFilterBase.h"
#include "clitkFilterWithAnatomicalFeatureDatabaseManagement.h"

// vtk
#include <vtkPolyData.h>

namespace clitk {
  
  //--------------------------------------------------------------------
  /*
    Try to extract the LymphStations part of a thorax CT.
    Need a set of Anatomical Features (AFDB)
  */
  //--------------------------------------------------------------------
  
  template <class TImageType>
  class ITK_EXPORT ExtractLymphStationsFilter: 
    public virtual clitk::FilterBase, 
    public clitk::FilterWithAnatomicalFeatureDatabaseManagement,
    public itk::ImageToImageFilter<TImageType, itk::Image<uchar, 3> >
  {

  public:
    /** Standard class typedefs. */
    typedef itk::ImageToImageFilter<TImageType, itk::Image<uchar, 3> > Superclass;
    typedef ExtractLymphStationsFilter          Self;
    typedef itk::SmartPointer<Self>             Pointer;
    typedef itk::SmartPointer<const Self>       ConstPointer;
    
    /** Method for creation through the object factory. */
    itkNewMacro(Self);
    
    /** Run-time type information (and related methods). */
    itkTypeMacro(ExtractLymphStationsFilter, ImageToImageFilter);

    /** Some convenient typedefs. */
    typedef TImageType                       ImageType;
    typedef typename ImageType::ConstPointer ImageConstPointer;
    typedef typename ImageType::Pointer      ImagePointer;
    typedef typename ImageType::RegionType   ImageRegionType; 
    typedef typename ImageType::PixelType    ImagePixelType; 
    typedef typename ImageType::SizeType     ImageSizeType; 
    typedef typename ImageType::IndexType    ImageIndexType; 
    typedef typename ImageType::PointType    ImagePointType; 
        
    typedef uchar MaskImagePixelType;
    typedef itk::Image<MaskImagePixelType, 3>    MaskImageType;  
    typedef typename MaskImageType::Pointer      MaskImagePointer;
    typedef typename MaskImageType::RegionType   MaskImageRegionType; 
    typedef typename MaskImageType::SizeType     MaskImageSizeType; 
    typedef typename MaskImageType::IndexType    MaskImageIndexType; 
    typedef typename MaskImageType::PointType    MaskImagePointType; 

    typedef itk::Image<MaskImagePixelType, 2>    MaskSliceType;
    typedef typename MaskSliceType::Pointer      MaskSlicePointer;
    typedef typename MaskSliceType::PointType    MaskSlicePointType;
    typedef typename MaskSliceType::RegionType   MaskSliceRegionType; 
    typedef typename MaskSliceType::SizeType     MaskSliceSizeType; 
    typedef typename MaskSliceType::IndexType    MaskSliceIndexType; 

    /** ImageDimension constants */
    itkStaticConstMacro(ImageDimension, unsigned int, ImageType::ImageDimension);
    FILTERBASE_INIT;
   
    itkGetConstMacro(BackgroundValue, MaskImagePixelType);
    itkGetConstMacro(ForegroundValue, MaskImagePixelType);
    itkSetMacro(BackgroundValue, MaskImagePixelType);
    itkSetMacro(ForegroundValue, MaskImagePixelType);

    // Station 8
    // itkSetMacro(DistanceMaxToAnteriorPartOfTheSpine, double);
    //itkGetConstMacro(DistanceMaxToAnteriorPartOfTheSpine, double);
    itkSetMacro(EsophagusDiltationForAnt, MaskImagePointType);
    itkGetConstMacro(EsophagusDiltationForAnt, MaskImagePointType);
    itkSetMacro(EsophagusDiltationForRight, MaskImagePointType);
    itkGetConstMacro(EsophagusDiltationForRight, MaskImagePointType);
    itkSetMacro(InjectedThresholdForS8, double);
    itkGetConstMacro(InjectedThresholdForS8, double);

    // Station 7
    itkGetConstMacro(S7_UseMostInferiorPartOnlyFlag, bool);
    itkSetMacro(S7_UseMostInferiorPartOnlyFlag, bool);
    itkBooleanMacro(S7_UseMostInferiorPartOnlyFlag);

    // All stations
    bool GetComputeStation(std::string s);
    void AddComputeStation(std::string station) ;
    void SetFuzzyThreshold(std::string station, std::string tag, double value);
    double GetFuzzyThreshold(std::string station, std::string tag);

  protected:
    ExtractLymphStationsFilter();
    virtual ~ExtractLymphStationsFilter() {}
    
    virtual void GenerateOutputInformation();
    virtual void GenerateInputRequestedRegion();
    virtual void GenerateData();
    
    ImageConstPointer  m_Input;
    MaskImagePointer   m_Mediastinum;
    MaskImagePointer   m_Working_Support;
    std::map<std::string, MaskImagePointer> m_ListOfStations;
    std::map<std::string, MaskImagePointer> m_ListOfSupports;
    MaskImagePixelType m_BackgroundValue;
    MaskImagePixelType m_ForegroundValue;
    std::map<std::string, bool> m_ComputeStationMap;

    bool CheckForStation(std::string station);
    void Remove_Structures(std::string station, std::string s);

    // Functions common to several stations
    void FindLineForS7S8Separation(MaskImagePointType & A, MaskImagePointType & B);
    double FindCarinaSlicePosition();
    void FindLeftAndRightBronchi();

    // Global parameters
    typedef std::map<std::string, double> FuzzyThresholdByStructureType;
    std::map<std::string, FuzzyThresholdByStructureType> m_FuzzyThreshold;    

    // Station's supports
    void ExtractStationSupports();

    // Station 8
    // double m_DistanceMaxToAnteriorPartOfTheSpine;
    double m_DiaphragmInferiorLimit;
    double m_CarinaZ;
    double m_OriginOfRightMiddleLobeBronchusZ;
    double m_InjectedThresholdForS8;
    MaskImagePointer m_Esophagus;
    MaskImagePointType m_EsophagusDiltationForAnt;
    MaskImagePointType m_EsophagusDiltationForRight;

    void ExtractStation_8();
    void ExtractStation_8_SetDefaultValues();
    void ExtractStation_8_SI_Limits();
    void ExtractStation_8_Ant_Limits();
    void ExtractStation_8_Left_Sup_Limits();
    void ExtractStation_8_Left_Inf_Limits();
    void ExtractStation_8_Single_CCL_Limits();
    void ExtractStation_8_Remove_Structures();

    // Station 3P
    void ExtractStation_3P();
    void ExtractStation_3P_SetDefaultValues();
    void ExtractStation_3P_SI_Limits();
    void ExtractStation_3P_Remove_Structures();
    void ExtractStation_3P_Ant_Limits();
    void ExtractStation_3P_Post_Limits();
    void ExtractStation_3P_LR_sup_Limits();
    void ExtractStation_3P_LR_sup_Limits_2();
    void ExtractStation_3P_LR_inf_Limits();

    // Station 2RL
    void ExtractStation_2RL();
    void ExtractStation_2RL_SetDefaultValues();
    void ExtractStation_2RL_SI_Limits();
    void ExtractStation_2RL_Ant_Limits();
    void ExtractStation_2RL_Ant_Limits2();
    void ExtractStation_2RL_Post_Limits();
    void ExtractStation_2RL_LR_Limits();
    void ExtractStation_2RL_Remove_Structures();
    void ExtractStation_2RL_SeparateRL();
    vtkSmartPointer<vtkPolyData> Build3DMeshFrom2DContour(const std::vector<ImagePointType> & points);

    // Station 3A
    void ExtractStation_3A();
    void ExtractStation_3A_SetDefaultValues();
    void ExtractStation_3A_SI_Limits();
    void ExtractStation_3A_Ant_Limits();
    void ExtractStation_3A_Post_Limits();

    // Station 7
    void ExtractStation_7();
    void ExtractStation_7_SetDefaultValues();
    void ExtractStation_7_SI_Limits();
    void ExtractStation_7_RL_Interior_Limits();


    void ExtractStation_7_RL_Limits_OLD();
    void ExtractStation_7_Posterior_Limits();   
    void ExtractStation_7_Remove_Structures();
    bool m_S7_UseMostInferiorPartOnlyFlag;
    MaskImagePointer m_Working_Trachea;
    MaskImagePointer m_LeftBronchus;
    MaskImagePointer m_RightBronchus;
    typedef std::vector<MaskImageType::PointType> ListOfPointsType;
    ListOfPointsType  m_RightMostInLeftBronchus;
    ListOfPointsType  m_AntMostInLeftBronchus;
    ListOfPointsType  m_PostMostInLeftBronchus;
    ListOfPointsType  m_LeftMostInRightBronchus;
    ListOfPointsType  m_AntMostInRightBronchus;
    ListOfPointsType  m_PostMostInRightBronchus;

    void FindExtremaPointsInBronchus(MaskImagePointer input, 
				     int direction,
				     double distance_max_from_center_point, 
				     ListOfPointsType & LR, 
				     ListOfPointsType & Ant, 
				     ListOfPointsType & Post);
    // Station 4RL
    void ExtractStation_4RL();
    void ExtractStation_4RL_SI_Limits();
    void ExtractStation_4RL_LR_Limits();
    void ExtractStation_4RL_AP_Limits();
    MaskImagePointer m_RightSupport;
    MaskImagePointer m_LeftSupport;

  private:
    ExtractLymphStationsFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented
    
  }; // end class
  //--------------------------------------------------------------------

} // end namespace clitk
//--------------------------------------------------------------------

#ifndef ITK_MANUAL_INSTANTIATION
#include "clitkExtractLymphStationsFilter.txx"
#include "clitkExtractLymphStation_Supports.txx"
#include "clitkExtractLymphStation_8.txx"
#include "clitkExtractLymphStation_3P.txx"
#include "clitkExtractLymphStation_2RL.txx"
#include "clitkExtractLymphStation_3A.txx"
#include "clitkExtractLymphStation_7.txx"
#include "clitkExtractLymphStation_4RL.txx"
#endif

#endif
