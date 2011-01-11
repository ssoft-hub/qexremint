#include "QexSlotActivator.h"

////////////////////////////////////////////////////////////////////////////////

class QexSlotActivatorPrivate
{
public:
	QObject * object_reciver;
	QByteArray sender_key;
	QByteArray slot_signature;
	QByteArray signal_signature;
	int slot_arg_count;
};

////////////////////////////////////////////////////////////////////////////////

QexSlotActivator::QexSlotActivator ( const QByteArray & key, const char * a_signal
									, QObject * reciver, const char * a_slot, QObject * parent )
	: QObject( parent )
	, p_data( new QexSlotActivatorPrivate )
{
	p_data->object_reciver = 0;

	if ( !reciver || !a_signal || !a_slot )
	{
		qWarning( "QexSlotActivator: Not a valid input data" );
		return;
	}

	if ( key.isEmpty() )
	{
		qWarning( "QexSlotActivator: The key is empty" );
		return;
	}

	// нормализация сигнатур
	QByteArray signal_name = QMetaObject::normalizedSignature( a_signal + 1 );
	QByteArray slot_name = QMetaObject::normalizedSignature( a_slot + 1 );

	// проверка существования слота
	const QMetaObject * mo = reciver->metaObject();
	int slot_index = mo->indexOfMethod( slot_name );
	if ( slot_index < 0 )
	{
		qWarning( "QexSlotActivator: No such slot: '%s'", slot_name.constData() );
		return;
	}

	// проверка соответствия сигнатур сигнала и слота
	QByteArray temp_slot_name = slot_name;
	temp_slot_name.chop( 1 ); // отрезание в конце ')'
	if ( !signal_name.mid( signal_name.lastIndexOf( '(' ) )
		.startsWith( temp_slot_name.mid( temp_slot_name.lastIndexOf( '(' ) ) ) )
	{
		qWarning( "QexSlotActivator: The signal and slot signatures are not compatible" );
		return;
	}

	p_data->object_reciver = reciver;
	p_data->sender_key = key;
	p_data->slot_signature = slot_name.left( slot_name.indexOf( '(' ) );
	p_data->signal_signature = signal_name;
	p_data->slot_arg_count = mo->method( slot_index ).parameterTypes().count();

	connect( reciver, SIGNAL( destroyed() ), this, SLOT( clean() ) );
};

QexSlotActivator::~QexSlotActivator ()
{
	delete p_data;
};

bool QexSlotActivator::isValid () const
{
	return p_data->object_reciver != 0;
};

QObject * QexSlotActivator::activatingReciver () const
{
	return p_data->object_reciver;
};

QByteArray QexSlotActivator::senderKey () const
{
	return p_data->sender_key;
};

QByteArray QexSlotActivator::signalSignature () const
{
	return p_data->signal_signature;
};

QByteArray QexSlotActivator::slotSignature () const
{
	return p_data->slot_signature;
};

void QexSlotActivator::clean ()
{
	disconnect( p_data->object_reciver );
	p_data->object_reciver = 0;
	p_data->sender_key = QByteArray();
	p_data->slot_signature = QByteArray();
	p_data->signal_signature = QByteArray();
};

void QexSlotActivator::activate ( const QByteArray & data )
{
	if ( !isValid() )
		return;

	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::ReadOnly );
	QByteArray data_sender_key;
	QByteArray data_signal_signature;
	QList< QByteArray > data_argument_types;
	QList< QByteArray > data_argument_list;

	stream >> data_sender_key >> data_signal_signature >> data_argument_types >> data_argument_list;

	// проверка имени посылающего объекта
	if ( p_data->sender_key != data_sender_key )
	{
		qWarning( "QexSlotActivator: Different sender key" );
		return;
	}

	// проверка сигнатуры вызвавшего сигнала
	if ( p_data->signal_signature != data_signal_signature )
	{
		qWarning( "QexSlotActivator: Different signal signature" );
		return;
	}

	// проверка количества данных и типов
	if ( data_argument_types.count() != data_argument_list.count() )
	{
		qWarning( "QexSlotActivator: Different counts of types and arguments at input data" );
		return;
	}

	// проверка максимального количества данных
	if ( data_argument_types.count() > 10 )
	{
		qWarning( "QexSlotActivator: There are many arguments %d, must be <= 10", data_argument_types.count() );
		return;
	}

	// подсчет 
	QGenericArgument ga[ 10 ];
	QList< void * > argument_list;

	int i;
	for ( i = 0; i < p_data->slot_arg_count; ++i )
	{
		int type_id = QMetaType::type( data_argument_types.at( i ).constData() );
		if ( type_id == 0 ) break;
		void * arg = QMetaType::construct( type_id );
		argument_list.append( arg );

		QByteArray arg_arr = data_argument_list.at( i );
		QDataStream arg_stream( &arg_arr, QIODevice::ReadOnly );
		if ( !QMetaType::load( arg_stream, QMetaType::type( data_argument_types.at( i ).constData() ), arg ) )
		{
			qWarning( "Can't load from stream type %s", data_argument_types.at( i ).constData() );
		}

		ga[i] = QGenericArgument( data_argument_types.at( i ).constData(), arg );
	}

	if ( i == p_data->slot_arg_count )
	{
		QMetaObject::invokeMethod( p_data->object_reciver, p_data->slot_signature.constData()
			, ga[0], ga[1], ga[2], ga[3], ga[4], ga[5], ga[6], ga[7], ga[8], ga[9] );
	}

	for ( i = 0; i < argument_list.count(); ++i )
	{
		int type_id = QMetaType::type( data_argument_types.at( i ).constData() );
		QMetaType::destroy( type_id, argument_list.at( i ) );
	}
};
