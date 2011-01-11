#include "QexPropertyTranslator.h"
#include "QexPropertyRequester.h"

typedef QPointer< QObject > QObjectPointer;

////////////////////////////////////////////////////////////////////////////////

struct QexPropertyRequestGroup
{
	int group_id;
	QObjectPointer requester;
	QByteArray ret_type;
	QByteArray slot_member;

	QexPropertyRequestGroup ( int id, QObject * req, const QByteArray & type, const QByteArray & member )
		: group_id( id )
		, requester( req )
		, ret_type( type )
		, slot_member( member )
	{
	};

	QexPropertyRequestGroup ( const QexPropertyRequestGroup & other )
		: group_id( other.group_id )
		, requester( other.requester )
		, ret_type( other.ret_type )
		, slot_member( other.slot_member )
	{
	};

	QexPropertyRequestGroup & operator = ( const QexPropertyRequestGroup & other )
	{
		group_id = other.group_id;
		requester = other.requester;
		ret_type = other.ret_type;
		slot_member = other.slot_member;
		return *this;
	};

	bool operator == ( const QexPropertyRequestGroup & other ) const
	{
		return group_id == other.group_id
			&& requester == other.requester
			&& ret_type == other.ret_type
			&& slot_member == other.slot_member;
	};

	bool operator != ( const QexPropertyRequestGroup & other ) const
	{
		return !( *this == other );
	};
};

typedef QList< QexPropertyRequestGroup > QexPropertyRequestGroupList;

////////////////////////////////////////////////////////////////////////////////

typedef QHash< QByteArray, QObjectPointer > QexPropertyToRespondentMap;
typedef QHash< QByteArray, QexPropertyToRespondentMap > QexKeyToPropertyToRespondentMap;

////////////////////////////////////////////////////////////////////////////////

class QexPropertyTranslatorPrivate
{
public:
	QexPropertyRequestGroupList requester_group_list;
	QexKeyToPropertyToRespondentMap key_property_respondent_map;
};

////////////////////////////////////////////////////////////////////////////////

QexPropertyTranslator::QexPropertyTranslator ( QObject * parent )
	: QObject( parent )
	, p_data( new QexPropertyTranslatorPrivate )
{
};

QexPropertyTranslator::~QexPropertyTranslator ()
{
	delete p_data;
};

bool QexPropertyTranslator::getProperty ( const QByteArray & key, const char * a_property
										 , QObject * requester, const char * a_slot )
{
	if ( key.isEmpty() || !a_property || !requester || !a_slot )
		return false;

	// проверка корректости исходных данных
	QByteArray property_name = QMetaObject::normalizedSignature( a_property + 1 );
	QByteArray slot_name = QMetaObject::normalizedSignature( a_slot + 1 );

	int slot_index = requester->metaObject()->indexOfMethod( slot_name.constData() );
	if ( slot_index < 0 )
	{
		qWarning( "QexMethodTranslator: Requester has no a slot %s", slot_name.constData() );
		return false;
	}

	int ret_beg = slot_name.indexOf( '(' ) + 1;
	int ret_end = slot_name.lastIndexOf( ')' );
	if ( ret_end <= ret_beg )
	{
		qWarning( "QexMethodTranslator: The slot has no input value%s", slot_name.constData() );
		return false;
	}

	QByteArray ret_type = slot_name.mid( ret_beg, ret_end - ret_beg );
	if ( ret_type.contains( ',' ) )
	{
		qWarning( "QexMethodTranslator: The slot %s has many input values but must be one", slot_name.constData() );
		return false;
	}

	// определяем id запроса
	static int id = 0;
	++id;

	// запоминаем по id обработчик ответа (requester) и метод обработки ответа (a_slot).
	QByteArray member = slot_name;
	member.truncate( slot_name.indexOf( '(' ) );
	p_data->requester_group_list.append( QexPropertyRequestGroup( id, requester, ret_type, member ) );

	// пакуем метод
	// << id << key << property_name << ret_type << argument_types << argument_list;
	QByteArray data;
	QDataStream stream( &data, QIODevice::WriteOnly );
	stream << id << key << property_name << ret_type;

	emit getPropertyPacked( data );

	return true;
};

