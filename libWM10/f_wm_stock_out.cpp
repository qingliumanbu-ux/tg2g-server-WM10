/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: f_wm00_stock_out
*  程序描述			: 仓库主函数
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化			(ADD)程序建立
*			... ...
* **************************************************************************** */
/* C/C++ 的标准头文件部分 */
#include "stdafx.h"		// 框架头，不可删除 

BM2_FUNCTION_EXPORT
int f_wm_stock_out(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* ***** 程序变量 ***** */
	int doFlag = 0;

	/* ***** 数据库SQL操作字符串 ***** */
	CString	sqlstr = " ";

	/* ***** 应用程序开始处理 ***** */
	CModel twma1 = CModel(table_name);
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); ++i)
		{
			twma1["MAT_NO"] = bcls_rec->Tables[0].Rows[i]["MAT_NO"].ToString();
			if (!twma1.Query("MAT_NO"))
			{
				sprintf(s.msg, "函数f_wm_stock_out中材料号[" + twma1["MAT_NO"].ToString() + "]不存在");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//主档表在库标记为1的材料不能再入库
			if (twma1["IN_FLAG"].ToString().Trim() != "1")
			{
				sprintf(s.msg, "函数f_wm_stock_out中材料号[" + twma1["MAT_NO"].ToString() + "]不在库中不能出库");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//出库完成删除出库队列
			Db::Execute("DELETE FROM TWMA0 WHERE MAT_NO='" + bcls_rec->Tables[0].Rows[i]["MAT_NO"].ToString() + "'");
		}
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
