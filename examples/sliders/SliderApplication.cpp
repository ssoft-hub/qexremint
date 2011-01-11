#include "SliderApplication.h"

SliderApplication::SliderApplication ( int & argc, char ** argv )
	: QApplication( argc, argv )
{
	slider = new QSlider( Qt::Horizontal );
	slider->show();

	connect( &server, SIGNAL( newConnection () )
		, this, SLOT( newServerConection() ) );

	if ( !server.listen( QHostAddress::LocalHost, 23456 ) )
	{
		client.connectToHost( "localhost", 23456 );

		connect( &client, SIGNAL( disconnected() )
			, &client_helper, SLOT( reset() ) );

		connect( &client_helper, SIGNAL( inputted( const QByteArray & ) )
			, &client_translator, SLOT( inputData( const QByteArray & ) ) );

		connect( &client_translator, SIGNAL( dataOutputted( const QByteArray & ) )
			, &client_helper, SLOT( output( const QByteArray & ) ) );

		// Interprocess signal slot
		client_helper.setDevice( &client );
		client_translator.signalSlotTranslator()
			.connectSlot( "Slider", SIGNAL( valueChanged ( int ) )
				, slider, SLOT( setValue( int ) ) );
	}
};

void SliderApplication::newServerConection ()
{
	while ( server.hasPendingConnections() )
	{
		QTcpSocket * socket = server.nextPendingConnection();

		QexDeviceHelper * helper = new QexDeviceHelper( socket );
		QexMetaTranslator * translator = new QexMetaTranslator( socket );

		connect( helper, SIGNAL( inputted( const QByteArray & ) )
			, translator, SLOT( inputData( const QByteArray & ) ) );

		connect( translator, SIGNAL( dataOutputted( const QByteArray & ) )
			, helper, SLOT( output( const QByteArray & ) ) );

		// Interprocess signal slot
		helper->setDevice( socket );
		translator->signalSlotTranslator()
			.connectSignal( "Slider", slider, SIGNAL( valueChanged ( int ) ) );
		translator->signalSlotTranslator()
			.connectSlot( "Slider", SIGNAL( valueChanged ( int ) )
				, slider, SLOT( setValue( int ) ) );
	}
};