bool QexPropertyTranslator::setProperty ( const QByteArray & key, const char * a_property, QVariant value )
{
	return setProperty( key, a_property, QGenericArgument( value.typeName(), value.data() ) );
};

bool QexPropertyTranslator::setProperty ( const QByteArray & key, const char * a_property, QGenericArgument arg )
{
	if ( key.isEmpty() || !a_property )
		return false;

	// проверка корректости исходных данных
	QByteArray property_name = QMetaObject::normalizedSignature( a_property + 1 );
	QByteArray property_type = arg.name();
	QByteArray property_value;
	QDataStream value_stream( &property_value, QIODevice::WriteOnly );
	if ( !QMetaType::save( value_stream, QMetaType::type( arg.name() ), arg.data() ) )
	{
		qWarning( "Can't save to stream type %s", property_type.constData() );
		return false;
	}

	// пакуем
	// << key << property_name << answer_type << answer;
	QByteArray data;
	QDataStream stream( &data, QIODevice::WriteOnly );
	stream << key << property_name << property_type << property_value;

	emit setPropertyPacked( data );

	return true;
};

int QexPropertyTranslator::connectAllProperties ( const QByteArray & key, QObject * respondent )
{
	int ret = 0;
	const QMetaObject * mo = respondent->metaObject();
	for ( int i = 0; i < mo->propertyCount(); ++i )
	{
		QByteArray property_type = mo->property(i).typeName();
		if ( !property_type.isEmpty() )
		{
			QByteArray arr;
			arr += '3';
			arr += mo->property(i).name();
			connectProperty( key, respondent, arr.constData() );
			++ret;
		}
	}
	return ret;
};

bool QexPropertyTranslator::connectProperty ( const QByteArray & key, QObject * respondent, const char * a_property )
{
	if ( !respondent || !a_property )
	{
		qWarning( "QexMethodTranslator: Incorrect inputs: respondent or property" );
		return false;
	}

	QByteArray property_name = QMetaObject::normalizedSignature( a_property + 1 );
	if ( respondent->metaObject()->indexOfProperty( property_name.constData() ) < 0 )
	{
		qWarning( "QexPropertyTranslator: The respondent has no property %s", property_name.constData() );
		return false;
	}

	const QexKeyToPropertyToRespondentMap const_map = p_data->key_property_respondent_map;
	QexKeyToPropertyToRespondentMap::const_iterator key_iter = const_map.find( key );
	if ( key_iter != const_map.end() )
	{
		QexPropertyToRespondentMap::const_iterator property_iter = ( *key_iter ).find( property_name );
		if ( property_iter != ( *key_iter ).end() )
		{
			if ( property_iter.value() == respondent )
			{
				return true;
			}
			else
			{
				qWarning( "QexMethodTranslator: The respondent already exists" );
				return false;
			}
		}
	}
	p_data->key_property_respondent_map[ key ][ property_name ] = respondent;
	return true;
};

void QexPropertyTranslator::disconnectProperty ( const QByteArray & key, QObject * respondent, const char * a_property )
{
	QByteArray property_name = QMetaObject::normalizedSignature( a_property + 1 );
	const QexKeyToPropertyToRespondentMap const_map = p_data->key_property_respondent_map;
	QexKeyToPropertyToRespondentMap::const_iterator key_iter = const_map.find( key );
	if ( key_iter != const_map.end() )
	{
		QexPropertyToRespondentMap::const_iterator property_iter = ( *key_iter ).find( property_name );
		if ( property_iter != ( *key_iter ).end() )
		{
			if ( property_iter.value() == respondent )
			{
				p_data->key_property_respondent_map[ key ].remove( property_name );
				if ( p_data->key_property_respondent_map[ key ].isEmpty() )
				{
					p_data->key_property_respondent_map.remove( key );
				}
				return;
			}
		}
	}
};

