#include "QexMetaTranslator.h"

////////////////////////////////////////////////////////////////////////////////

class QexMetaTranslatorPrivate
{
public:
	QexSignalSlotTranslator signal_slot_translator;
	QexMethodTranslator method_translator;
	QexPropertyTranslator property_translator;
};

////////////////////////////////////////////////////////////////////////////////

QexMetaTranslator::QexMetaTranslator ( QObject * parent )
	: QObject( parent )
	, p_data( new QexMetaTranslatorPrivate )
{
	connect( &( p_data->signal_slot_translator ), SIGNAL( signalPacked( const QByteArray & ) )
		, this, SLOT( outputSignal( const QByteArray & ) ) );
	connect( &( p_data->method_translator ), SIGNAL( methodPacked( const QByteArray & ) )
		, this, SLOT( outputMethod( const QByteArray & ) ) );
	connect( &( p_data->method_translator ), SIGNAL( answerPacked( const QByteArray & ) )
		, this, SLOT( outputMethodAnswer( const QByteArray & ) ) );
	connect( &( p_data->property_translator ), SIGNAL( getPropertyPacked( const QByteArray & ) )
		, this, SLOT( outputGetProperty( const QByteArray & ) ) );
	connect( &( p_data->property_translator ), SIGNAL( setPropertyPacked( const QByteArray & ) )
		, this, SLOT( outputSetProperty( const QByteArray & ) ) );
	connect( &( p_data->property_translator), SIGNAL( answerPacked( const QByteArray & ) )
		, this, SLOT( outputGetPropertyAnswer( const QByteArray & ) ) );
};

QexMetaTranslator::~QexMetaTranslator ()
{
	delete p_data;
};

bool QexMetaTranslator::callMethod ( const QByteArray & key, const char * a_method
									, QObject * requester, const char * a_slot, QVariant arg1
									, QVariant arg2, QVariant arg3, QVariant arg4
									, QVariant arg5, QVariant arg6, QVariant arg7
									, QVariant arg8, QVariant arg9, QVariant arg10 )
{
	return p_data->method_translator.callMethod( key, a_method, requester, a_slot
		, arg1 ,arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
};

bool QexMetaTranslator::callMethod ( const QByteArray & key, const char * a_method
									, QObject * requester, const char * a_slot, QGenericArgument arg1
									, QGenericArgument arg2, QGenericArgument arg3, QGenericArgument arg4
									, QGenericArgument arg5, QGenericArgument arg6, QGenericArgument arg7
									, QGenericArgument arg8, QGenericArgument arg9, QGenericArgument arg10 )
{
	return p_data->method_translator.callMethod( key, a_method, requester, a_slot
		, arg1 ,arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 );
};


bool QexMetaTranslator::getProperty ( const QByteArray & key, const char * a_property
									 , QObject * requester, const char * a_slot )
{
	return p_data->property_translator.getProperty( key, a_property, requester, a_slot );
};

bool QexMetaTranslator::setProperty ( const QByteArray & key, const char * a_property, QVariant value )
{
	return p_data->property_translator.setProperty( key, a_property, value);
};

bool QexMetaTranslator::setProperty ( const QByteArray & key, const char * a_property, QGenericArgument arg )
{
	return p_data->property_translator.setProperty( key, a_property, arg );
};

QexMethodTranslator & QexMetaTranslator::methodTranslator ()
{
	return p_data->method_translator;
};

QexPropertyTranslator & QexMetaTranslator::propertyTranslator ()
{
	return p_data->property_translator;
};

QexSignalSlotTranslator & QexMetaTranslator::signalSlotTranslator ()
{
	return p_data->signal_slot_translator;
};

enum MagicMetaAction_tag
{
	ActivateSlot                 = 0x45853498
	, ActivateMethod             = 0x34987894
	, ActivateMethodAnswer       = 0x09598433
	, ActivateGetProperty        = 0x90807144
	, ActivateSetProperty        = 0x20759604
	, ActivateGetPropertyAnswer  = 0x92197504
};

void QexMetaTranslator::inputData ( const QByteArray & data )
{
	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::ReadOnly );
	int action_id;
	QByteArray data_value;
	stream >> action_id >> data_value;

	switch ( action_id )
	{
	case ActivateSlot:
		p_data->signal_slot_translator.activateSlot( data_value );
		break;
	case ActivateMethod:
		p_data->method_translator.activateMethod( data_value );
		break;
	case ActivateMethodAnswer:
		p_data->method_translator.activateAnswer( data_value );
		break;
	case ActivateGetProperty:
		p_data->property_translator.activateGetProperty( data_value );
		break;
	case ActivateSetProperty:
		p_data->property_translator.activateSetProperty( data_value );
		break;
	case ActivateGetPropertyAnswer:
		p_data->property_translator.activateAnswer( data_value );
		break;
	default:
		emit actionError();
	}
};

void QexMetaTranslator::outputSignal ( const QByteArray & data )
{
	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::WriteOnly );
	stream << ( int ) ActivateSlot << data;
	dataOutputted( arr );
};

void QexMetaTranslator::outputMethod ( const QByteArray & data )
{
	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::WriteOnly );
	stream << ( int ) ActivateMethod << data;
	dataOutputted( arr );
};

void QexMetaTranslator::outputMethodAnswer ( const QByteArray & data )
{
	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::WriteOnly );
	stream << ( int ) ActivateMethodAnswer << data;
	dataOutputted( arr );
};

void QexMetaTranslator::outputGetProperty ( const QByteArray & data )
{
	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::WriteOnly );
	stream << ( int ) ActivateGetProperty << data;
	dataOutputted( arr );
};

void QexMetaTranslator::outputSetProperty ( const QByteArray & data )
{
	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::WriteOnly );
	stream << ( int ) ActivateSetProperty << data;
	dataOutputted( arr );
};

void QexMetaTranslator::outputGetPropertyAnswer ( const QByteArray & data )
{
	QByteArray arr = data;
	QDataStream stream( &arr, QIODevice::WriteOnly );
	stream << ( int ) ActivateGetPropertyAnswer << data;
	dataOutputted( arr );
};

