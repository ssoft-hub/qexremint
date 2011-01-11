#include "QexMethodTranslator.h"
#include "QexMethodInvoker.h"

typedef QPointer< QObject > QObjectPointer;
////////////////////////////////////////////////////////////////////////////////

struct QexMethodRequestGroup
{
	int group_id;
	QObjectPointer requester;
	QByteArray ret_type;
	QByteArray slot_member;

	QexMethodRequestGroup ( int id, QObject * req, const QByteArray & type, const QByteArray & member )
		: group_id( id )
		, requester( req )
		, ret_type( type )
		, slot_member( member )
	{
		if ( !requester )
			group_id = 0;
	};

	~QexMethodRequestGroup ()
	{
	};

	bool operator == ( const QexMethodRequestGroup & other ) const
	{
		return group_id == other.group_id
			&& requester == other.requester
			&& ret_type == other.ret_type
			&& slot_member == other.slot_member;
	};

	bool operator != ( const QexMethodRequestGroup & other ) const
	{
		return !( *this == other );
	};
};

typedef QList< QexMethodRequestGroup > QexMethodRequestGroupList;

////////////////////////////////////////////////////////////////////////////////

typedef QHash< QByteArray, QObjectPointer > QexMethodToRespondentMap;
typedef QHash< QByteArray, QexMethodToRespondentMap > QexKeyToMethodToRespondentMap;

////////////////////////////////////////////////////////////////////////////////

class QexMethodTranslatorPrivate
{
public:
	QexMethodRequestGroupList requester_group_list;
	QexKeyToMethodToRespondentMap key_method_respondent_map;
};

////////////////////////////////////////////////////////////////////////////////

QexMethodTranslator::QexMethodTranslator ( QObject * parent )
	: QObject( parent )
	, p_data( new QexMethodTranslatorPrivate )
{
};

QexMethodTranslator::~QexMethodTranslator ()
{
	delete p_data;
};


bool QexMethodTranslator::callMethod ( const QByteArray & key, const char * a_method
								  , QObject * requester, const char * a_slot, QVariant arg1
								  , QVariant arg2, QVariant arg3, QVariant arg4
								  , QVariant arg5, QVariant arg6, QVariant arg7
								  , QVariant arg8, QVariant arg9, QVariant arg10 )
{
	return callMethod( key, a_method, requester, a_slot
		, QGenericArgument( arg1.typeName(), arg1.data() )
		, QGenericArgument( arg2.typeName(), arg2.data() )
		, QGenericArgument( arg3.typeName(), arg3.data() )
		, QGenericArgument( arg4.typeName(), arg4.data() )
		, QGenericArgument( arg5.typeName(), arg5.data() )
		, QGenericArgument( arg6.typeName(), arg6.data() )
		, QGenericArgument( arg7.typeName(), arg7.data() )
		, QGenericArgument( arg8.typeName(), arg8.data() )
		, QGenericArgument( arg9.typeName(), arg9.data() )
		, QGenericArgument( arg10.typeName(), arg10.data() ) );
};

