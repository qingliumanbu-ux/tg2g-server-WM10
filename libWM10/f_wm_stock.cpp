/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: f_wm00_stock_in
*  程序描述			: 仓库主函数
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化	(ADD)程序建立
*			... ...
* **************************************************************************** */
/* ***************************传入参数********************************
传入块名：WM_STOCK
传入列名：MAT_LINE_TYPE              产线类型                可空
          MAT_KIND                   物料类型                可空
          MAT_NO                     材料号                  非空
          STOCK_OPER_ORDER           库业务类型              非空
		  TO_STOCK_PLACE_NO          目标库位号              非空（倒跺和入库）
		  TO_LYAERNO                 目标层号                可空
* **************************************************************************** */
/* C/C++ 的标准头文件部分 */
#include "stdafx.h"		// 框架头，不可删除 

//库位更新函数
int f_wm_stock_update(EIClass *bcls_rec, EIClass *bcls_ret,CString table_name, CDbConnection *conn);
//物料函数
int f_wm_mm0099(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn);
//生产函数
int f_wm_pm0099(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn);
//计划函数
int f_wm_ps0099(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn);
//获取主档函数
CString f_gettable_name(CString mat_no, CString mat_kind, CString mat_line_type);

BM2_FUNCTION_EXPORT
int f_wm_stock(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__); 

	/* ***** 程序变量 ***** */
	int doFlag = 0;
	CString	c_mat_no = " ";
	CString	c_stock_oper_order = " ";
	CString	c_to_stock_place_no = " ";
	CString	c_mat_table = " ";
	CString	c_mat_line_type = " ";
	CString	c_mat_kind = " ";

	/* ***** 数据库SQL操作字符串 ***** */
	CString	sqlstr = " ";

	/* ***** 数据库操作类定义 ***** */
	CDbCommand comm(conn);

	/* ***** 应用程序开始处理 ***** */
	try
	{
		if (!bcls_rec->Tables.Contains("WM_STOCK")){
			sprintf(s.msg, "函数f_wm00_stock_in中找不到接收块名[WM_STOCK]");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (bcls_rec->Tables["WM_STOCK"].Rows.get_Count() == 0){
			Log::Trace("", __FUNCTION__, "函数f_wm00_stock_in中传入数据为空");
			return doFlag;
		}
	
		//传入列校验  
		if (!bcls_rec->Tables["WM_STOCK"].Columns.Contains("MAT_NO")){
			sprintf(s.msg, "函数f_wm_stock中接收块[WM_STOCK]没有列[MAT_NO]");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		if (!bcls_rec->Tables["WM_STOCK"].Columns.Contains("STOCK_OPER_ORDER")){
			sprintf(s.msg, "函数f_wm_stock中接收块[WM_STOCK]没有列[STOCK_OPER_ORDER]");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//获取产线类型和主档表
		if (bcls_rec->Tables["WM_STOCK"].Columns.Contains("MAT_LINE_TYPE"))
			c_mat_line_type = bcls_rec->Tables["WM_STOCK"].Rows[0]["MAT_LINE_TYPE"].ToString().Trim();

		if (bcls_rec->Tables["WM_STOCK"].Columns.Contains("MAT_KIND"))
			c_mat_kind = bcls_rec->Tables["WM_STOCK"].Rows[0]["MAT_KIND"].ToString().Trim();

		//传入产线根据产线获取主档表，未传的根据第一个材料号获取主档表
		c_mat_table = f_gettable_name(bcls_rec->Tables["WM_STOCK"].Rows[0]["MAT_NO"].ToString().Trim(), c_mat_kind, c_mat_line_type);
		Log::Trace("", __FUNCTION__, "主档表名[{0}]", c_mat_table);
		if (c_mat_table.Trim()==""){
			sprintf(s.msg, "函数f_wm_stock中获取主档表失败");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//调用更新库位函数
		doFlag = f_wm_stock_update(bcls_rec, bcls_ret, c_mat_table,conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
		//调用生产函数
		doFlag = f_wm_pm0099(bcls_rec, bcls_ret, c_mat_table, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
		//调用计划函数
		doFlag = f_wm_ps0099(bcls_rec, bcls_rec, c_mat_table, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//调用物料
		doFlag = f_wm_mm0099(bcls_rec, bcls_ret, c_mat_table, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CString str = ex.GetMsg() + "\r\n" + sqlstr;
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
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
