/*=========================================================================
  Program:         vv http://www.creatis.insa-lyon.fr/rio/vv
  Main authors :   XX XX XX

  Authors belongs to:
  - University of LYON           http://www.universite-lyon.fr/
  - Léon Bérard cancer center    http://www.centreleonberard.fr
  - CREATIS CNRS laboratory      http://www.creatis.insa-lyon.fr

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the copyright notices for more information.

  It is distributed under dual licence
  - BSD       http://www.opensource.org/licenses/bsd-license.php
  - CeCILL-B  http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html

  =========================================================================*/

#include "clitkDicomRT_StructureSet.h"
#include <vtksys/SystemTools.hxx>
#include "gdcmFile.h"
#if GDCM_MAJOR_VERSION == 2
#include "gdcmReader.h"
#include "gdcmAttribute.h"
#endif

//--------------------------------------------------------------------
clitk::DicomRT_StructureSet::DicomRT_StructureSet()
{
  mStudyID = "NoStudyID";
  mStudyTime = "NoStudyTime";
  mStudyDate = "NoStudyDate";
  mLabel = "NoLabel";
  mName = "NoName";
  mDate = "NoDate";
  mTime = "NoTime";
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
clitk::DicomRT_StructureSet::~DicomRT_StructureSet()
{
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
const std::string & clitk::DicomRT_StructureSet::GetStudyID() const
{
  return mStudyID;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
const std::string & clitk::DicomRT_StructureSet::GetStudyTime() const
{
  return mStudyTime;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
const std::string & clitk::DicomRT_StructureSet::GetStudyDate() const
{
  return mStudyDate;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
const std::string & clitk::DicomRT_StructureSet::GetLabel() const
{
  return mLabel;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
const std::string & clitk::DicomRT_StructureSet::GetName() const
{
  return mName;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
const std::string & clitk::DicomRT_StructureSet::GetDate() const
{
  return mDate;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
const std::string & clitk::DicomRT_StructureSet::GetTime() const
{
  return mTime;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
const std::vector<clitk::DicomRT_ROI::Pointer> & clitk::DicomRT_StructureSet::GetListOfROI() const
{
  return mListOfROI;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
clitk::DicomRT_ROI* clitk::DicomRT_StructureSet::GetROI(int n)
{
  if (mMapOfROIIndex.find(n) == mMapOfROIIndex.end()) {
    std::cerr << "No ROI number " << n << std::endl;
    return NULL;
  }
  //  DD(mListOfROI[mMapOfROIIndex[n]]->GetName());
  //DD(mListOfROI[mMapOfROIIndex[n]]->GetROINumber());
  return mListOfROI[mMapOfROIIndex[n]];
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
void clitk::DicomRT_StructureSet::Print(std::ostream & os) const
{
  os << "Study ID      = " << mStudyID << std::endl
     << "Study Date    = " << mStudyDate << std::endl
     << "Study Time    = " << mStudyTime << std::endl
     << "Struct Label  = " << mLabel << std::endl
     << "Struct Name   = " << mName << std::endl
     << "Struct Time   = " << mTime << std::endl
     << "Number of ROI = " << mListOfROI.size() << std::endl;
  for(unsigned int i=0; i<mListOfROI.size(); i++) {
    mListOfROI[i]->Print(os);
  }
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
void clitk::DicomRT_StructureSet::Read(const std::string & filename)
{
  // Open DICOM
#if GDCM_MAJOR_VERSION == 2
  gdcm::Reader reader;
  reader.SetFileName(filename.c_str());
  reader.Read();

  const gdcm::File & file = reader.GetFile();
  const gdcm::DataSet & ds = file.GetDataSet();

  // Check file type
  //Verify if the file is a RT-Structure-Set dicom file
  gdcm::MediaStorage ms;
  ms.SetFromFile(file);
  if( ms != gdcm::MediaStorage::RTStructureSetStorage )
    {
    std::cerr << "Error. the file " << filename
              << " is not a Dicom Struct ? (must have a SOP Class UID [0008|0016] = 1.2.840.10008.5.1.4.1.1.481.3 ==> [RT Structure Set Storage])"
              << std::endl;
    exit(0);
    }

  gdcm::Attribute<0x8,0x60> modality;
  modality.SetFromDataSet( ds );
  if( modality.GetValue() != "RTSTRUCT" )
    {
    std::cerr << "Error. the file " << filename
              << " is not a Dicom Struct ? (must have 0x0008,0x0060 = RTSTRUCT [RT Structure Set Storage])"
              << std::endl;
    exit(0);
    }

  // Read global info
  gdcm::Attribute<0x20,0x10> studyid;
  studyid.SetFromDataSet( ds );
  gdcm::Attribute<0x8,0x20> studytime;
  studytime.SetFromDataSet( ds );
  gdcm::Attribute<0x8,0x30> studydate;
  studydate.SetFromDataSet( ds );
  gdcm::Attribute<0x3006,0x02> label;
  label.SetFromDataSet( ds );
  gdcm::Attribute<0x3006,0x04> atname;
  atname.SetFromDataSet( ds );
  gdcm::Attribute<0x3006,0x09> time;
  time.SetFromDataSet( ds );

  mStudyID   = studyid.GetValue();
  mStudyTime = studytime.GetValue();
  mStudyDate = studydate.GetValue();
  mLabel     = label.GetValue();
  mName      = atname.GetValue();
  mTime      = time.GetValue();

  //----------------------------------
  // Read all ROI Names and number
  // 0x3006,0x0020 = [ Structure Set ROI Sequence ]
  gdcm::Tag tssroisq(0x3006,0x0020);
  const gdcm::DataElement &ssroisq = ds.GetDataElement( tssroisq );
  gdcm::SmartPointer<gdcm::SequenceOfItems> roi_seq = ssroisq.GetValueAsSQ();
  assert(roi_seq); // TODO error message
  for(unsigned int ridx = 0; ridx < roi_seq->GetNumberOfItems(); ++ridx)
    {
    gdcm::Item & item = roi_seq->GetItem( ridx + 1); // Item starts at 1
    const gdcm::DataSet& nestedds = item.GetNestedDataSet();

    gdcm::Attribute<0x3006,0x26> roiname;
    roiname.SetFromDataSet( nestedds );
    std::string name = roiname.GetValue();      // 0x3006,0x0026 = [ROI Name]
    gdcm::Attribute<0x3006,0x0022> roinumber;
    roinumber.SetFromDataSet( nestedds );
    int nb = roinumber.GetValue();  // 0x3006,0x0022 = [ROI Number]
    // Change number if needed

    //TODO

    // Check if such a number already exist
    if (mMapOfROIName.find(nb) != mMapOfROIName.end()) {
      std::cerr << "WARNING. A Roi already exist with the number "
        << nb << ". I replace." << std::endl;
    }
    // Add in map
    mMapOfROIName[nb] = name;
    }
  // DD(mMapOfROIName.size());

  //----------------------------------
  // Read all ROI
  // 0x3006,0x0039 = [ ROI Contour Sequence ]
  gdcm::Tag troicsq(0x3006,0x0039);
  const gdcm::DataElement &roicsq = ds.GetDataElement( troicsq );
  gdcm::SmartPointer<gdcm::SequenceOfItems> roi_contour_seq = roicsq.GetValueAsSQ();
  assert(roi_contour_seq); // TODO error message
  int n=0;
  for(unsigned int ridx = 0; ridx < roi_contour_seq->GetNumberOfItems(); ++ridx)
    {
    gdcm::Item & item = roi_contour_seq->GetItem( ridx + 1); // Item starts at 1

    DicomRT_ROI::Pointer roi = DicomRT_ROI::New();
    roi->Read(mMapOfROIName, item);
    mListOfROI.push_back(roi);
    mMapOfROIIndex[roi->GetROINumber()] = n;
    n++;
    }

#else
  gdcm::File reader;
  reader.SetFileName(filename.c_str());
  reader.SetMaxSizeLoadEntry(16384); // Needed ...
  reader.SetLoadMode(gdcm::LD_NOSHADOW); // don't load shadow tags (in order to save memory)
  reader.Load();

  // Check file type
  //Verify if the file is a RT-Structure-Set dicom file
  if (!gdcm::Util::DicomStringEqual(reader.GetEntryValue(0x0008,0x0016),"1.2.840.10008.5.1.4.1.1.481.3")) {  //SOP clas UID
    std::cerr << "Error. the file " << filename
              << " is not a Dicom Struct ? (must have a SOP Class UID [0008|0016] = 1.2.840.10008.5.1.4.1.1.481.3 ==> [RT Structure Set Storage])"
              << std::endl;
    exit(0);
  }
  if (!gdcm::Util::DicomStringEqual(reader.GetEntryValue(0x0008,0x0060),"RTSTRUCT")) {  //SOP clas UID
    std::cerr << "Error. the file " << filename
              << " is not a Dicom Struct ? (must have 0x0008,0x0060 = RTSTRUCT [RT Structure Set Storage])"
              << std::endl;
    exit(0);
  }

  // Read global info
  mStudyID   = reader.GetValEntry(0x0020,0x0010)->GetValue();
  mStudyTime = reader.GetValEntry(0x008,0x0020)->GetValue();
  mStudyDate = reader.GetValEntry(0x008,0x0030)->GetValue();
  mLabel     = reader.GetValEntry(0x3006,0x002)->GetValue();
  if (!reader.GetValEntry(0x3006,0x004)) {
    mName = "Anonymous";
  }
  else {
    mName = reader.GetValEntry(0x3006,0x004)->GetValue();
  }
  mTime      = reader.GetValEntry(0x3006,0x009)->GetValue();

  //----------------------------------
  // Read all ROI Names and number
  // 0x3006,0x0020 = [ Structure Set ROI Sequence ]
  gdcm::SeqEntry * roi_seq=reader.GetSeqEntry(0x3006,0x0020);
  assert(roi_seq); // TODO error message
  for (gdcm::SQItem* r=roi_seq->GetFirstSQItem(); r!=0; r=roi_seq->GetNextSQItem()) {
    std::string name = r->GetEntryValue(0x3006,0x0026);      // 0x3006,0x0026 = [ROI Name]
    int nb = atoi(r->GetEntryValue(0x3006,0x0022).c_str());  // 0x3006,0x0022 = [ROI Number]
    // Change number if needed

    //TODO

    // Check if such a number already exist
    if (mMapOfROIName.find(nb) != mMapOfROIName.end()) {
      std::cerr << "WARNING. A Roi already exist with the number "
                << nb << ". I replace." << std::endl;
    }
    // Add in map
    mMapOfROIName[nb] = name;
  }
  // DD(mMapOfROIName.size());

  //----------------------------------
  // Read all ROI
  // 0x3006,0x0039 = [ ROI Contour Sequence ]
  gdcm::SeqEntry * roi_contour_seq=reader.GetSeqEntry(0x3006,0x0039);
  assert(roi_contour_seq); // TODO error message
  int n=0;
  for (gdcm::SQItem* r=roi_contour_seq->GetFirstSQItem(); r!=0; r=roi_contour_seq->GetNextSQItem()) {
    DicomRT_ROI::Pointer roi = DicomRT_ROI::New();
    roi->Read(mMapOfROIName, r);
    mListOfROI.push_back(roi);
    mMapOfROIIndex[roi->GetROINumber()] = n;
    n++;
  }

#endif
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
int clitk::DicomRT_StructureSet::AddBinaryImageAsNewROI(vvImage * im, std::string n)
{
  //DD("AddBinaryImageAsNewROI");
  // Search max ROI number
  int max = -1;
  for(unsigned int i=0; i<mListOfROI.size(); i++) {
    if (mListOfROI[i]->GetROINumber() > max)
      max = mListOfROI[i]->GetROINumber();
  }
  //  DD(max);
  ++max;
  //DD(max);

  // Compute name
  std::ostringstream oss;
  oss << vtksys::SystemTools::GetFilenameName(vtksys::SystemTools::GetFilenameWithoutLastExtension(n));
  //      << "_roi_" << max << vtksys::SystemTools::GetFilenameLastExtension(n);
  //DD(oss.str());
  mMapOfROIName[max] = oss.str();

  // Set color
  std::vector<double> color;
  color.push_back(1);
  color.push_back(0);
  color.push_back(0);

  // Create ROI
  DicomRT_ROI::Pointer roi = DicomRT_ROI::New();
  roi->SetFromBinaryImage(im, max, oss.str(), color, n);
  mListOfROI.push_back(roi);
  mMapOfROIIndex[mListOfROI.size()-1] = max;
  //DD(mMapOfROIIndex[mListOfROI.size()-1]);
  return max;
}
//--------------------------------------------------------------------


