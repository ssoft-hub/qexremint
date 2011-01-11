/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Транслятор сигналов и слотов
/*!
	\file   QexSignalSlotTranslator.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_SIGNAL_TRANSLATOR_H
#define QEX_SIGNAL_TRANSLATOR_H

#include "QexRemintConfig.h"

class QexSignalSlotTranslatorPrivate;

class QEX_REMINT_EXPORT QexSignalSlotTranslator : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexSignalSlotTranslator )

private:
	QexSignalSlotTranslatorPrivate * p_data;

public:
	QexSignalSlotTranslator ( QObject * parent = 0 );
	virtual ~QexSignalSlotTranslator ();

public slots:
	int connectAllSignals ( const QByteArray & key, QObject * sender );
	bool connectSignal ( const QByteArray & key, QObject * sender, const char * a_signal );
	void disconnectSignal ( const QByteArray & key, QObject * sender, const char * a_signal );
	void disconnectSignals ( const QByteArray & key, QObject * sender );
	void disconnectSignals ( QObject * sender, const char * a_signal );
	void disconnectSignals ( QObject * sender );
	void disconnectAllSignals ();

	bool connectSlot ( const QByteArray & key, const char * a_signal, QObject * reciver, const char * a_slot );
	void disconnectSlot ( const QByteArray & key, const char * a_signal, QObject * reciver, const char * a_slot );
	void disconnectSlots ( const char * a_signal, QObject * reciver, const char * a_slot );
	void disconnectSlots ( QObject * reciver, const char * a_slot );
	void disconnectSlots ( QObject * reciver );
	void disconnectAllSlots ();

	void activateSlot ( const QByteArray & data );

signals:
	void signalPacked ( const QByteArray & );
};

#endif // QEX_SIGNAL_TRANSLATOR_H
