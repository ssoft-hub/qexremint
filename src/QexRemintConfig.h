/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Файл конфигурации
/*!
	\file   QexRemintConfig.h
	\author Andrey Stepanov

*******************************************************************************/

#ifndef QEX_REMINT_CONFIG_H
#define QEX_REMINT_CONFIG_H

#include "stable.h"

#	if defined WIN32 || defined WINCE
#		ifdef QEX_REMINT_LIB
#			define QEX_REMINT_EXPORT __declspec( dllexport )
#		else
#			define QEX_REMINT_EXPORT __declspec( dllimport )
#		endif //QEX_REMINT_LIB
#	else
#		define QEX_REMINT_EXPORT
#	endif // WIN32

#endif // QEX_REMINT_CONFIG_H
