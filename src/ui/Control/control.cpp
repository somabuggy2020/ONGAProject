#include "control.h"
#include "ui_control.h"
//#include "csvread.h"
//#include "histcalc.h"

Control::Control(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Control)
{
	ui->setupUi(this);
	//	setWindowTitle("Save_Image");

	ui->dividednum->setValue(7);
	width_divied = ui->dividednum->value();
	ui->datacount->setValue(0);
	ui->gridms->setChecked(false);
	ui->allms->setChecked(true);
	allbool = ui->allms->isChecked();
	ui->depth->setValue(3.0);
	tvalue = 0.0;
	//	inclination = 1.0;
	//	intercept = 0.0;
	logDepthDataPath = "NULL";
}

void Control::init()
{
	emit setdivnum(ui->dividednum->value());
	emit clickradiobotton(allbool);
	emit set_depth(ui->depth->value());
}

Control::~Control()		//destructor
{
	delete ui;
}

void Control::on_rgbsave_clicked()
{
	emit save_rgbimage();
}

void Control::on_dividednum_valueChanged(int arg1)
{
	width_divied = arg1;
	emit setdivnum(arg1);
}

void Control::countup()
{
	ui->datacount->setValue(ui->datacount->value() + 1);
}

void Control::clearcount()
{
	ui->datacount->setValue(0);
}

void Control::on_gridms_clicked(bool checked)
{
	ui->allms->setChecked(!checked);
	allbool = false;
	emit clickradiobotton(allbool);
}

void Control::on_allms_clicked(bool checked)
{
	ui->gridms->setChecked(!checked);
	allbool = true;
	emit clickradiobotton(allbool);
}

/*void Control::on_set_clicked() 0828
{
	QVector<int> gridindex;
	QString gridtext;
	QStringList split_text;
	QStringList split_text2;//,あと-用
	QString gridlist;//最終的なList 数字と-にする
	split_text.clear();
	split_text2.clear();
	gridlist.clear();
	gridindex.clear();

	gridtext = ui->gridnum->text();

	split_text = gridtext.split(",");

	bool num;
	for(int i = 0;i < split_text.size(); i++){
		QString numtext = split_text[i];
		qDebug()<<numtext;
		int search = numtext.indexOf("-");
		if(search == -1 && numtext != ""){
			int grid = numtext.toInt(&num);
			if(num)gridindex.push_back(grid);
		}
		else{
			split_text2 = numtext.split("-");
			if(split_text2.size() >= 2 ){
				QString num1 = split_text2[0];
				QString num2 = split_text2[1];
				int first = num1.toInt(&num);
				int second;
				if(num)second = num2.toInt(&num);
				if(num)if(first < second)for(int j = first;j <= second; j++)gridindex.push_back(j);
			}
		}
	}

	std::vector<int> index = gridindex.toStdVector();
	std::sort(index.begin(), index.end());

	gridindex.clear();
	gridindex = QVector<int>::fromStdVector(index);

	emit setgridnum(gridindex);
}*/

/* これは，R200が垂直に認識できないと考えていた時の，補正関数の式．
void Control::set_property(double a0, double a1)
{
	inclination = 1.0;
	intercept = 0;
	//cout << "inclination =" <<inclination << endl;  //inclination:傾き
	//cout << "intercept = " << intercept << endl;		//intercept:切片
	savePath = Control::getPath("LogCalibrationCurve");
	ofstream ofs(savePath.toStdString());
	ofs << "inclination"<< "," << "intercept" << endl;
	ofs <<  inclination << "," <<  intercept  << endl;
}*/

void Control::setLogDepthDatapath(QString lDDP){
	logDepthDataPath = lDDP;

	qDebug() << "set_lDDP: " << logDepthDataPath;
}

void Control::on_depth_valueChanged(double arg1)
{
	emit set_depth(arg1);
}

void Control::on_finish_clicked()
{
	cout << "---------------------------histgram is maked---------------------------" << endl;
	string fileNam = logDepthDataPath.toStdString();
	cout << "logDepthDataPath:" << fileNam << endl;

	//	CsvRead csv1(width_divied);
	//	csv1.setData(fileNam);

	//	cout << "row number : " << csv1.getRowIndex() << endl;
	//	cout << "col number : " << csv1.getColIndex() << endl;

	//	HistCalc hist1(csv1.getRowIndex(),csv1.getColIndex(),csv1.getData());
	//	hist1.calc_hist();

	//書き込みテスト用：保存するパスを指定
	savePath = Control::getPath("ResultDepthData");
	ofstream ofs(savePath.toStdString());
	cout << "savePath:" << savePath.toStdString() << endl;

	ofs << "width_divied," << width_divied << endl;
	ofs << "height," << width_divied << endl;
	ofs << "grid_No" << "," << "average" << endl;

	/*
	for(int n=0;n<csv1.getColIndex();n++){
		depth_average = hist1.get_hist_average(n);
		if(depth_average == -1){
			if(csv1.getRowIndex() < 1000){
				emit remeasure();		//再計測の呼び出し

				cout << "average" << "["<<n << "]: "<< depth_average << endl;
				//     このａｖｅｒａｇｅを補正の関数に渡して，推定される真値を得る
				//ofs << n <<","<< depth_average << "," << inclination*double(depth_average)+intercept << endl;
				ofs << n <<","<< depth_average << endl;
			}else{

				//ｄｅｂｕｇ用
				cout << "average" << "["<<n << "]: "<< depth_average << endl;
				ofs << n <<","<< depth_average << endl;
				emit PCVopen();		//pointcloudviewerの再起動.
			}
		}else{
			//ｄｅｂｕｇ用
			cout << "average" << "["<<n << "]: "<< depth_average << endl;
			ofs << n <<","<< depth_average << endl;
			emit PCVopen();		//pointcloudviewerの再起動.
		}
	} //end for
	*/
}

void Control::Getaverage(QString file){
	double average;
	string fileNam = file.toStdString();
	//	CsvRead csv1(1); //引数は，読み込むデータのcolumn数なので，中心のグリッドのみ＝>1
	//	csv1.setData(fileNam);
	cout << "fileNam:" << fileNam << endl;
	//	cout << "row number : " << csv1.getRowIndex() << endl;
	//	cout << "col number : " << csv1.getColIndex() << endl;

	//	HistCalc hist1(csv1.getRowIndex(),csv1.getColIndex(),csv1.getData());
	//	hist1.calc_hist();

	//	average =  hist1.get_hist_average(0); //計算する平均の値は１つ

	//	cout << "average : " << average << endl;

	emit plotdata(average, ui->depth->value());
}

QString Control::getPath(QString dirnam)
{
	//build directoryまでの絶対パスを取得し，取得したパスから目的のdirectoryまでを操作する．
	buildDir = QDir::current();
	orgDirPath = buildDir.path();
	currentDateTime = QDateTime::currentDateTime();

	if(!QDir(orgDirPath + QString("/") + dirnam ).exists())
		buildDir.mkdir(dirnam);
	buildDir.cd(dirnam);

	QString date = currentDateTime.toString("yyyyMMdd");;

	if(!QDir(buildDir.path() + QString("/") + date).exists())
		buildDir.mkdir(date);
	buildDir.cd(date);

	QString time = currentDateTime.toString("hhmm");
	buildDir.mkdir(time);
	buildDir.cd(time);

	return buildDir.path()+ QString("/") + time + QString(".csv"); //　　”/buildDir/dirnam/date/time/time.csv” のパスを返す
}
