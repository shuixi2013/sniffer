// =====================================================================================
// 
//       Filename:  mainwindow.cpp
//
//    Description:  主窗口的类实现文件
//
//        Version:  1.0
//        Created:  2013年01月20日 17时06分16秒
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================

#include <QtGui>

#include "../include/mainwindow.h"
#include "../include/listtreeview.h"
#include "../include/prototreeview.h"
#include "../include/capturethread.h"
#include "../include/choosedevdialog.h"
#include "../include/findqqdialog.h"
#include "../include/sniffer.h"

MainWindow::MainWindow()		
{
	captureThread = NULL;

	sniffer = new Sniffer;
	if (sniffer->getNetDevInfo() == false) {
		QMessageBox::warning(this, tr("Sniffer"),
						tr("<H3>无法在您的机器上获取网络适配器接口。</H3>"
							"<p>很遗憾出现这样的结果，可能出现的原因有：\n"
							"<p>1. 不支持您的网卡，请到 <a href=\"http://winpcap.org\">"
							"http://winpcap.org</a> 查阅支持的硬件列表\n"
							"<p>2. 您的杀毒软件或者HIPS程序阻止本程序运行"), QMessageBox::Ok);
	}

	settingInfo = new SettingInfo;

	createMainWeiget();
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	readSettings();

	setWindowIcon(QIcon(":/res/ico/main.png"));
	setWindowTitle(tr("Sniffer -- 测试版"));
}

MainWindow::~MainWindow()		
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
	int res = this->isToContinue();
	if (res == QMessageBox::Yes) {
		save();
		writeSettings();
		event->accept();
	} else if (res == QMessageBox::No){
		writeSettings();
		event->accept();
	} else if (res == QMessageBox::Cancel){
		event->ignore();
	}
}

void MainWindow::newFile()
{
	if (isToContinue() == QMessageBox::No) {
		mainTreeView->rebuildInfo();
		setCurrentFile("");
	} else if (isToContinue() == QMessageBox::Yes) {
		save();
	}
}
	
void MainWindow::open()
{
	if (isToContinue() == QMessageBox::No) {
		mainTreeView->rebuildInfo();
		QString fileName = QFileDialog::getOpenFileName(this,
							tr("打开"), ".",
							tr("Sniffer 捕获数据文件 (*.sni)"));
		
		if (!fileName.isEmpty()) {
			if (loadFile(fileName) == false) {
				QMessageBox::warning(this, tr("Sniffer"),
						tr("<H3>文件打开失败。</H3>很遗憾遇到这样的事情，"
									"很可能文件正在被其它程序使用。"), QMessageBox::Ok);
			}
		}
	} else if (isToContinue() == QMessageBox::Yes) {
		save();
	}
}

void MainWindow::save()
{
	QString fileName = QFileDialog::getSaveFileName(this,
							tr("另存为 ..."), ".",
							tr("Sniffer 捕获数据文件 (*.sni)"));

	if (!fileName.isEmpty()) {
		saveFile(fileName);
	}
}

void MainWindow::print()
{
	QPrintDialog printDialog;
	if (printDialog.exec() == QDialog::Accepted) {
		// To Do ...
	}
}

void MainWindow::chooseDev()
{
	ChooseDevDialog chooseDevDialog(sniffer, this);

	if (chooseDevDialog.exec() == QDialog::Accepted) {
		chooseDevDialog.GetUserSet(settingInfo);
		if (settingInfo->iOpenDevNum > 0) {
			if (settingInfo->bAutoBegin == true) {
				begin();
			} else {
				beginAction->setEnabled(true);
				findQQAction->setEnabled(true);
			}
		}
	}
}

