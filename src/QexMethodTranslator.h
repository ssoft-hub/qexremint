/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Транслятор методов
/*!
	\file   QexMethodTranslator.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_METHOD_TRANSLATOR_H
#define QEX_METHOD_TRANSLATOR_H

#include "QexRemintConfig.h"

class QexMethodTranslatorPrivate;

class QEX_REMINT_EXPORT QexMethodTranslator: public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexMethodTranslator )

private:
	QexMethodTranslatorPrivate * p_data;

public:
	QexMethodTranslator ( QObject * parent = 0 );
	virtual ~QexMethodTranslator ();

	bool callMethod ( const QByteArray & key, const char * a_method
		, QObject * requester, const char * a_slot, QVariant arg1
		, QVariant arg2 = QVariant(), QVariant arg3 = QVariant(), QVariant arg4 = QVariant()
		, QVariant arg5 = QVariant(), QVariant arg6 = QVariant(), QVariant arg7 = QVariant()
		, QVariant arg8 = QVariant(), QVariant arg9 = QVariant(), QVariant arg10 = QVariant() );

	bool callMethod ( const QByteArray & key, const char * a_method
		, QObject * requester, const char * a_slot, QGenericArgument arg1 = QGenericArgument()
		, QGenericArgument arg2 = QGenericArgument(), QGenericArgument arg3 = QGenericArgument(), QGenericArgument arg4 = QGenericArgument()
		, QGenericArgument arg5 = QGenericArgument(), QGenericArgument arg6 = QGenericArgument(), QGenericArgument arg7 = QGenericArgument()
		, QGenericArgument arg8 = QGenericArgument(), QGenericArgument arg9 = QGenericArgument(), QGenericArgument arg10 = QGenericArgument() );

	bool callMethod ( const QByteArray & key, const char * a_method, QVariant arg1
		, QVariant arg2 = QVariant(), QVariant arg3 = QVariant(), QVariant arg4 = QVariant()
		, QVariant arg5 = QVariant(), QVariant arg6 = QVariant(), QVariant arg7 = QVariant()
		, QVariant arg8 = QVariant(), QVariant arg9 = QVariant(), QVariant arg10 = QVariant() );

	bool callMethod ( const QByteArray & key, const char * a_method, QGenericArgument arg1 = QGenericArgument()
		, QGenericArgument arg2 = QGenericArgument(), QGenericArgument arg3 = QGenericArgument(), QGenericArgument arg4 = QGenericArgument()
		, QGenericArgument arg5 = QGenericArgument(), QGenericArgument arg6 = QGenericArgument(), QGenericArgument arg7 = QGenericArgument()
		, QGenericArgument arg8 = QGenericArgument(), QGenericArgument arg9 = QGenericArgument(), QGenericArgument arg10 = QGenericArgument() );

public slots:
	int connectAllMethods ( const QByteArray & key, QObject * respondent );
	bool connectMethod ( const QByteArray & key, QObject * respondent, const char * a_method );
	void disconnectMethod ( const QByteArray & key, QObject * respondent, const char * a_method );
	void disconnectMethods ( QObject * respondent, const char * a_method );
	void disconnectMethods ( QObject * respondent );
	void disconnectAllMethods ();

	void disconnectRequester ( QObject * requester, const char * a_slot );
	void disconnectRequester ( QObject * requester );
	void disconnectAllRequesters ();

	void activateMethod ( const QByteArray & data );
	void activateAnswer ( const QByteArray & data );

signals:
	void methodPacked ( const QByteArray & );
	void answerPacked ( const QByteArray & );
};

#endif // QEX_METHOD_TRANSLATOR_H
