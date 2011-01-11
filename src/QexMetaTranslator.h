/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Транслятор метаданных (методы, атрибуты, сигналы и слоты)
/*!
	\file   QexMetaTranslator.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_META_TRANSLATOR_H
#define QEX_META_TRANSLATOR_H

#include "QexRemintConfig.h"
#include "QexMethodTranslator.h"
#include "QexPropertyTranslator.h"
#include "QexSignalSlotTranslator.h"

class QexMetaTranslatorPrivate;

class QEX_REMINT_EXPORT QexMetaTranslator : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexMetaTranslator )

private:
	QexMetaTranslatorPrivate * p_data;

public:
	QexMetaTranslator ( QObject * parent = 0 );
	virtual ~QexMetaTranslator ();

public:
	bool callMethod ( const QByteArray & key, const char * a_method
		, QObject * requester, const char * a_slot, QVariant arg1 = QVariant()
		, QVariant arg2 = QVariant(), QVariant arg3 = QVariant(), QVariant arg4 = QVariant()
		, QVariant arg5 = QVariant(), QVariant arg6 = QVariant(), QVariant arg7 = QVariant()
		, QVariant arg8 = QVariant(), QVariant arg9 = QVariant(), QVariant arg10 = QVariant() );

	bool callMethod ( const QByteArray & key, const char * a_method
		, QObject * requester, const char * a_slot, QGenericArgument arg1
		, QGenericArgument arg2 = QGenericArgument(), QGenericArgument arg3 = QGenericArgument(), QGenericArgument arg4 = QGenericArgument()
		, QGenericArgument arg5 = QGenericArgument(), QGenericArgument arg6 = QGenericArgument(), QGenericArgument arg7 = QGenericArgument()
		, QGenericArgument arg8 = QGenericArgument(), QGenericArgument arg9 = QGenericArgument(), QGenericArgument arg10 = QGenericArgument() );

	bool getProperty ( const QByteArray & key, const char * a_property
		, QObject * requester, const char * a_slot );

	bool setProperty ( const QByteArray & key, const char * a_property, QVariant value );
	bool setProperty ( const QByteArray & key, const char * a_property, QGenericArgument arg );

	QexMethodTranslator & methodTranslator ();
	QexPropertyTranslator & propertyTranslator ();
	QexSignalSlotTranslator & signalSlotTranslator ();

public slots:
	void inputData ( const QByteArray & data );

private slots:
	void outputSignal ( const QByteArray & data );
	void outputMethod ( const QByteArray & data );
	void outputMethodAnswer ( const QByteArray & data );
	void outputGetProperty ( const QByteArray & data );
	void outputSetProperty ( const QByteArray & data );
	void outputGetPropertyAnswer ( const QByteArray & data );

signals:
	void actionError ();
	void dataOutputted ( const QByteArray & );
};

#endif // QEX_META_TRANSLATOR_H
