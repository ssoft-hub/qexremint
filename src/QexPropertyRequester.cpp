#include "QexPropertyRequester.h"

////////////////////////////////////////////////////////////////////////////////

class QexPropertyRequesterPrivate
{
public:
	QObject * respondent;
	QByteArray data;
};

////////////////////////////////////////////////////////////////////////////////

QexPropertyRequester::QexPropertyRequester ( QObject * respondent, const QByteArray & data, QObject * parent )
	: QObject( parent )
	, p_data( new QexPropertyRequesterPrivate )
{
	p_data->respondent = respondent;
	p_data->data = data;
};

QexPropertyRequester::~QexPropertyRequester ()
{
	delete p_data;
};

void QexPropertyRequester::request ()
{
	// распаковываем
	// >> id >> key >> property_name >> property_type
	int id;
	QByteArray key;
	QByteArray property_name;
	QByteArray property_type;
	QByteArray answer;

	QDataStream stream( &p_data->data, QIODevice::ReadOnly );
	stream >> id >> key >> property_name >> property_type;

	int property_index = p_data->respondent->metaObject()->indexOfProperty( property_name.constData() );
	if ( property_index < 0 )
	{
		property_type = QByteArray();
		answer = "Respondent has no such property ";
		answer += property_name;
	}

	QMetaProperty meta_property = p_data->respondent->metaObject()->property( property_index );
	QByteArray type_name = meta_property.typeName();

	if ( type_name != property_type )
	{
		answer = "The answer and property types are different ( ";
		answer += property_type;
		answer += " and ";
		answer += property_type;
		answer += " )";
		property_type = QByteArray();
	}

	// если тип правильный, то запрос значения
	if ( !property_type.isEmpty() )
	{
		QVariant a = meta_property.read( p_data->respondent );
		if ( !a.isValid() )
		{
			property_type = QByteArray();
			answer = "Property read error";
		}
		else
		{
			QDataStream ans_stream( &answer, QIODevice::WriteOnly );
			QMetaType::save( ans_stream, QMetaType::type( a.typeName() ), a.data() );
		}
	}

	// пакуем и отправляем ответ
	QByteArray ans;
	QDataStream ans_stream( &ans, QIODevice::WriteOnly );
	ans_stream << id << key << property_name << property_type << answer;
	emit answerPacked( ans );
	deleteLater();
};
