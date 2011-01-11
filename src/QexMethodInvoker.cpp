#include "QexMethodInvoker.h"

////////////////////////////////////////////////////////////////////////////////

class QexMethodInvokerPrivate
{
public:
	QObject * respondent;
	QByteArray data;
};

////////////////////////////////////////////////////////////////////////////////

QexMethodInvoker::QexMethodInvoker ( QObject * respondent, const QByteArray & data, QObject * parent )
	: QObject( parent )
	, p_data( new QexMethodInvokerPrivate )
{
	p_data->respondent = respondent;
	p_data->data = data;
};

QexMethodInvoker::~QexMethodInvoker ()
{
	delete p_data;
};

void QexMethodInvoker::invoke ()
{
	// распаковываем метод
	// >> id >> key >> method_name >> ret_type >> argument_types >> argument_list;
	int id;
	QByteArray key;
	QByteArray method_name;
	QByteArray answer_type;
	QList< QByteArray > argument_types;
	QList< QByteArray > argument_list;
	QByteArray answer;

	QDataStream stream( &p_data->data, QIODevice::ReadOnly );
	stream >> id >> key >> method_name >> answer_type;

	int method_index = p_data->respondent->metaObject()->indexOfMethod( method_name.constData() );
	if ( method_index < 0 )
	{
		answer_type = QByteArray();
		answer = "Respondent has no such method ";
		answer += method_name;
	}

	QByteArray method_type = p_data->respondent->metaObject()->method( method_index ).typeName();

	if ( ( id != 0 ) && ( !answer_type.isEmpty() ) && ( method_type != answer_type ) )
	{
		answer = "The answer and method types are different ( ";
		answer += answer_type;
		answer += " and ";
		answer += answer_type;
		answer += " )";
		answer_type = QByteArray();
	}

	// если не выявлено ошибок, то активация метода
	if ( answer.isEmpty() )
	{
		stream  >> argument_types >> argument_list;

		int ret_type_id = ( id == 0 || answer_type.isEmpty() ) ? 0 : QMetaType::type( answer_type.constData() );
		void * ret_arg = ( id == 0 || answer_type.isEmpty() ) ? 0 : QMetaType::construct( ret_type_id );

		QGenericArgument ga[ 10 ];
		QList< void * > arguments;

		int i;
		for ( i = 0; i < argument_types.count(); ++i )
		{
			int type_id = QMetaType::type( argument_types.at( i ).constData() );
			if ( type_id == 0 ) break;
			void * arg = QMetaType::construct( type_id );
			arguments.append( arg );

			QByteArray arg_arr = argument_list.at( i );
			QDataStream arg_stream( &arg_arr, QIODevice::ReadOnly );
			QMetaType::load( arg_stream, QMetaType::type( argument_types.at( i ).constData() ), arg );

			ga[i] = QGenericArgument( argument_types.at( i ).constData(), arg );
		}

		if ( i == argument_types.count() )
		{
			QByteArray member = method_name;
			bool res;
			member.truncate( method_name.indexOf( '(' ) );
			if ( id == 0 || answer_type.isEmpty()  )
			{
				res = QMetaObject::invokeMethod( p_data->respondent, member
						, ga[0], ga[1], ga[2], ga[3], ga[4], ga[5], ga[6], ga[7], ga[8], ga[9] );
			}
			else
			{
				res = QMetaObject::invokeMethod( p_data->respondent, member
						, QGenericReturnArgument( answer_type.constData(), ret_arg )
						, ga[0], ga[1], ga[2], ga[3], ga[4], ga[5], ga[6], ga[7], ga[8], ga[9] );
			}

			if( !res )
			{
				answer_type = QByteArray();
				answer = "False invoke method";
			}
		}

		for ( i = 0; i < arguments.count(); ++i )
		{
			int type_id = QMetaType::type( argument_types.at( i ).constData() );
			QMetaType::destroy( type_id, arguments.at( i ) );
		}

		if ( id != 0 && answer.isEmpty() )
		{
			QDataStream ans_stream( &answer, QIODevice::WriteOnly );
			QMetaType::save( ans_stream, ret_type_id, ret_arg );
		}
		if ( ret_arg ) QMetaType::destroy( ret_type_id, ret_arg );
	}

	if ( id != 0 ) // если требуется ответ
	{
		// пакуем и отправляем ответ
		QByteArray ans;
		QDataStream ans_stream( &ans, QIODevice::WriteOnly );
		ans_stream << id << key << method_name << answer_type << answer;
		emit answerPacked( ans );
	}
	deleteLater();
};