bool QexMethodTranslator::callMethod ( const QByteArray & key, const char * a_method
								  , QObject * requester, const char * a_slot, QGenericArgument arg1
								  , QGenericArgument arg2, QGenericArgument arg3, QGenericArgument arg4
								  , QGenericArgument arg5, QGenericArgument arg6, QGenericArgument arg7
								  , QGenericArgument arg8, QGenericArgument arg9, QGenericArgument arg10 )
{
	if ( key.isEmpty() || !a_method )
		return false;

	// проверка корректости исходных данных
	bool need_answer = requester && a_slot;
	QByteArray slot_name = need_answer
		? QMetaObject::normalizedSignature( a_slot + 1 )
		: QByteArray();
	QByteArray ret_type;

	if ( need_answer )
	{
		int slot_index = requester->metaObject()->indexOfMethod( slot_name );
		if ( slot_index < 0 )
		{
			qWarning( "QexMethodTranslator: Requester has no a slot %s", slot_name.constData() );
			return false;
		}

		int ret_beg = slot_name.indexOf( '(' ) + 1;
		int ret_end = slot_name.lastIndexOf( ')' );
		if ( ret_end < ret_beg )
		{
			qWarning( "QexMethodTranslator: Wrong slot signature %s", slot_name.constData() );
			return false;
		}

		ret_type = slot_name.mid( ret_beg, ret_end - ret_beg );
		if ( ret_type.contains( ',' ) )
		{
			qWarning( "QexMethodTranslator: The slot %s has many input values but must be one", slot_name.constData() );
			return false;
		}
	}

	QByteArray method_name = QMetaObject::normalizedSignature( a_method + 1 );
	QByteArray member_name = method_name;
	member_name.truncate( method_name.indexOf( '(' ) + 1 );
	QList< QByteArray > argument_types;

	QGenericArgument * arg [] = { &arg1, &arg2, &arg3, &arg4, &arg5
								, &arg6, &arg7, &arg8, &arg9, &arg10 };

	for ( int i = 0; i < 10; ++i )
	{
		const char * type_name = arg[ i ]->name();

		if ( type_name == 0 )
		{
			member_name += ')';
			break;
		}

		if ( i != 0 )
			member_name += ',';

		argument_types.append( type_name );
		member_name += type_name;
	}

	if ( member_name != method_name )
	{
		qWarning( "QexMethodTranslator: The method signature %s not correspond to inputed types"
			, method_name.constData() );
		return false;
	}

	QList< QByteArray > argument_list;
	for ( int i = 0; i < argument_types.count(); ++i )
	{
		QByteArray arr;
		QDataStream stream( &arr, QIODevice::WriteOnly );
		if ( !QMetaType::save( stream, QMetaType::type( argument_types.at( i ) ), arg[ i ]->data() ) )
		{
			qWarning( "QexMethodTranslator: The type %s has no stream operators registration"
				, argument_types.at( i ).constData() );
			return false;
		}
		argument_list.append( arr );
	}

	// определяем id запроса
	static qint32 id = 0;
	qint32 answer_id = need_answer ? ++id : 0;

	// запоминаем по id обработчик ответа (requester) и метод обработки ответа (a_slot).
	if ( need_answer )
	{
		QByteArray member = slot_name;
		member.truncate( slot_name.indexOf( '(' ) );
		p_data->requester_group_list.append( QexMethodRequestGroup( answer_id, requester, ret_type, member ) );
	}

	// пакуем метод
	// << id << key << method_name << ret_type << argument_types << argument_list;
	QByteArray data;
	QDataStream stream( &data, QIODevice::WriteOnly );
	stream << answer_id << key << method_name << ret_type << argument_types << argument_list;

	emit methodPacked( data );

	return true;
};

bool QexMethodTranslator::callMethod ( const QByteArray & key, const char * a_method, QVariant arg1
									  , QVariant arg2, QVariant arg3, QVariant arg4
									  , QVariant arg5, QVariant arg6, QVariant arg7
									  , QVariant arg8, QVariant arg9, QVariant arg10 )
{
	return callMethod( key, a_method
		, QGenericArgument( arg1.typeName(), arg1.data() )
		, QGenericArgument( arg2.typeName(), arg2.data() )
		, QGenericArgument( arg3.typeName(), arg3.data() )
		, QGenericArgument( arg4.typeName(), arg4.data() )
		, QGenericArgument( arg5.typeName(), arg5.data() )
		, QGenericArgument( arg6.typeName(), arg6.data() )
		, QGenericArgument( arg7.typeName(), arg7.data() )
		, QGenericArgument( arg8.typeName(), arg8.data() )
		, QGenericArgument( arg9.typeName(), arg9.data() )
		, QGenericArgument( arg10.typeName(), arg10.data() ) );
};

bool QexMethodTranslator::callMethod ( const QByteArray & key, const char * a_method, QGenericArgument arg1
									  , QGenericArgument arg2, QGenericArgument arg3, QGenericArgument arg4
									  , QGenericArgument arg5, QGenericArgument arg6, QGenericArgument arg7
									  , QGenericArgument arg8, QGenericArgument arg9, QGenericArgument arg10 )
{
	return callMethod( key, a_method, 0, 0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 ,arg10 );
};

