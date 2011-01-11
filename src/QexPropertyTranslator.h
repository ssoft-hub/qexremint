/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Транслятор атрибутов (свойств)
/*!
	\file   QexSignalPacker.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_PROPERTY_TRANSLATOR_H
#define QEX_PROPERTY_TRANSLATOR_H

#include "QexRemintConfig.h"

#define PROPERTY(a)        "3"#a

class QexPropertyTranslatorPrivate;

class QEX_REMINT_EXPORT QexPropertyTranslator: public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexPropertyTranslator )

private:
	QexPropertyTranslatorPrivate * p_data;

public:
	QexPropertyTranslator ( QObject * parent = 0 );
	virtual ~QexPropertyTranslator ();

	bool getProperty ( const QByteArray & key, const char * a_property
		, QObject * requester, const char * a_slot );
	bool setProperty ( const QByteArray & key, const char * a_property, QVariant value );
	bool setProperty ( const QByteArray & key, const char * a_property, QGenericArgument arg );

public slots:
	int connectAllProperties ( const QByteArray & key, QObject * respondent );
	bool connectProperty ( const QByteArray & key, QObject * respondent, const char * a_property );
	void disconnectProperty ( const QByteArray & key, QObject * respondent, const char * a_property );
	void disconnectProperties ( QObject * respondent, const char * a_property );
	void disconnectProperties ( QObject * respondent );
	void disconnectAllProperties ();

	void disconnectRequester ( QObject * requester, const char * a_slot );
	void disconnectRequester ( QObject * requester );
	void disconnectAllRequesters ();

	void activateGetProperty ( const QByteArray & data );
	void activateSetProperty ( const QByteArray & data );
	void activateAnswer ( const QByteArray & data );

signals:
	void getPropertyPacked ( const QByteArray & );
	void setPropertyPacked ( const QByteArray & );
	void answerPacked ( const QByteArray & );
};

#endif // QEX_PROPERTY_TRANSLATOR_H
