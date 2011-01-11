#ifndef QEX_EXAMPLE_APPLICATION_H
#define QEX_EXAMPLE_APPLICATION_H

class SliderApplication : public QApplication
{
	Q_OBJECT
	Q_DISABLE_COPY( SliderApplication )

private:
	QTcpServer server;
	QTcpSocket client;

	QexDeviceHelper client_helper;
	QexMetaTranslator client_translator;

	QSlider * slider;

public:
	SliderApplication ( int & argc, char ** argv );

public slots:
	void newServerConection ();
};

#endif // QEX_EXAMPLE_APPLICATION_H
