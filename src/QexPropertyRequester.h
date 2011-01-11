/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Запросчик значений атрибутов
/*!
	\file   QexPropertyRequester.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_PROPERTY_REQUESTER_H
#define QEX_PROPERTY_REQUESTER_H

#include "QexRemintConfig.h"

class QexPropertyRequesterPrivate;

class QexPropertyRequester : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexPropertyRequester )

private:
	QexPropertyRequesterPrivate * p_data;

public:
	QexPropertyRequester ( QObject * respondent, const QByteArray & data, QObject * parent = 0 );
	virtual ~QexPropertyRequester ();

protected slots:
	virtual void request ();

signals:
	void answerPacked ( const QByteArray & );
};

#endif // QEX_PROPERTY_REQUESTER_H
