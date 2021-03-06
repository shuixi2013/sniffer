// =====================================================================================
// 
//       Filename:  findqqthread.h
//
//    Description:  捕获QQ数据的多线程类头文件
//
//        Version:  1.0
//        Created:  2013年01月28日 14时36分49秒
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================
#ifndef FINDQQTHREAD_H_
#define FINDQQTHREAD_H_

#include <QThread>

class FindQQDialog;
class Sniffer;

class  FindQQThread : public QThread
{
	Q_OBJECT

public:
	FindQQThread();
	FindQQThread(FindQQDialog *findQQ, Sniffer *sni);
	
	void stop();

protected:
	void run();

private:
	volatile bool bStopped;
	FindQQDialog  *findQQDialog;
	Sniffer 	  *sniffer;
};

#endif	// FINDQQTHREAD_H_
