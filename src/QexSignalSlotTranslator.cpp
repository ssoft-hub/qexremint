#include "QexSignalSlotTranslator.h"
#include "QexSignalPacker.h"
#include "QexSlotActivator.h"

typedef QSharedPointer< QexSignalPacker > QexSignalPackerPointer;
typedef QHash< QByteArray, QexSignalPackerPointer > QexKeyToPackerMap;
typedef QHash< QByteArray, QexKeyToPackerMap > QexSignalToKeyToPackerMap;
typedef QHash< QObject *, QexSignalToKeyToPackerMap > QexSenderToSignalToKeyToPackerMap;

typedef QSharedPointer< QexSlotActivator > QexSlotActivatorPointer;
typedef QHash< QByteArray, QexSlotActivatorPointer > QexKeyToActivatorMap;
typedef QHash< QByteArray, QexKeyToActivatorMap > QexSignalToKeyToActivatorMap;
typedef QHash< QByteArray, QexSignalToKeyToActivatorMap > QexSlotToSignalToKeyToActivatorMap;
typedef QHash< QObject *, QexSlotToSignalToKeyToActivatorMap > QexReciverToSlotToSignalToKeyToActivatorMap;

////////////////////////////////////////////////////////////////////////////////

class QexSignalSlotTranslatorPrivate
{
public:
	QexSenderToSignalToKeyToPackerMap sender_signal_key_packer_map;
	QexReciverToSlotToSignalToKeyToActivatorMap reciver_slot_signal_key_activator_map;
};

////////////////////////////////////////////////////////////////////////////////

QexSignalSlotTranslator::QexSignalSlotTranslator ( QObject * parent )
	: QObject( parent )
	, p_data( new QexSignalSlotTranslatorPrivate )
{
};

QexSignalSlotTranslator::~QexSignalSlotTranslator ()
{
	delete p_data;
};

int QexSignalSlotTranslator::connectAllSignals ( const QByteArray & key, QObject * sender )
{
	int ret = 0;
	const QMetaObject * mo = sender->metaObject();
	for ( int i = 0; i < mo->methodCount(); ++i )
	{
		if ( mo->method( i ).methodType() == QMetaMethod::Signal )
		{
			QByteArray arr;
			arr += '0';
			arr += mo->method(i).signature();
			connectSignal( key, sender, arr.constData() );
			++ret;
		}
	}
	return ret;
};

bool QexSignalSlotTranslator::connectSignal ( const QByteArray & key, QObject * sender, const char * a_signal )
{
	// регистрация объекта
	QByteArray signal_name = QMetaObject::normalizedSignature( a_signal + 1 );

	const QexSenderToSignalToKeyToPackerMap const_map = p_data->sender_signal_key_packer_map;
	// проверка , что сигнал уже не зарегестрирован
	QexSenderToSignalToKeyToPackerMap::const_iterator s_iter = const_map.find( sender );
	if ( s_iter != const_map.end() )
	{
		QexSignalToKeyToPackerMap::const_iterator c_iter = ( *s_iter ).find( signal_name );
		if ( c_iter != ( *s_iter ).end() )
		{
			QexKeyToPackerMap::const_iterator k_iter = ( *c_iter ).find( key );
			if ( k_iter != ( *c_iter ).end() )
			{
				return true;
			}
		}
	};

	// создание паковщика
	QexSignalPackerPointer packer( new QexSignalPacker( key, sender, a_signal, this ) );
	if ( !packer->isValid() )
	{
		qWarning( "QexSignalSlotTranslator: Connected signal not valid" );
		return false;
	}

	// создание записи
	p_data->sender_signal_key_packer_map[ sender ][ signal_name ][ key ] = packer;

	// если sender будет удален, то необходимо почистить регистрацию
	disconnect( sender, SIGNAL( destroyed( QObject * ) ), this, SLOT( disconnectSignals( QObject * ) ) );
	connect( sender, SIGNAL( destroyed( QObject * ) ), this, SLOT( disconnectSignals( QObject * ) ) );

	// перенаправление сигнала
	connect( packer.data(), SIGNAL( signalPacked( const QByteArray & ) )
		, this, SIGNAL( signalPacked( const QByteArray & ) ) );

	return true;
};

