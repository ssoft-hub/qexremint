#ifndef QEX_DEVICE_HELPER_H
#define QEX_DEVICE_HELPER_H

#include "QexRemintReader.h"

class QexDeviceHelperPrivate;

class QEX_REMINT_EXPORT QexDeviceHelper : public QexRemintReader
{
	Q_OBJECT
	Q_DISABLE_COPY( QexDeviceHelper )

private:
	QexDeviceHelperPrivate * p_data;

public:
	QexDeviceHelper ( QObject * parent = 0 );
	virtual ~QexDeviceHelper ();

	void setDevice ( QIODevice * device );
	void setHandshake( int handshake );

public slots:
	void output ( const QByteArray & data );

private slots:
	void reading ();
	void deviceDestroyed ( QObject * obj );

signals:
	void inputted ( const QByteArray & );
};

#endif // QEX_DEVICE_HELPER_H