bool QexMethodTranslator::connectMethod ( const QByteArray & key, QObject * respondent, const char * a_method )
{
	if ( !respondent || !a_method )
	{
		qWarning( "QexMethodTranslator: Incorrect inputs: respondent or method" );
		return false;
	}

	QByteArray method_name = QMetaObject::normalizedSignature( a_method + 1 );
	if ( respondent->metaObject()->indexOfMethod( method_name.constData() ) < 0 )
	{
		qWarning( "QexMethodTranslator: The respondent has no method %s", method_name.constData() );
		return false;
	}

	const QexKeyToMethodToRespondentMap const_map = p_data->key_method_respondent_map;
	QexKeyToMethodToRespondentMap::const_iterator key_iter = const_map.find( key );
	if ( key_iter != const_map.end() )
	{
		QexMethodToRespondentMap::const_iterator method_iter = ( *key_iter ).find( method_name );
		if ( method_iter != ( *key_iter ).end() )
		{
			if ( method_iter.value() == respondent )
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
	p_data->key_method_respondent_map[ key ][ method_name ] = respondent;
	return true;
};

int QexMethodTranslator::connectAllMethods ( const QByteArray & key, QObject * respondent )
{
	int ret = 0;
	const QMetaObject * mo = respondent->metaObject();
	for ( int i = 0; i < mo->methodCount(); ++i )
	{
		QByteArray method_type = mo->method(i).typeName();
		if ( !method_type.isEmpty() )
		{
			QByteArray arr;
			arr += '0';
			arr += mo->method(i).signature();
			connectMethod( key, respondent, arr.constData() );
			++ret;
		}
	}
	return ret;
};

void QexMethodTranslator::disconnectMethod ( const QByteArray & key, QObject * respondent, const char * a_method )
{
	QByteArray method_name = QMetaObject::normalizedSignature( a_method + 1 );

	const QexKeyToMethodToRespondentMap const_map = p_data->key_method_respondent_map;
	QexKeyToMethodToRespondentMap::const_iterator key_iter = const_map.find( key );
	if ( key_iter != const_map.end() )
	{
		QexMethodToRespondentMap::const_iterator method_iter = ( *key_iter ).find( method_name );
		if ( method_iter != ( *key_iter ).end() )
		{
			if ( method_iter.value() == respondent )
			{
				p_data->key_method_respondent_map[ key ].remove( method_name );
				if ( p_data->key_method_respondent_map[ key ].isEmpty() )
				{
					p_data->key_method_respondent_map.remove( key );
				}
				return;
			}
		}
	}
};

void QexMethodTranslator::disconnectMethods ( QObject * respondent, const char * a_method )
{
	QByteArray method_name = QMetaObject::normalizedSignature( a_method + 1 );

	const QexKeyToMethodToRespondentMap const_map = p_data->key_method_respondent_map;
	for ( QexKeyToMethodToRespondentMap::const_iterator key_iter = const_map.begin();
		key_iter != const_map.end(); ++key_iter )
	{
		const QByteArray & key = key_iter.key();
		QexMethodToRespondentMap::const_iterator method_iter = ( *key_iter ).find( method_name );
		if ( method_iter != ( *key_iter ).end() )
		{
			if ( method_iter.value() == respondent )
			{
				p_data->key_method_respondent_map[ key ].remove( method_name );
				if ( p_data->key_method_respondent_map[ key ].isEmpty() )
				{
					p_data->key_method_respondent_map.remove( key );
				}
			}
		}
	}
};

void QexMethodTranslator::disconnectMethods ( QObject * respondent )
{
	const QexKeyToMethodToRespondentMap const_map = p_data->key_method_respondent_map;
	for ( QexKeyToMethodToRespondentMap::const_iterator key_iter = const_map.begin();
		key_iter != const_map.end(); ++key_iter )
	{
		const QByteArray & key = key_iter.key();
		for ( QexMethodToRespondentMap::const_iterator method_iter = ( *key_iter ).begin();
			method_iter != ( *key_iter ).end(); ++method_iter )
		{
			const QByteArray & method_name = method_iter.key();
			if ( method_iter.value() == respondent )
			{
				p_data->key_method_respondent_map[ key ].remove( method_name );
				if ( p_data->key_method_respondent_map[ key ].isEmpty() )
				{
					p_data->key_method_respondent_map.remove( key );
				}
			}
		}
	}
};

void QexMethodTranslator::disconnectAllMethods ()
{
	p_data->key_method_respondent_map.clear();
};

void QexMethodTranslator::disconnectRequester ( QObject * requester, const char * a_slot )
{
	QByteArray slot_name = QMetaObject::normalizedSignature( a_slot + 1 );
	QByteArray member_name = slot_name;
	member_name.truncate( slot_name.indexOf( '(' ) );
	int ret_beg = slot_name.indexOf( '(' ) + 1;
	int ret_end = slot_name.lastIndexOf( ')' );
	QByteArray ret_type = slot_name.mid( ret_beg, ret_end - ret_beg );

	const QexMethodRequestGroupList temp_list = p_data->requester_group_list;
	for ( QexMethodRequestGroupList::const_iterator iter = temp_list.begin();
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

void QexMethodTranslator::disconnectRequester ( QObject * requester )
{
	const QexMethodRequestGroupList temp_list = p_data->requester_group_list;
	for ( QexMethodRequestGroupList::const_iterator iter = temp_list.begin();
		iter != temp_list.end(); ++iter )
	{
		if ( ( *iter ).requester == requester )
		{
			p_data->requester_group_list.removeOne( *iter );
		}
	}
};

void QexMethodTranslator::disconnectAllRequesters ()
{
	p_data->requester_group_list.clear();
};

void QexMethodTranslator::activateMethod ( const QByteArray & data )
{
	// частично распаковываем метод
	// >> id >> key >> method_name >> ret_type >> argument_types >> argument_list;
	int id;
	QByteArray key;
	QByteArray method_name;
	QByteArray answer_type;
	QByteArray answer;

	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::ReadOnly );
	stream >> id >> key >> method_name >> answer_type;

	// ищем подписчика
	const QexKeyToMethodToRespondentMap const_map = p_data->key_method_respondent_map;
	QexKeyToMethodToRespondentMap::const_iterator key_iter = const_map.find( key );
	if ( key_iter == const_map.end() )
	{
		answer_type = QByteArray();
		answer = QByteArray( "There is no method respondent with key " ) + key;
	}

	if ( answer.isEmpty() )
	{
		QexMethodToRespondentMap::const_iterator method_iter = ( *key_iter ).find( method_name );
		if ( method_iter == ( *key_iter ).end() || method_iter.value().isNull() )
		{
			answer_type = QByteArray();
			answer = QByteArray( "There is no respondent with method " ) + method_name;

			if ( method_iter != ( *key_iter ).end() )
				disconnectMethods( 0 );
		}

		if ( answer.isEmpty() )
		{
			QObject * respondent = method_iter.value();
			QexMethodInvoker * invoker = new QexMethodInvoker( respondent, data );
			invoker->moveToThread( respondent->thread() );

			Q_ASSERT( invoker->thread() == respondent->thread() );
			connect( invoker, SIGNAL( answerPacked( const QByteArray & ) )
				, this, SIGNAL( answerPacked( const QByteArray & ) ) );

			QMetaObject::invokeMethod ( invoker, "invoke" );
		}
	}

	if ( !answer.isEmpty() ) // если есть что вернуть
	{
		// пакуем и отправляем ответ
		QByteArray ans;
		QDataStream ans_stream( &ans, QIODevice::WriteOnly );
		ans_stream << id << key << method_name << answer_type << answer;
		emit answerPacked( ans );
	}
};

void QexMethodTranslator::activateAnswer ( const QByteArray & data )
{
	// распаковываем ответ
	int id;
	QByteArray key;
	QByteArray method_name;
	QByteArray answer_type;
	QByteArray answer;

	QDataStream stream( data );
	stream >> id >> key >> method_name >> answer_type >> answer;

	const QexMethodRequestGroupList temp_list = p_data->requester_group_list;
	QexMethodRequestGroupList::const_iterator iter = temp_list.begin();
	while ( iter != temp_list.end() )
	{
		if ( ( *iter ).group_id == id )
			break;
		++iter;
	}

	bool answer_error = !id || ( answer_type.isEmpty() && !answer.isEmpty() );
	if ( answer_error )
	{
		qWarning( "Call method error: %s", answer.constData() );
	}

	if ( iter != temp_list.end() )
	{
		QObject * requester = ( *iter ).requester;
		QByteArray slot_member = ( *iter ).slot_member;
		p_data->requester_group_list.removeOne( *iter );

		if ( requester && !answer_error )
		{
			if ( answer_type.isEmpty() )
			{
				QMetaObject::invokeMethod( requester, slot_member.constData() );
			}
			else
			{
				int type_id = QMetaType::type( answer_type.constData() );
				void * arg = QMetaType::construct( type_id );

				QDataStream arg_stream( &answer, QIODevice::ReadOnly );
				QMetaType::load( arg_stream, type_id, arg );

				QMetaObject::invokeMethod( requester, slot_member
					, QGenericArgument( answer_type.constData(), arg ) );
				QMetaType::destroy( type_id, arg );
			}
		}
	}
	else
	{
		qWarning( "QexMethodTranslator: There is no answer requester" );
	}
};