void QexSignalSlotTranslator::disconnectSignal ( const QByteArray & key, QObject * sender, const char * a_signal )
{
	// удаление регистрации объекта
	QByteArray signal_name = QMetaObject::normalizedSignature( a_signal + 1 );

	const QexSenderToSignalToKeyToPackerMap const_map = p_data->sender_signal_key_packer_map;
	// проверка , что сигнал зарегестрирован
	QexSenderToSignalToKeyToPackerMap::const_iterator s_iter = const_map.find( sender );
	if ( s_iter != const_map.end() )
	{
		QexSignalToKeyToPackerMap::const_iterator c_iter = ( *s_iter ).find( signal_name );
		if ( c_iter != ( *s_iter ).end() )
		{
			QexKeyToPackerMap::const_iterator k_iter = ( *c_iter ).find( key );
			if ( k_iter != ( *c_iter ).end() )
			{
				p_data->sender_signal_key_packer_map[ sender ][ signal_name ].remove( key );
				if ( p_data->sender_signal_key_packer_map[ sender ][ signal_name ].isEmpty() )
				{
					p_data->sender_signal_key_packer_map[ sender ].remove( signal_name );
					if ( p_data->sender_signal_key_packer_map[ sender ].isEmpty() )
						p_data->sender_signal_key_packer_map.remove( sender );
				}
				return;
			}
		}
	}
};

void QexSignalSlotTranslator::disconnectSignals ( const QByteArray & key, QObject * sender )
{
	const QexSenderToSignalToKeyToPackerMap const_map = p_data->sender_signal_key_packer_map;
	// удаление регистрации объекта
	QexSenderToSignalToKeyToPackerMap::const_iterator s_iter = const_map.find( sender );
	if ( s_iter != const_map.end() )
	{
		for ( QexSignalToKeyToPackerMap::const_iterator c_iter = ( *s_iter ).begin();
			c_iter != ( *s_iter ).end(); ++c_iter )
		{
			const QByteArray & signal_name = c_iter.key();
			QexKeyToPackerMap::const_iterator k_iter = ( *c_iter ).find( key );
			if ( k_iter != ( *c_iter ).end() )
			{
				p_data->sender_signal_key_packer_map[ sender ][ signal_name ].remove( key );
				if ( p_data->sender_signal_key_packer_map[ sender ][ signal_name ].isEmpty() )
				{
					p_data->sender_signal_key_packer_map[ sender ].remove( signal_name );
					if ( p_data->sender_signal_key_packer_map[ sender ].isEmpty() )
						p_data->sender_signal_key_packer_map.remove( sender );
				}
			}
		}
	}
};

void QexSignalSlotTranslator::disconnectSignals ( QObject * sender, const char * a_signal )
{
	// удаление регистрации объекта и его сигнала
	QByteArray signal_name = QMetaObject::normalizedSignature( a_signal + 1 );

	const QexSenderToSignalToKeyToPackerMap const_map = p_data->sender_signal_key_packer_map;
	// проверка , что сигнал зарегестрирован
	QexSenderToSignalToKeyToPackerMap::const_iterator s_iter = const_map.find( sender );
	if ( s_iter != const_map.end() )
	{
		QexSignalToKeyToPackerMap::const_iterator c_iter = ( *s_iter ).find( signal_name );
		if ( c_iter != ( *s_iter ).end() )
		{
			for ( QexKeyToPackerMap::const_iterator k_iter = ( *c_iter ).begin();
				k_iter != ( *c_iter ).end(); ++k_iter )
			{
				const QByteArray & key = k_iter.key();

				p_data->sender_signal_key_packer_map[ sender ][ signal_name ].remove( key );
				if ( p_data->sender_signal_key_packer_map[ sender ][ signal_name ].isEmpty() )
				{
					p_data->sender_signal_key_packer_map[ sender ].remove( signal_name );
					if ( p_data->sender_signal_key_packer_map[ sender ].isEmpty() )
						p_data->sender_signal_key_packer_map.remove( sender );
				}
			}
		}
	}
};