void QexPropertyTranslator::disconnectProperties ( QObject * respondent, const char * a_property )
{
	QByteArray property_name = QMetaObject::normalizedSignature( a_property + 1 );
	
	const QexKeyToPropertyToRespondentMap const_map = p_data->key_property_respondent_map;
	for ( QexKeyToPropertyToRespondentMap::const_iterator key_iter = const_map.begin();
		key_iter != const_map.end(); ++key_iter )
	{
		const QByteArray & key = key_iter.key();
		QexPropertyToRespondentMap::const_iterator property_iter = ( *key_iter ).find( property_name );
		if ( property_iter != ( *key_iter ).end() )
		{
			if ( property_iter.value() == respondent )
			{
				p_data->key_property_respondent_map[ key ].remove( property_name );
				if ( p_data->key_property_respondent_map[ key ].isEmpty() )
				{
					p_data->key_property_respondent_map.remove( key );
				}
			}
		}
	}
};

void QexPropertyTranslator::disconnectProperties ( QObject * respondent )
{
	const QexKeyToPropertyToRespondentMap const_map = p_data->key_property_respondent_map;
	for ( QexKeyToPropertyToRespondentMap::const_iterator key_iter = const_map.begin();
		key_iter != const_map.end(); ++key_iter )
	{
		const QByteArray & key = key_iter.key();
		for ( QexPropertyToRespondentMap::const_iterator property_iter = ( *key_iter ).begin();
			property_iter != ( *key_iter ).end(); ++property_iter )
		{
			const QByteArray & property_name = property_iter.key();
			if ( property_iter.value() == respondent )
			{
				p_data->key_property_respondent_map[ key ].remove( property_name );
				if ( p_data->key_property_respondent_map[ key ].isEmpty() )
				{
					p_data->key_property_respondent_map.remove( key );
				}
			}
		}
	}
};

void QexPropertyTranslator::disconnectAllProperties ()
{
	p_data->key_property_respondent_map.clear();
};

void QexPropertyTranslator::disconnectRequester ( QObject * requester, const char * a_slot )
{
	QByteArray slot_name = QMetaObject::normalizedSignature( a_slot + 1 );
	QByteArray member_name = slot_name;
	member_name.truncate( slot_name.indexOf( '(' ) );
	int ret_beg = slot_name.indexOf( '(' ) + 1;
	int ret_end = slot_name.lastIndexOf( ')' );
	QByteArray ret_type = slot_name.mid( ret_beg, ret_end - ret_beg );

	const QexPropertyRequestGroupList temp_list = p_data->requester_group_list;
	for ( QexPropertyRequestGroupList::const_iterator iter = temp_list.begin();
		iter != temp_list.end(); ++iter )
	{
		if ( ( *iter ).requester == requester
			&& ( *iter ).slot_member == member_name
			&& ( *iter ).ret_type == ret_type )
		{
			p_data->requester_group_list.removeOne( *iter );
		}
	}
};

void QexPropertyTranslator::disconnectRequester ( QObject * requester )
{
	const QexPropertyRequestGroupList temp_list = p_data->requester_group_list;
	for ( QexPropertyRequestGroupList::const_iterator iter = temp_list.begin();
		iter != temp_list.end(); ++iter )
	{
		if ( ( *iter ).requester == requester )
		{
			p_data->requester_group_list.removeOne( *iter );
		}
	}
};

void QexPropertyTranslator::disconnectAllRequesters ()
{
	p_data->requester_group_list.clear();
};

void QexPropertyTranslator::activateGetProperty ( const QByteArray & data )
{
	// распаковываем
	// >> id >> key >> property_name >> property_type;
	int id;
	QByteArray key;
	QByteArray property_name;
	QByteArray property_type;
	QByteArray answer;

	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::ReadOnly );
	stream >> id >> key >> property_name >> property_type;

	// ищем подписчика
	const QexKeyToPropertyToRespondentMap const_map = p_data->key_property_respondent_map;
	QexKeyToPropertyToRespondentMap::const_iterator key_iter = const_map.find( key );
	if ( key_iter == const_map.end() )
	{
		property_type = QByteArray();
		answer = QByteArray( "There is no property respondent with key " ) + key;
	}

	if ( answer.isEmpty() )
	{
		QexPropertyToRespondentMap::const_iterator property_iter = ( *key_iter ).find( property_name );
		if ( property_iter == ( *key_iter ).end() || property_iter.value().isNull() )
		{
			property_type = QByteArray();
			answer = QByteArray( "There is no respondent with property " ) + property_name;

			if ( property_iter != ( *key_iter ).end() )
				disconnectProperties( 0 );
		}

		if ( answer.isEmpty() )
		{
			QObject * respondent = property_iter.value();
			QexPropertyRequester * req = new QexPropertyRequester( respondent, data );
			req->moveToThread( respondent->thread() );

			connect( req, SIGNAL( answerPacked( const QByteArray & ) )
				, this, SIGNAL( answerPacked( const QByteArray & ) ) );

			QMetaObject::invokeMethod ( req, "request" );
		}
	}

	if ( !answer.isEmpty() ) // если есть что вернуть
	{
		// пакуем и отправляем ответ
		QByteArray ans;
		QDataStream ans_stream( &ans, QIODevice::WriteOnly );
		ans_stream << id << key << property_name << property_type << answer;
		emit answerPacked( ans );
	}
};

