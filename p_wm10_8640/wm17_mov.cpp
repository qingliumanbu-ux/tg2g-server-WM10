/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: wm17_mov
*  程序描述			: 仓库倒跺后台
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化			(ADD)程序建立
*			... ...
* **************************************************************************** */

//框架头文件
#include "stdafx.h"

//函数申明
BM2_FUNCTION_IMPORT
int f_wm_stock(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);

BM2F_ENTERACE(wm17_mov);
int f_wm17_mov(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;
	CString sqlstr = "";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	/*程序用变量*/
	CString stock_oper_order = "30";
	CString mat_no = "";
	CString stock_no = "";
	CString stock_no_old = "";
	CString stock_place_no = "";

	//调用仓库入库主函数
	EIClass bcls_stock_in;
	bcls_stock_in.Tables[0].set_TableName("WM_STOCK");
	bcls_stock_in.Tables[0].Columns.Add(DT_STRING, "MAT_NO");
	bcls_stock_in.Tables[0].Columns.Add(DT_STRING, "STOCK_OPER_ORDER");
	bcls_stock_in.Tables[0].Columns.Add(DT_STRING, "STOCK_PLACE_NO");

	try
	{

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			mat_no = bcls_rec->Tables[0].Rows[i]["MAT_NO"].ToString().Trim();
			stock_no = bcls_rec->Tables[0].Rows[i]["STOCK_NO"].ToString().Trim();
			stock_place_no = bcls_rec->Tables[0].Rows[i]["TO_STOCK_PLACE_NO"].ToString().Trim();
			Log::Trace("", __FUNCTION__, "参数赋值mat_no：\t[{0}]", mat_no);
			Log::Trace("", __FUNCTION__, "参数赋值stock_oper_order：\t[{0}]", stock_oper_order);
			Log::Trace("", __FUNCTION__, "参数赋值stock_no：\t[{0}]", stock_no);
			Log::Trace("", __FUNCTION__, "参数赋值stock_place_no：\t[{0}]", stock_place_no);

			//判断当前库区和目标库区是否相同
			stock_no_old = Db::QueryCString("SELECT STOCK_NO FROM TWMA2 WHERE MAT_NO='" + mat_no + "'");
			if (stock_no_old.Trim() == ""){
				sprintf(s.msg, "后台wm17_mov中：材料原库区为空");
				throw CApplicationException(-1, s.msg, log.Location);
			
			}
			if (stock_no.Trim() == ""){
				sprintf(s.msg, "后台wm17_mov中：材料目标库区为空");
				throw CApplicationException(-1, s.msg, log.Location);

			}
			if (stock_no_old != stock_no)stock_oper_order = "32";

			//调用仓库入库主函数
			bcls_stock_in.Tables["WM_STOCK"].Rows.Add();
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["MAT_NO"] = mat_no;
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER"] = stock_oper_order;
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["STOCK_PLACE_NO"] = stock_place_no;
		}

		doFlag = f_wm_stock(&bcls_stock_in, bcls_ret, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;

}

