#include "QexSignalPacker.h"

////////////////////////////////////////////////////////////////////////////////

class QexSignalPackerPrivate
{
public:
	QObject * object_sender;
	QByteArray sender_key;
	QByteArray signal_signature;
	QList< QByteArray > argument_types;
};

////////////////////////////////////////////////////////////////////////////////

QexSignalPacker::QexSignalPacker ( const QByteArray & key, QObject * sender, const char * a_signal, QObject * parent )
	: QObject( parent )
	, p_data( new QexSignalPackerPrivate )
{
	p_data->object_sender = 0;

	if ( !sender || !a_signal )
	{
		qWarning( "QexSignalPacker: Not a valid input data" );
		return;
	}

	if ( key.isEmpty() )
	{
		qWarning( "QexSignalPacker: The sender key is empty" );
		return;
	}

	QByteArray ba = QMetaObject::normalizedSignature( a_signal + 1 );
	const QMetaObject * mo = sender->metaObject();
	int signal_index = mo->indexOfMethod( ba.constData() );

	p_data->object_sender = sender;
	p_data->sender_key = key;
	p_data->signal_signature = ba;
	p_data->argument_types = mo->method( signal_index ).parameterTypes();

	QByteArray slot_name = "inputSignal(";
	for ( int i = 0; i < p_data->argument_types.count(); )
	{
		slot_name += ( ++i == p_data->argument_types.count() ) ? "void*" : "void*,";
	}
	slot_name += ')';
	slot_name = QMetaObject::normalizedSignature( slot_name.constData() );

	int slot_index = metaObject()->indexOfMethod( slot_name.constData() );

	if ( slot_index < 0 )
	{
		qFatal( "QexSignalPacker: No such slot: '%s'", slot_name.constData() );
		return;
	}

	if ( !QMetaObject::connect( sender, signal_index, this, slot_index, Qt::AutoConnection, 0 ) )
	{
		qWarning( "QexSignalPacker: QMetaObject::connect returned false. Unable to connect." );
		return;
	}

	connect( sender, SIGNAL( destroyed() ), this, SLOT( clean() ) );
};

QexSignalPacker::~QexSignalPacker ()
{
	delete p_data;
};

bool QexSignalPacker::isValid () const
{
	return !p_data->signal_signature.isEmpty();
}

QObject * QexSignalPacker::packedSender () const
{
	return p_data->object_sender;
};

QByteArray QexSignalPacker::senderKey () const
{
	return p_data->sender_key;
};

QByteArray QexSignalPacker::signalSignature () const
{
	return p_data->signal_signature;
};

void QexSignalPacker::clean ()
{
	disconnect( p_data->object_sender );
	p_data->object_sender = 0;
	p_data->signal_signature = QByteArray();
	p_data->argument_types.clear();
};

void QexSignalPacker::packSignal ( void ** a )
{
	if ( !isValid() )
		return;

	QList< QByteArray > argument_list;
	for ( int i = 0; i < p_data->argument_types.count(); ++i )
	{
		QByteArray arr;
		QDataStream stream( &arr, QIODevice::WriteOnly );
		if ( !QMetaType::save( stream, QMetaType::type( p_data->argument_types.at( i ) ), a[ i ] ) )
		{
			qWarning( "Can't save to stream type %s", p_data->argument_types.at( i ).constData() );
			return;
		};

		argument_list.append( arr );
	}

	QByteArray arr;
	QDataStream stream( &arr, QIODevice::WriteOnly );
	stream << p_data->sender_key << p_data->signal_signature << p_data->argument_types << argument_list;

	emit signalPacked( arr );
}

void QexSignalPacker::inputSignal ()
{
	void ** _a = 0;
	packSignal( _a );
}

void QexSignalPacker::inputSignal ( const void * & a1 )
{
	const void ** _a[] = { &a1 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2 )
{
	const void ** _a[] = { &a1, &a2 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2, const void * & a3 )
{
	const void ** _a[] = { &a1, &a2, &a3 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4 )
{
	const void ** _a[] = { &a1, &a2, &a3, &a4 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5 )
{
	const void ** _a[] = { &a1, &a2, &a3, &a4, &a5 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
								   , const void * & a6 )
{
	const void ** _a[] = { &a1, &a2, &a3, &a4, &a5, &a6 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
								   , const void * & a6, const void * & a7 )
{
	const void ** _a[] = { &a1, &a2, &a3, &a4, &a5, &a6, &a7 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
								   , const void * & a6, const void * & a7, const void * & a8 )
{
	const void ** _a[] = { &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
								   , const void * & a6, const void * & a7, const void * & a8, const void * & a9 )
{
	const void ** _a[] = { &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}

void QexSignalPacker::inputSignal ( const void * & a1, const void * & a2, const void * & a3, const void * & a4, const void * & a5
								   , const void * & a6, const void * & a7, const void * & a8, const void * & a9, const void * & a10 )
{
	const void ** _a[] = { &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10 };
	packSignal( reinterpret_cast< void ** > ( _a ) );
}
