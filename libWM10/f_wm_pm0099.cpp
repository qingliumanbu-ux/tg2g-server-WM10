/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: f_WM_pm0099
*  程序描述			: PM生产同步电文处理
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化			(ADD)程序建立
*			... ...
* **************************************************************************** */
/* C/C++ 的标准头文件部分 */
#include "stdafx.h"		// 框架头，不可删除 

//外部函数声明
BM2_FUNCTION_IMPORT
int f_pmol_zk(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

BM2_FUNCTION_EXPORT
int f_wm_pm0099(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* ***** 程序变量 ***** */
	int doFlag = 0;
	CString v_datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString v_mat_no = " ";
	CString v_stock_oper_order = " ";
	CString v_vehicle_no = " ";
	int ii = 0;
	CString	sqlstr = "";

	//调用生产函数
	EIClass bcls_pmol02;
	bcls_pmol02.Tables[0].set_TableName("PMOL02");
	bcls_pmol02.Tables[0].Columns.Add(DT_STRING, "MAT_NO");
	bcls_pmol02.Tables[0].Columns.Add(DT_STRING, "MOVE_OUT_TIME");
	bcls_pmol02.Tables[0].Columns.Add(DT_STRING, "VEHICLE_NO");
	bcls_pmol02.Tables[0].Columns.Add(DT_STRING, "TRANSFER_TYPE");
	bcls_pmol02.Tables[0].Rows.Clear();

	CModel twma1 = CModel(table_name);


	/* ***** 应用程序开始处理 ***** */
	try
	{
		for (int iRow = 0; iRow < bcls_rec->Tables["WM_STOCK"].Rows.get_Count(); iRow++)
		{
			v_mat_no = bcls_rec->Tables["WM_STOCK"].Rows[iRow]["MAT_NO"].ToString().Trim();
			v_stock_oper_order = bcls_rec->Tables["WM_STOCK"].Rows[iRow]["STOCK_OPER_ORDER"].ToString().Trim();

			twma1["MAT_NO"] = v_mat_no;
			twma1.Query("MAT_NO");

			if (twma1["TRANSFER_FLAG"].ToString().Trim() == "1")
			{
				if (v_stock_oper_order[0] == '1')
				{
					bcls_pmol02.Tables["PMOL02"].Rows.Add();
					ii = bcls_pmol02.Tables["PMOL02"].Rows.get_Count() - 1;
					bcls_pmol02.Tables["PMOL02"].Rows[ii]["MAT_NO"] = v_mat_no;
					bcls_pmol02.Tables["PMOL02"].Rows[ii]["MOVE_OUT_TIME"] = v_datetime;
					bcls_pmol02.Tables["PMOL02"].Rows[ii]["TRANSFER_TYPE"] = "1";
				}
				else if (v_stock_oper_order[0] == '2')
				{
					bcls_pmol02.Tables["PMOL02"].Rows.Add();
					ii = bcls_pmol02.Tables["PMOL02"].Rows.get_Count() - 1;
					bcls_pmol02.Tables["PMOL02"].Rows[ii]["MAT_NO"] = v_mat_no;
					bcls_pmol02.Tables["PMOL02"].Rows[ii]["MOVE_OUT_TIME"] = v_datetime;
					bcls_pmol02.Tables["PMOL02"].Rows[ii]["TRANSFER_TYPE"] = "0";
				}
			}
		}
		if (bcls_pmol02.Tables["PMOL02"].Rows.get_Count() > 0)
		{
			doFlag = f_pmol_zk(&bcls_pmol02, bcls_ret, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006"), arguments, 1);
		CString str = ex.GetMsg() + "\r\n" + sqlstr;
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
