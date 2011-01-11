#include "QexDeviceHelper.h"

class QexDeviceHelperPrivate
{
public:
	QIODevice * device;
	int handshake;
	QByteArray buffer;

	QexDeviceHelperPrivate ()
		: device( 0 )
		, handshake( 0 )
	{
	}
};

QexDeviceHelper::QexDeviceHelper ( QObject * parent )
	: QexRemintReader( parent )
	, p_data( new QexDeviceHelperPrivate )
{
	connect( this, SIGNAL( dataFormed( const QByteArray & ) ),
		this, SIGNAL( inputted( const QByteArray & ) ) );
};

QexDeviceHelper::~QexDeviceHelper ()
{
	delete p_data;
};

void QexDeviceHelper::setDevice ( QIODevice * device )
{
	if ( p_data->device )
	{
		disconnect( p_data->device, SIGNAL( destroyed( QObject * ) )
			, this, SLOT( deviceDestroyed ( QObject * ) ) );
		disconnect( p_data->device, SIGNAL( readyRead() ), this, SLOT( reading() ) );
	}

	p_data->device = device;
	if ( p_data->device )
	{
		connect( p_data->device, SIGNAL( destroyed( QObject * ) )
			, this, SLOT( deviceDestroyed ( QObject * ) ) );
		connect( p_data->device, SIGNAL( readyRead() ), this, SLOT( reading() ) );
	}
	reading();
};

void QexDeviceHelper::setHandshake( int handshake )
{
	p_data->handshake = handshake;
};

void QexDeviceHelper::output ( const QByteArray & data )
{
	if ( !p_data->device || !p_data->device->isWritable() )
		return;

	QDataStream stream( p_data->device );
	stream << data;
};

void QexDeviceHelper::deviceDestroyed ( QObject * obj )
{
	if ( p_data->device == obj )
		p_data->device = 0;
};

void QexDeviceHelper::reading ()
{
	if ( !p_data->device || !p_data->device->isReadable() )
		return;

	if ( p_data->handshake )
	{
		p_data->buffer += p_data->device->readAll();
		if ( p_data->buffer.count() < 4 )
			return;

		QDataStream stream( p_data->buffer );
		int handshake = 0;
		stream >> handshake;
		if ( p_data->handshake != handshake )
		{
			p_data->device->close();
			return;
		}

		p_data->handshake = 0;
		addData( p_data->buffer.mid( 4 ) );
		p_data->buffer = QByteArray();
	}

	addData( p_data->device->readAll() );
};
