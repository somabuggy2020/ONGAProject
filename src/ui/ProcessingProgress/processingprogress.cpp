#include "processingprogress.h"
#include "ui_processingprogress.h"

/*!
 * \brief processingProgress::processingProgress
 * \param windowTitle
 * \param parent
 */
ProcessingProgress::ProcessingProgress(QString windowTitle,QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ProcessingProgress)
{
	ui->setupUi(this);
	this->setWindowTitle(windowTitle);
	//	this->setFixedSize(339,240);
}

/*!
 * \brief processingProgress::~processingProgress
 */
ProcessingProgress::~ProcessingProgress()
{
	delete ui;
}

void ProcessingProgress::on_btnStart_clicked()
{
	emit start();
}

/*!
 * \brief processingProgress::showData
 * \param showData
 */
void ProcessingProgress::showData(QString showData)
{
	/*ui->textEdit->append(showData);
	ui->lblConnect->setStyleSheet("background:#00ff00;");*/		//0407ezaki
	//qDebug() << "show data" << showData;
}

/*!
 * \brief processingProgress::on_stopButton_clicked
 * stop and save button がクリックされたとき
 */
void ProcessingProgress::on_stopButton_clicked()
{
	emit sendStop();
}

/*!
 * \brief processingProgress::on_clearButton_clicked
 * data clearがクリックされたとき
 */
void ProcessingProgress::on_clearButton_clicked()
{
	emit dataclear();
}

//計測を開始するボタンがクリックされたとき
//void ProcessingProgress::on_startButton_clicked()
//{
//	emit start();
//}

void ProcessingProgress::showDicision(int data)
{
	//0407ezaki　竹上君のプログラム．プログラムが上手く行ったら最後に消す．
}



void ProcessingProgress::on_calcButton_clicked()
{
	//jumpRai->show();
}