void QexPropertyTranslator::activateSetProperty ( const QByteArray & data )
{
	// распаковываем
	// >> id >> key >> property_name >> property_type;
	QByteArray key;
	QByteArray property_name;
	QByteArray property_type;
	QByteArray property_value;

	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::ReadOnly );
	stream >> key >> property_name >> property_type;

	// ищем подписчика
	const QexKeyToPropertyToRespondentMap const_map = p_data->key_property_respondent_map;
	QexKeyToPropertyToRespondentMap::const_iterator key_iter = const_map.find( key );
	if ( key_iter == p_data->key_property_respondent_map.end() )
	{
		property_type = QByteArray();
		property_value = QByteArray( "There is no property respondent with key " ) + key;
	}

	if ( !property_type.isEmpty() )
	{
		QexPropertyToRespondentMap::const_iterator property_iter = ( *key_iter ).find( property_name );
		if ( property_iter == ( *key_iter ).end() )
		{
			property_type = QByteArray();
			property_value = QByteArray( "There is no respondent with property " ) + property_name;
		}

		if ( !property_type.isEmpty() )
		{
			QObject * respondent = property_iter.value();
			if ( respondent )
			{
				int property_type_id = QMetaType::type( property_type.constData() );
				void * value_unit = QMetaType::construct( property_type_id );
				stream >> property_value;

				QDataStream value_stream( &property_value, QIODevice::ReadOnly );
				QMetaType::load( value_stream, property_type_id, value_unit );

				QVariant variant( property_type_id, value_unit );

				if ( !respondent->setProperty( property_name.constData(), variant ) )
				{
					qWarning( "Set property error" );
				}

				QMetaType::destroy( property_type_id, value_unit );
			}
			else
				disconnectProperties( respondent );
		}
	}
};

void QexPropertyTranslator::activateAnswer ( const QByteArray & data )
{
	// распаковываем ответ
	QByteArray ans = data;

	int id;
	QByteArray key;
	QByteArray property_name;
	QByteArray answer_type;
	QByteArray answer;

	QDataStream stream( &ans, QIODevice::ReadOnly );
	stream >> id >> key >> property_name >> answer_type >> answer;

	if ( answer_type.isEmpty() )
	{
		qWarning( "QexPropertyTranslator: answer error %s", answer.constData() );
	}

	const QexPropertyRequestGroupList temp_list = p_data->requester_group_list;
	QexPropertyRequestGroupList::const_iterator iter = temp_list.begin();
	while ( iter != temp_list.end() )
	{
		if ( ( *iter ).group_id == id )
			break;
		++iter;
	}

	bool answer_error = answer_type.isEmpty() && !answer.isEmpty();
	if ( answer_error )
	{
		qWarning( "Property request error: %s", answer.constData() );
	}

	if ( iter != temp_list.end() )
	{
		QObject * requester = ( *iter ).requester;
		QByteArray slot_member = ( *iter ).slot_member;
		p_data->requester_group_list.removeOne( *iter );

		if ( requester && !answer_error )
		{
			int type_id = QMetaType::type( answer_type.constData() );
			void * arg = QMetaType::construct( type_id );

			QDataStream arg_stream( &answer, QIODevice::ReadOnly );
			QMetaType::load( arg_stream, type_id, arg );
			QMetaObject::invokeMethod( requester, slot_member.constData()
				, QGenericArgument( answer_type.constData(), arg ) );
			QMetaType::destroy( type_id, arg );
		}
	}
	else
	{		
		qWarning( "QexPropertyTranslator: There is no property_value requester" );
	}
};
