/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Активатор методов с возвращвемым значением
/*!
	\file   QexMethodInvoker.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_METHOD_INVOKER_H
#define QEX_METHOD_INVOKER_H

#include "QexRemintConfig.h"

class QexMethodInvokerPrivate;

class QexMethodInvoker : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexMethodInvoker )

private:
	QexMethodInvokerPrivate * p_data;

public:
	QexMethodInvoker ( QObject * respondent, const QByteArray & data, QObject * parent = 0 );
	virtual ~QexMethodInvoker ();

protected slots:
	virtual void invoke ();

signals:
	void answerPacked ( const QByteArray & );
};

#endif // QEX_METHOD_INVOKER_H
