/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: f_wm00_mm0099
*  程序描述			: 物料主档/物料同步电文处理
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化			(ADD)程序建立
*			... ...
* **************************************************************************** */
/* C/C++ 的标准头文件部分 */
#include "stdafx.h"		// 框架头，不可删除 

//外部函数声明
int f_mm0099(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

BM2_FUNCTION_EXPORT
int f_wm_mm0099(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* ***** 程序变量 ***** */
	int doFlag = 0;
	CString v_datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString v_mat_no = "";
	CString v_stock_oper_order = "";
	CString v_stock_oper_order_div = "";
	CString v_old_stock_no = "";
	CString v_old_stock_place_no = "";
	CDecimal v_old_layer_no = 0;
	CString v_eventid = "";
	CString v_tcno = "";
	CString v_come_reject_cause = " ";

	/* ***** 数据库SQL操作字符串 ***** */
	CString	sqlstr = "";

	/* ***** 数据库操作类定义 ***** */
	ParameterList paraList;
	RecordList recordList;
	Record record;

	/* ***** 定义表实体对象 ***** */
	CModel twma0 = CModel("TWMA0");
	CModel twma1 = CModel(table_name);
	CModel twma2 = CModel("TWMA2");

	EIClass bcls_rec_mm99;
	bcls_rec_mm99.Tables[0].set_TableName("MM0099");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_ID");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_DESC");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "EVENT_LINE_TYPE");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "SYSTEM_ID");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "FUNC_ID");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "FACTORY_STORE");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "SLABTOP_FLAG");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "SLABTOP_TIME");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "SLAB_RETURN_CAUSE_CODE");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(DT_STRING, "BACK_MAT_REASON");
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(twma1);
	bcls_rec_mm99.Tables["MM0099"].Columns.Add(twma2);
	bcls_rec_mm99.Tables["MM0099"].Rows.Clear();

	/* ***** 应用程序开始处理 ***** */
	try
	{
		for (int iRow = 0; iRow < bcls_rec->Tables["WM_STOCK"].Rows.get_Count(); iRow++)
		{
			v_mat_no = bcls_rec->Tables["WM_STOCK"].Rows[iRow]["MAT_NO"].ToString().Trim();
			v_stock_oper_order = bcls_rec->Tables["WM_STOCK"].Rows[iRow]["STOCK_OPER_ORDER"].ToString().Trim();

			Log::Trace("", __FUNCTION__, "传入参数 v_mat_no\t[{0}]", v_mat_no);
			Log::Trace("", __FUNCTION__, "传入参数 v_stock_oper_order\t[{0}]", v_stock_oper_order);

			twma1["MAT_NO"] = v_mat_no;
			if (!twma1.Query("MAT_NO"))
			{
				sprintf(s.msg, "TWMA1找不到材料[{%s}]", (const char*)twma1["MAT_NO"]);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			twma2["MAT_NO"] = v_mat_no;

			//获取事件号
			sqlstr =
				" SELECT EVENT_ID FROM TWM0B"
				" WHERE MAT_LINE_TYPE = @mat_line_type"
				" AND MAT_KIND = @mat_kind"
				" AND STOCK_OPER_ORDER = @stock_oper_order";

			v_eventid = "";

			if (v_eventid.Trim() == "")
			{
				paraList.Set("mat_line_type", twma1["MAT_LINE_TYPE"].ToString());
				paraList.Set("mat_kind", twma1["MAT_KIND"].ToString());
				paraList.Set("stock_oper_order", v_stock_oper_order);
				v_eventid = Db::QueryCString(sqlstr, paraList);
			}

			if (v_stock_oper_order == "2R")
			{
				if (v_eventid.Trim() == "")
				{
					//未配置不调用
					sprintf(s.msg, "TWM0B表中未配置事件号");
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			if (v_eventid.Trim() == "")
			{
				if (v_stock_oper_order[0] == '1')
				{
					if (v_stock_oper_order_div.Trim() != "D")
					{
						v_eventid = "WM01";
					}
					else
					{
						v_eventid = "WM10";
					}
				}
				else if (v_stock_oper_order[0] == '2')
				{
					v_eventid = "WM02";
				}
				else if (v_stock_oper_order[0] == '3')
				{
					v_eventid = "WM03";
				}
			}

			Log::Trace("", __FUNCTION__, "v_eventid\t[{0}]", v_eventid);

			if (v_eventid.Trim() == "")
			{
				sprintf(s.msg, "TWM0B表中未配置事件号");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//调用物料函数f_mm0099
			bcls_rec_mm99.Tables["MM0099"].Rows.Add();
			int row_count = bcls_rec_mm99.Tables["MM0099"].Rows.get_Count();

			CDataTable dt_temp;
			dt_temp.Clear();
			twma1.MergeTo(dt_temp);
			bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1].Merge(dt_temp.Rows[0]);

			if (twma2.Query("MAT_NO"))
			{
				dt_temp.Clear();
				twma2.MergeTo(dt_temp);
				bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1].Merge(dt_temp.Rows[0]);
			}

			bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["EVENT_ID"] = v_eventid;
			bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["EVENT_LINE_TYPE"] = "00";
			bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["SYSTEM_ID"] = "WM00";
			bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["FUNC_ID"] = s.svc_name;
			bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["SLABTOP_TIME"] = v_datetime;
			bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["STOCK_OPER_ORDER"] = v_stock_oper_order;

			if (twma2["STOCK_NO"].ToString().Trim() != "")
			{
				sqlstr =
					" SELECT FACTORY_DIV,MAT_LINE_TYPE FROM TWM01"
					" WHERE STOCK_NO = @stock_no";
				paraList.Set("stock_no", twma2["STOCK_NO"]);
				record = Db::QueryFirst(sqlstr, paraList);

				bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["FACTORY_STORE"] = record.GetCString("FACTORY_DIV");
				bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["MAT_LINE_TYPE"] = record.GetCString("MAT_LINE_TYPE");
			}

			if (v_stock_oper_order[0] == '1')
			{
				bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["IN_FLAG"] = "1";
				bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["IN_STOCK_TIME"] = v_datetime;
			}
			else if (v_stock_oper_order[0] == '2')
			{
				bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["IN_FLAG"] = "0";
				bcls_rec_mm99.Tables["MM0099"].Rows[row_count - 1]["OUT_STOCK_TIME"] = v_datetime;
			}
			
		}
		
		if (bcls_rec_mm99.Tables["MM0099"].Rows.get_Count() > 0)
		{
			doFlag = f_mm0099(&bcls_rec_mm99, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
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

