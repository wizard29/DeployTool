//==============================================================================
// Copyright: Reksoft Ltd.
//==============================================================================
//
// Author:              $Author:$
// Creation date:       2015.04.03
// Modification date:   $Date$
// Module:              DeployTool
// Platform:            MS Windows 7/8, MAC OS X 10.10.x
//
//------------------------------------------------------------------------------
// Note:
//------------------------------------------------------------------------------
// Warning:
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#ifndef DTGLOBAL_H
#define DTGLOBAL_H

#include <qnamespace.h>

namespace DT
{
    enum DataRole
    {
        ItemTypeRole = Qt::UserRole,
        AttributeTypeRole
    };// enum DataRole

    enum OutputItemType
    {
        OutputFolderType = 0,
        OutputBinaryType,
        OutputOtherFileType,
        OutputAttributeType,
        OutputDependencyType,
        OutputDependencyFolderType
    };//enum OutputItemType

    enum AttributeType
    {
        PathAttribute = 0,
        RelocateAttribute,
        RelocatePathAttribute
    };
}//namespace DT
#endif // DTGLOBAL_H