void QexSignalSlotTranslator::disconnectSignals ( QObject * sender )
{
	const QexSenderToSignalToKeyToPackerMap const_map = p_data->sender_signal_key_packer_map;
	// удаление регистрации объекта
	// проверка , что объект зарегестрирован
	QexSenderToSignalToKeyToPackerMap::const_iterator s_iter = const_map.find( sender );
	if ( s_iter != const_map.end() )
	{
		for ( QexSignalToKeyToPackerMap::const_iterator c_iter = ( *s_iter ).begin(); 
			c_iter != ( *s_iter ).end(); ++c_iter )
		{
			const QByteArray & signal_name = c_iter.key();
			for ( QexKeyToPackerMap::const_iterator k_iter = ( *c_iter ).begin();
				k_iter != ( *c_iter ).end(); ++k_iter )
			{
				const QByteArray & key = k_iter.key();

				p_data->sender_signal_key_packer_map[ sender ][ signal_name ].remove( key );
				if ( p_data->sender_signal_key_packer_map[ sender ][ signal_name ].isEmpty() )
				{
					p_data->sender_signal_key_packer_map[ sender ].remove( signal_name );
					if ( p_data->sender_signal_key_packer_map[ sender ].isEmpty() )
						p_data->sender_signal_key_packer_map.remove( sender );
				}
			}
		}
		p_data->sender_signal_key_packer_map.remove( sender );
		return;
	};
};

void QexSignalSlotTranslator::disconnectAllSignals ()
{
	p_data->sender_signal_key_packer_map.clear();
};

bool QexSignalSlotTranslator::connectSlot ( const QByteArray & key, const char * a_signal, QObject * reciver, const char * a_slot )
{
	// регистрация объекта
	QByteArray signal_name = QMetaObject::normalizedSignature( a_signal + 1 );
	QByteArray slot_name = QMetaObject::normalizedSignature( a_slot + 1 );

	const QexReciverToSlotToSignalToKeyToActivatorMap const_map = p_data->reciver_slot_signal_key_activator_map;
	QexReciverToSlotToSignalToKeyToActivatorMap::const_iterator reciver_iter = const_map.find( reciver );
	if ( reciver_iter != const_map.end() )
	{
		QexSlotToSignalToKeyToActivatorMap::const_iterator slot_iter = ( *reciver_iter ).find( slot_name );
		if ( slot_iter != ( *reciver_iter ).end() )
		{
			QexSignalToKeyToActivatorMap::const_iterator signal_iter = ( *slot_iter ).find( signal_name );
			if ( signal_iter != ( *slot_iter ).end() )
			{
				QexKeyToActivatorMap::const_iterator key_iter = ( *signal_iter ).find( key );
				if ( key_iter != ( *signal_iter ).end() )
				{
					return true;
				}
			}
		}
	}

	// создание активатора
	QexSlotActivatorPointer activator( new QexSlotActivator( key, a_signal, reciver, a_slot, this ) );
	if ( !activator->isValid() )
	{
		qWarning( "QexSignalSlotTranslator: Connected slot not valid" );
		return false;
	}

	// создание записи
	p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ][ key ] = activator;

	// если reciver будет удален, то необходимо почистить регистрацию
	disconnect( reciver, SIGNAL( destroyed( QObject * ) ), this, SLOT( disconnectSlots( QObject * ) ) );
	connect( reciver, SIGNAL( destroyed( QObject * ) ), this, SLOT( disconnectSlots( QObject * ) ) );

	return true;
};