void MainWindow::begin()
{
	if (mainTreeView->isChanged()) {
		int result = QMessageBox::warning(this, tr("Sniffer"),
						tr("<H3>看起来我们似乎已经捕获到了一些数据。</H3>"
							"您确定放弃这些数据开始新的捕获吗？"),
						QMessageBox::Yes | QMessageBox::No);
		
		if (result == QMessageBox::No) {
			return;
		} else {
			mainTreeView->rebuildInfo();
		}
	}

	if (captureThread != NULL) {
		delete captureThread;
	}

	QDateTime nowTime   = QDateTime::currentDateTime();
	QString tmpFileName = QDir::tempPath() + "/sniffer~" + nowTime.toString("yyyy-MM-dd~hh-mm-ss") + ".tmp";

	setCurrentFile(tmpFileName);

	captureThread = new CaptureThread(mainTreeView, sniffer, tmpFileName);

	bool bOpenSucceed = false;
	if (settingInfo->bPromiscuous == true) {
		bOpenSucceed = sniffer->openNetDev(settingInfo->iOpenDevNum,
										PCAP_OPENFLAG_PROMISCUOUS, settingInfo->iDataLimit);
	} else {
		bOpenSucceed = sniffer->openNetDev(settingInfo->iOpenDevNum,
										 PCAP_OPENFLAG_NOCAPTURE_LOCAL, settingInfo->iDataLimit);
	}

	if (bOpenSucceed == true) {
		sniffer->setDevsFilter(settingInfo->filterString.c_str());
		captureThread->start();

		chooseDevAction->setEnabled(false);
		beginAction->setEnabled(false);
		findQQAction->setEnabled(false);
		endAction->setEnabled(true);
		
	} else {
		QMessageBox::warning(this, tr("Sniffer"),
						tr("<H3>无法在您的机器上打开网络适配器接口。</H3>"
							"<p>很遗憾出现这样的结果，可能出现的原因有：\n"
							"<p>1. 不支持您的网卡，请到 <a href=\"http://winpcap.org\">"
							"http://winpcap.org</a> 查阅支持的硬件列表\n"
							"<p>2. 您的杀毒软件或者HIPS程序阻止本程序运行"), QMessageBox::Ok);
	}
}

void MainWindow::end()
{
	chooseDevAction->setEnabled(true);
	beginAction->setEnabled(true);
	findQQAction->setEnabled(true);
	
	endAction->setEnabled(false);

	captureThread->stop();

	if (mainTreeView->isChanged()) {
		saveAction->setEnabled(true);
	}
}

void MainWindow::findQQ()
{
	FindQQDialog findQQDialog(this, this, sniffer);

	findQQDialog.exec();
}

void MainWindow::readSettings()
{
	QSettings settings("Sniffer.ini", QSettings::IniFormat);

	settingInfo->iOpenDevNum  = settings.value("iOpenDevNum", true).toInt();
	settingInfo->bPromiscuous = settings.value("bPromiscuous", true).toBool();
	settingInfo->bAutoBegin   = settings.value("bAutoBegin", true).toBool();
}

void MainWindow::writeSettings()
{
	QSettings settings("Sniffer.ini", QSettings::IniFormat);

	settings.setValue("iOpenDevNum", settingInfo->iOpenDevNum);
	settings.setValue("bPromiscuous", settingInfo->bPromiscuous);
	settings.setValue("bAutoBegin", settingInfo->bAutoBegin);
}

bool MainWindow::loadFile(const QString &fileName)
{
#ifdef WIN32

	if (sniffer->OpenSaveCaptureFile((const char *)fileName.toLocal8Bit()) == true) {
		if (captureThread != NULL) {
			delete captureThread;
		}

		captureThread = new CaptureThread(mainTreeView, sniffer);

		captureThread->start();

		setCurrentFile(fileName);
		statusBar()->showMessage(tr("File loaded"), 2000);
		
		return true;
	}
	
#endif
	return false;
}

bool MainWindow::saveFile(const QString &fileName)
{
	if (curFile.isEmpty()) {
		return false;
	}
	
	if(!QFile::copy(curFile, fileName)) {
		QMessageBox::warning(this, tr("Sniffer"),
						tr("<H3>文件保存失败。</H3>很遗憾遇到这样的事情，"
									"您是否选择了一个没有权限覆盖的文件？"), QMessageBox::Ok);
		return false;
	}

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File saved"), 2000);

	return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	if (!fileName.isEmpty()) {
		curFile = fileName;
		statusLabel->setText(fileName);
	} else {
		saveAction->setEnabled(false);
		statusLabel->setText("Sniffer -- 测试版");
	}
}

