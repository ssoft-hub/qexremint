/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Паковщик сигналов
/*!
	\file   QexSignalPacker.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_SIGNAL_PACKER_H
#define QEX_SIGNAL_PACKER_H

#include "QexRemintConfig.h"

class QexSignalPackerPrivate;

class QexSignalPacker : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexSignalPacker )

private:
	QexSignalPackerPrivate * p_data;

public:
	QexSignalPacker ( const QByteArray & key, QObject * sender, const char * a_signal, QObject * parent = 0 );
	virtual ~QexSignalPacker ();

	bool isValid () const;
	QObject * packedSender () const;
	QByteArray senderKey () const;
	QByteArray signalSignature () const;

private:
	void packSignal( void ** a );

private slots:
	void clean ();
	void inputSignal ();
	void inputSignal ( const void * & a1 );
	void inputSignal ( const void * & a1, const void * & a2 );
	void inputSignal ( const void * & a1, const void * & a2, const void * & a3 );
	void inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4 );
	void inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5 );
	void inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
		, const void * & a6 );
	void inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
		, const void * & a6, const void * & a7 );
	void inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
		, const void * & a6, const void * & a7, const void * & a8 );
	void inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
		, const void * & a6, const void * & a7, const void * & a8, const void * & a9 );
	void inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
		, const void * & a6, const void * & a7, const void * & a8, const void * & a9, const void * & a10 );

signals:
	void signalPacked ( const QByteArray & );
};

#endif // QEX_SIGNAL_PACKER_H
