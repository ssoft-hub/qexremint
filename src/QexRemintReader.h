/*******************************************************************************
** This file is part of the QexRemint module of the Qt Extension Toolkit.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*******************************************************************************/
//! Ридер для пересылаемых данных
/*!
	\file   QexRemintReader.h
	\author Andrey Stepanov

*******************************************************************************/
#ifndef QEX_META_STREAM_READER_H
#define QEX_META_STREAM_READER_H

#include "QexRemintConfig.h"

class QexRemintReaderPrivate;

class QEX_REMINT_EXPORT QexRemintReader : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( QexRemintReader )

private:
	QexRemintReaderPrivate * p_data;

public:
	QexRemintReader ( QObject * parent = 0 );
	virtual ~QexRemintReader ();

public slots:
	void reset ();
	void addData ( const QByteArray & data );

signals:
	void dataFormed ( const QByteArray & );
};

#endif // QEX_META_STREAM_READER_H