void MainWindow::addDataToWidget(const QItemSelection &nowSelect)
{
	QModelIndexList items = nowSelect.indexes();
	QModelIndex 	index = items.first();

	QString strNumber;
	mainTreeView->getOrderNumber(index, strNumber);

	int iNumber = strNumber.toInt();

	if ((unsigned int)iNumber <= sniffer->snifferDataVector.size()) {	
		explainTreeView->ShowTreeAnalyseInfo(&(sniffer->snifferDataVector.at(iNumber-1)));
		explainEdit->setText(sniffer->snifferDataVector.at(iNumber-1).protoInfo.strSendInfo);
		originalEdit->setText(sniffer->snifferDataVector.at(iNumber-1).strData);
	} 
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("关于 Sniffer"),
			tr("<h2>Sniffer 测试版</h2>"
				"<p>Copyright (C) 2013 Class 1107 of Computer Science and Technology"
				"<p>一个跨平台的网络数据嗅探&抓包程序，基于Qt 4.x 以及 libpcap 库"
				"（Linux下）和 Winpcap库（Windows 下）。"));
}

int MainWindow::isToContinue()
{
	if (mainTreeView->isChanged()) {
		return QMessageBox::question(NULL, tr("Sniffer"),
						tr("<H3>看起来我们似乎已经捕获到了一些数据。</H3>"
							"您需要保存这些捕获的数据供以后分析使用吗？"),
						QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	}
	return QMessageBox::No;
}

void MainWindow::sleep(unsigned int msec)
{
	QTime dieTime = QTime::currentTime().addMSecs(msec);

	while( QTime::currentTime() < dieTime ) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}
}

void MainWindow::createMainWeiget()
{
	mainTreeView    = new ListTreeView;
	explainTreeView = new ProtoTreeView;
	explainEdit		= new QTextEdit;
	originalEdit    = new QTextEdit;

	explainEdit->setReadOnly(true);
	explainEdit->setCurrentFont(QFont("宋体", 10));

	originalEdit->setReadOnly(true);
	originalEdit->setCurrentFont(QFont("宋体", 12));

	rightSplitter = new QSplitter(Qt::Vertical);

	rightSplitter->addWidget(explainTreeView);
	rightSplitter->addWidget(explainEdit);
	rightSplitter->addWidget(originalEdit);

	mainSplitter = new QSplitter(Qt::Horizontal);

	mainSplitter->addWidget(mainTreeView);
	mainSplitter->addWidget(rightSplitter);

	this->setCentralWidget(mainSplitter);

	QObject::connect(mainTreeView->selectionModel(),
						 SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
								 this, SLOT(addDataToWidget(const QItemSelection &)));
}