void QexSignalSlotTranslator::disconnectSlot ( const QByteArray & key, const char * a_signal, QObject * reciver, const char * a_slot )
{
	// удаление регистрации
	QByteArray signal_name = QMetaObject::normalizedSignature( a_signal + 1 );
	QByteArray slot_name = QMetaObject::normalizedSignature( a_slot + 1 );

	const QexReciverToSlotToSignalToKeyToActivatorMap const_map = p_data->reciver_slot_signal_key_activator_map;
	QexReciverToSlotToSignalToKeyToActivatorMap::const_iterator reciver_iter = const_map.find( reciver );
	if ( reciver_iter != const_map.end() )
	{
		QexSlotToSignalToKeyToActivatorMap::const_iterator slot_iter = ( *reciver_iter ).find( slot_name );
		if ( slot_iter != ( *reciver_iter ).end() )
		{
			QexSignalToKeyToActivatorMap::const_iterator signal_iter = ( *slot_iter ).find( signal_name );
			if ( signal_iter != ( *slot_iter ).end() )
			{
				QexKeyToActivatorMap::const_iterator key_iter = ( *signal_iter ).find( key );
				if ( key_iter != ( *signal_iter ).end() )
				{
					p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ].remove( key );
					if ( p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ].isEmpty() )
					{
						p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ].remove( signal_name );
						if ( p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ].isEmpty() )
						{
							p_data->reciver_slot_signal_key_activator_map[ reciver ].remove( slot_name );
							if ( p_data->reciver_slot_signal_key_activator_map[ reciver ].isEmpty() )
							{
								p_data->reciver_slot_signal_key_activator_map.remove( reciver );
							}
						}
					}
					return;
				}
			}
		}
	}
};

void QexSignalSlotTranslator::disconnectSlots ( const char * a_signal, QObject * reciver, const char * a_slot )
{
	// удаление регистрации
	QByteArray signal_name = QMetaObject::normalizedSignature( a_signal + 1 );
	QByteArray slot_name = QMetaObject::normalizedSignature( a_slot + 1 );

	const QexReciverToSlotToSignalToKeyToActivatorMap const_map;
	QexReciverToSlotToSignalToKeyToActivatorMap::const_iterator reciver_iter = const_map.find( reciver );
	if ( reciver_iter != const_map.end() )
	{
		QexSlotToSignalToKeyToActivatorMap::const_iterator slot_iter = ( *reciver_iter ).find( slot_name );
		if ( slot_iter != ( *reciver_iter ).end() )
		{
			QexSignalToKeyToActivatorMap::const_iterator signal_iter = ( *slot_iter ).find( signal_name );
			if ( signal_iter != ( *slot_iter ).end() )
			{
				for ( QexKeyToActivatorMap::const_iterator key_iter = ( *signal_iter ).begin();
					key_iter != ( *signal_iter ).end(); ++key_iter )
				{
					const QByteArray & key = key_iter.key();

					p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ].remove( key );
					if ( p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ].isEmpty() )
					{
						p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ].remove( signal_name );
						if ( p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ].isEmpty() )
						{
							p_data->reciver_slot_signal_key_activator_map[ reciver ].remove( slot_name );
							if ( p_data->reciver_slot_signal_key_activator_map[ reciver ].isEmpty() )
							{
								p_data->reciver_slot_signal_key_activator_map.remove( reciver );
							}
						}
					}
				}
			}
		}
	}
};

void QexSignalSlotTranslator::disconnectSlots ( QObject * reciver, const char * a_slot )
{
	// удаление регистрации
	QByteArray slot_name = QMetaObject::normalizedSignature( a_slot + 1 );

	const QexReciverToSlotToSignalToKeyToActivatorMap const_map;
	QexReciverToSlotToSignalToKeyToActivatorMap::const_iterator reciver_iter = const_map.find( reciver );
	if ( reciver_iter != const_map.end() )
	{
		QexSlotToSignalToKeyToActivatorMap::const_iterator slot_iter = ( *reciver_iter ).find( slot_name );
		if ( slot_iter != ( *reciver_iter ).end() )
		{
			for ( QexSignalToKeyToActivatorMap::const_iterator signal_iter = ( *slot_iter ).begin();
				signal_iter != ( *slot_iter ).end(); ++signal_iter )
			{
				const QByteArray & signal_name = signal_iter.key();

				for ( QexKeyToActivatorMap::const_iterator key_iter = ( *signal_iter ).begin();
					key_iter != ( *signal_iter ).end(); ++key_iter )
				{
					const QByteArray & key = key_iter.key();

					p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ].remove( key );
					if ( p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ].isEmpty() )
					{
						p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ].remove( signal_name );
						if ( p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ].isEmpty() )
						{
							p_data->reciver_slot_signal_key_activator_map[ reciver ].remove( slot_name );
							if ( p_data->reciver_slot_signal_key_activator_map[ reciver ].isEmpty() )
							{
								p_data->reciver_slot_signal_key_activator_map.remove( reciver );
							}
						}
					}
				}
			}
		}
	}
};

