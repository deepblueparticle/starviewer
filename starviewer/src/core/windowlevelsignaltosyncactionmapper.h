/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#ifndef UDGWINDOWLEVELSIGNALTOSYNCACTIONMAPPER_H
#define UDGWINDOWLEVELSIGNALTOSYNCACTIONMAPPER_H

#include "signaltosyncactionmapper.h"

namespace udg {

class VoiLut;

/**
    Implementation of SignalToSyncActionMapper to map window level changes to WindowLevelSyncAction
 */
class WindowLevelSignalToSyncActionMapper : public SignalToSyncActionMapper {
Q_OBJECT
public:
    WindowLevelSignalToSyncActionMapper(QObject *parent = 0);
    ~WindowLevelSignalToSyncActionMapper();

    /// Maps current WW/WL of the viewer
    virtual void mapProperty();

protected:
    void mapSignal();
    void unmapSignal();

protected slots:
    /// Maps given WindowLevel to a WindowLevelSyncAction with the corresponding values
    /// Should be connected to the corresponding signals on Q*Viewer
    void mapToSyncAction(const VoiLut &voiLut);
};

} // End namespace udg

#endif
