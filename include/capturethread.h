// =====================================================================================
// 
//       Filename:  capturethread.h
//
//    Description:  ��̨�������ݵĶ��߳���ͷ�ļ�
//
//        Version:  1.0
//        Created:  2013��01��24�� 21ʱ36��49��
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================
#ifndef CAPTURETHREAD_H_
#define CAPTURETHREAD_H_

#include <QThread.h>

class ListTreeView;
class Sniffer;

class  CaptureThread : public QThread
{
	Q_OBJECT

public:
	CaptureThread();
	CaptureThread(ListTreeView *pTree, Sniffer *pSniffer, QString tmpFileName = "");

	void stop();

protected:
	void run();

private:
	volatile bool bStopped;
	ListTreeView  *mainTree;
	Sniffer       *sniffer;
	QString		  tmpFile;
};

#endif	// CAPTURETHREAD_H_