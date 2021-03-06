// =====================================================================================
// 
//       Filename:  settinginfo.h
//
//    Description:  程序设置信息类的头文件
//
//        Version:  1.0
//        Created:  2013年01月27日 14时15分49秒
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================

#ifndef SETTINGINFO_H_
#define SETTINGINFO_H_

#include <string>

// 程序配置信息类
struct SettingInfo
{
	int  		iOpenDevNum;			// 要打开的适配器编号
	bool 		bPromiscuous;			// 是否以混杂模式打开适配器
    bool 		bAutoBegin;				// 选择适配器后自动捕获
    int			iDataLimit;				// 捕获数据包大小限制
    std::string	filterString;			// 过滤器设置字符串

    SettingInfo()
    {
    	iOpenDevNum  = 0;
		bPromiscuous = true;
    	bAutoBegin   = false;
    	iDataLimit   = 65536;
    	filterString = "ip";
    }
};

#endif	// SETTINGINFO_H_