void QexSignalSlotTranslator::disconnectSlots ( QObject * reciver )
{
	const QexReciverToSlotToSignalToKeyToActivatorMap const_map = p_data->reciver_slot_signal_key_activator_map;
	QexReciverToSlotToSignalToKeyToActivatorMap::const_iterator reciver_iter = const_map.find( reciver );
	if ( reciver_iter != const_map.end() )
	{
		for ( QexSlotToSignalToKeyToActivatorMap::const_iterator slot_iter = ( *reciver_iter ).begin();
			slot_iter != ( *reciver_iter ).end(); ++slot_iter )
		{
			const QByteArray & slot_name = slot_iter.key();

			for ( QexSignalToKeyToActivatorMap::const_iterator signal_iter = ( *slot_iter ).begin();
				signal_iter != ( *slot_iter ).end(); ++signal_iter )
			{
				const QByteArray & signal_name = signal_iter.key();

				for ( QexKeyToActivatorMap::const_iterator key_iter = ( *signal_iter ).begin();
					key_iter != ( *signal_iter ).end(); ++key_iter )
				{
					const QByteArray & key = key_iter.key();

					p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ].remove( key );
					if ( p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ][ signal_name ].isEmpty() )
					{
						p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ].remove( signal_name );
						if ( p_data->reciver_slot_signal_key_activator_map[ reciver ][ slot_name ].isEmpty() )
						{
							p_data->reciver_slot_signal_key_activator_map[ reciver ].remove( slot_name );
							if ( p_data->reciver_slot_signal_key_activator_map[ reciver ].isEmpty() )
							{
								p_data->reciver_slot_signal_key_activator_map.remove( reciver );
							}
						}
					}
				}
			}
		}
	}
};

void QexSignalSlotTranslator::disconnectAllSlots ()
{
	p_data->reciver_slot_signal_key_activator_map.clear();
};

void QexSignalSlotTranslator::activateSlot ( const QByteArray & data )
{
	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::ReadOnly );
	QByteArray data_sender_key;
	QByteArray data_signal_signature;

	stream >> data_sender_key >> data_signal_signature;

	const QexReciverToSlotToSignalToKeyToActivatorMap const_map = p_data->reciver_slot_signal_key_activator_map;
	for ( QexReciverToSlotToSignalToKeyToActivatorMap::const_iterator reciver_iter = const_map.begin();
		reciver_iter != const_map.end(); ++reciver_iter )
	{
		for ( QexSlotToSignalToKeyToActivatorMap::const_iterator slot_iter = ( *reciver_iter ).begin();
			slot_iter != ( *reciver_iter ).end(); ++slot_iter )
		{
			QexSignalToKeyToActivatorMap::const_iterator signal_iter = ( *slot_iter ).find( data_signal_signature );
			if ( signal_iter != ( *slot_iter ).end() )
			{
				QexKeyToActivatorMap::const_iterator key_iter = ( *signal_iter ).find( data_sender_key );
				if ( key_iter != ( *signal_iter ).end() )
				{
					// здесь используется прямой вызов,
					// а внутри происходит вызов invokeMethod с параметром Qt::AutoConnection
					key_iter.value()->activate( data );
				}
			}
		}
	}
};
