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
#ifndef __QVTKOpenGLNativeWidget2_h
#define __QVTKOpenGLNativeWidget2_h

#include <QVTKOpenGLNativeWidget.h>

class QVTKOpenGLNativeWidget2 : public QVTKOpenGLNativeWidget
{
  Q_OBJECT

public:
  QVTKOpenGLNativeWidget2(QWidget *parent, Qt::WindowFlags f = Qt::WindowFlags{});

protected:
  bool event(QEvent* evt) override;
};

#endif