void MainWindow::createActions()
{
	newAction = new QAction(tr("新建"), this);
	newAction->setIcon(QIcon(":/res/images/new.png"));
	newAction->setShortcut(QKeySequence::New);
	newAction->setStatusTip(tr("创建一个新的捕获（清空历史）"));
	connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

	openAction = new QAction(tr("打开"), this);
	openAction->setIcon(QIcon(":/res/images/open.png"));
	openAction->setShortcut(QKeySequence::Open);
	openAction->setStatusTip(tr("打开历史的一个捕获记录"));
	connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

	saveAction = new QAction(tr("保存"), this);
	saveAction->setIcon(QIcon(":/res/images/save.png"));
	saveAction->setShortcut(QKeySequence::Save);
	saveAction->setStatusTip(tr("保存本次捕获信息到文件"));
	saveAction->setEnabled(false);
	connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

	printAction = new QAction(tr("打印"), this);
	printAction->setIcon(QIcon(":/res/images/print.png"));
	printAction->setShortcut(tr("Ctrl+P"));
	printAction->setStatusTip(tr("打印当前捕获的数据"));
	connect(printAction, SIGNAL(triggered()), this, SLOT(print()));

	exitAction = new QAction(tr("退出"), this);
	exitAction->setShortcut(tr("Ctrl+Q"));
	exitAction->setStatusTip(tr("退出程序"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

	chooseDevAction = new QAction(tr("选择捕获网卡"), this);
	chooseDevAction->setIcon(QIcon(":/res/images/corporation.png"));
	chooseDevAction->setShortcut(tr("Ctrl+N"));
	chooseDevAction->setStatusTip(tr("选择要捕获的网卡"));
	connect(chooseDevAction, SIGNAL(triggered()), this, SLOT(chooseDev()));

	beginAction = new QAction(tr("开始捕获"), this);
	beginAction->setIcon(QIcon(":/res/images/begin.png"));
	beginAction->setShortcut(tr("Ctrl+B"));
	beginAction->setStatusTip(tr("开始捕获数据包"));
	beginAction->setEnabled(false);
	connect(beginAction, SIGNAL(triggered()), this, SLOT(begin()));

	endAction = new QAction(tr("停止捕获"), this);
	endAction->setIcon(QIcon(":/res/images/end.png"));
	endAction->setShortcut(tr("Ctrl+E"));
	endAction->setStatusTip(tr("停止捕获数据包"));
	endAction->setEnabled(false);
	connect(endAction, SIGNAL(triggered()), this, SLOT(end()));

	findQQAction = new QAction(tr("捕获QQ号码"), this);
	findQQAction->setIcon(QIcon(":/res/images/findqq.png"));
	findQQAction->setStatusTip(tr("捕获数据包包含的QQ号码"));
	findQQAction->setEnabled(false);
	connect(findQQAction, SIGNAL(triggered()), this, SLOT(findQQ()));

	aboutAction = new QAction(tr("关于"), this);
	aboutAction->setIcon(QIcon(":/res/images/about.png"));
	aboutAction->setStatusTip(tr("关于信息"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAction = new QAction(tr("关于Qt"), this);
	aboutQtAction->setIcon(QIcon(":/res/images/aboutqt.png"));
	aboutQtAction->setStatusTip(tr("关于Qt信息"));
	connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
	fileMenu = this->menuBar()->addMenu(tr("文件"));
	fileMenu->addAction(newAction);
	fileMenu->addAction(openAction);
	fileMenu->addAction(saveAction);

	fileMenu->addSeparator();
	fileMenu->addAction(printAction);

	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	toolsMenu = this->menuBar()->addMenu(tr("捕获"));
	toolsMenu->addAction(chooseDevAction);
	toolsMenu->addSeparator();
	toolsMenu->addAction(beginAction);
	toolsMenu->addAction(endAction);
	toolsMenu->addSeparator();
	toolsMenu->addAction(findQQAction);

	this->menuBar()->addSeparator();

	helpMenu = this->menuBar()->addMenu(tr("帮助"));
	helpMenu->addAction(aboutAction);
	helpMenu->addAction(aboutQtAction);
}

void MainWindow::createToolBars()
{
	fileToolBar = addToolBar(tr("文件"));
	fileToolBar->addAction(newAction);
	fileToolBar->addAction(openAction);
	fileToolBar->addAction(saveAction);
	fileToolBar->addSeparator();
	fileToolBar->addAction(printAction);

	workToolBar = addToolBar(tr("捕获"));
	workToolBar->addAction(chooseDevAction);
	workToolBar->addSeparator();
	workToolBar->addAction(beginAction);
	workToolBar->addAction(endAction);
	workToolBar->addSeparator();
	workToolBar->addAction(findQQAction);

	infoToolBar = addToolBar(tr("帮助"));
	infoToolBar->addAction(aboutAction);
	infoToolBar->addAction(aboutQtAction);
}

void MainWindow::createStatusBar()
{
	statusLabel = new QLabel("Sniffer 测试版");
	statusLabel->setAlignment(Qt::AlignHCenter);
	statusLabel->setMinimumSize(statusLabel->sizeHint());

	this->statusBar()->addWidget(statusLabel);
}
