/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Активатор слотов
/*!
	\file   QexSlotActivator.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_SLOT_ACTIVATOR_H
#define QEX_SLOT_ACTIVATOR_H

#include "QexRemintConfig.h"

class QexSlotActivatorPrivate;

class QexSlotActivator : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexSlotActivator )

private:
	QexSlotActivatorPrivate * p_data;

public:
	QexSlotActivator ( const QByteArray & key, const char * a_signal
		, QObject * reciver, const char * a_slot, QObject * parent = 0 );
	virtual ~QexSlotActivator ();

	bool isValid () const;
	QObject * activatingReciver () const;
	QByteArray senderKey () const;
	QByteArray signalSignature () const;
	QByteArray slotSignature () const;

private slots:
	void clean ();

public slots:
	void activate ( const QByteArray & data );
};

#endif // QEX_SLOT_ACTIVATOR_H
