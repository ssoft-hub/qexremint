#include "QexRemintReader.h"

////////////////////////////////////////////////////////////////////////////////

class QexRemintReaderPrivate
{
public:
	int index;
	QByteArray data;
	QByteArray buffer;
};

////////////////////////////////////////////////////////////////////////////////

QexRemintReader::QexRemintReader ( QObject * parent )
	: QObject( parent )
	, p_data( new QexRemintReaderPrivate )
{
	reset();
};

QexRemintReader::~QexRemintReader ()
{
	delete p_data;
};

void QexRemintReader::reset ()
{
	p_data->index = 0;
	p_data->data = QByteArray();
};

void QexRemintReader::addData ( const QByteArray & data )
{
	p_data->buffer += data;
	QDataStream stream( p_data->buffer );

	while ( !stream.atEnd() )
	{
		if ( p_data->data.isEmpty() )
		{
			if ( stream.device()->bytesAvailable() < 4 )
			{
				p_data->buffer = stream.device()->readAll();
				return;
			}

			int count;
			stream >> count;
			p_data->data.resize( count );
		}

		p_data->index += stream.readRawData( &( p_data->data.data()[ p_data->index ] )
			, p_data->data.count() - p_data->index );

		if ( p_data->data.count() == p_data->index )
		{
			emit dataFormed( p_data->data );
			reset();
		}
	}
	p_data->buffer = QByteArray();
};
