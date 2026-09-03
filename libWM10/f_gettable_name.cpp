/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: f_gettable_name
*  程序描述			: 
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化	(ADD)程序建立
*			... ...
* **************************************************************************** */
#include "stdafx.h"		// 框架头，不可删除 


BM2_FUNCTION_EXPORT
CString f_gettable_name(CString mat_no, CString mat_kind, CString mat_line_type)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义


	/* ***** 程序变量 ***** */
	int doFlag = 0;
	
	/* ***** 数据库SQL操作字符串 ***** */
	CString	sqlstr = "";
	/* ***** 应用程序开始处理 ***** */
	try
	{		
		CString Table = "";
		if (mat_no.Trim()!="")
		{
			if (Db::QueryCDecimal("SELECT COUNT(1) FROM TMMCR01 WHERE MAT_NO='" + mat_no + "'")>0)
			{
				Table = "TMMCR01";
			}
			else if (Db::QueryCDecimal("SELECT COUNT(1) FROM TMMHR01 WHERE MAT_NO='" + mat_no + "'")>0)
			{
				Table = "TMMHR01";
			}
			else if (Db::QueryCDecimal("SELECT COUNT(1) FROM TMMBW01 WHERE MAT_NO='" + mat_no + "'")>0)
			{
				Table = "TMMBW01";
			}
			else if (Db::QueryCDecimal("SELECT COUNT(1) FROM TMMSM01 WHERE MAT_NO='" + mat_no + "'")>0)
			{
				Table = "TMMSM01";
			}
			else if (Db::QueryCDecimal("SELECT COUNT(1) FROM TMMHP01 WHERE MAT_NO='" + mat_no + "'")>0)
			{
				Table = "TMMHP01";
			}
		}
		else if (mat_line_type=="SM")
		{
			Table = "TMMSM01";
		}
		else if (mat_kind == "HR")
		{
			Table = "TMMHR01";
		}
		else if (mat_kind == "HP")
		{
			Table = "TMMHP01";
		}
		else if (mat_kind == "BW")
		{
			Table = "TMMBW01";
		}
		else if (mat_kind == "CR")
		{
			Table = "TMMCR01";
		}
		

		return Table;



	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };

		/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006"), arguments, 1);
		CString str = ex.GetMsg() + "\r\n" + sqlstr;

		/*返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应*/
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);

		/*数据库异常时返回-1，事务将被回滚*/
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), 399);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), 399);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
