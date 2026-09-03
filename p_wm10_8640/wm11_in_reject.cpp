/*************************************************
Copyright:		Baosight Software LTD.co Copyright (c) 2013
Author:
Version:		1.0
Date:			2016-03-05
Description:	来料拒收
**************************************************/

//框架头文件
#include "stdafx.h"
//程序用头文件
//#include "twm01.h"
//#include "twma0.h"
//#include "twma1.h"
//#include "twma2.h"

using namespace BM2;
using namespace BM2::Data;
using namespace BM2::Data::DbClient;



BM2_FUNCTION_IMPORT
int f_WM_mm0099(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);



BM2_FUNCTION_IMPORT
int f_wm00_queue(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);



/*<remark>=========================================================
///<summary>
///板坯入库功能
///<para>
///2.排序方式：
///</para>
///<para>数据库表：TWMA0 倒躲队列；TWMA1 物料主档表
///<returns>执行预材料预入库功能</returns>
===========================================================</remark>*/

BM2F_ENTERACE(WM11_in_reject);

int f_WM11_in_reject(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;
	int blkNum = 0;
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	CDecimal rowCount = 0;

	/*程序用变量*/
	CString stock_no = "";
	CString stock_place_no = "";
	CString vehicle_no = "";
	CString stock_oper_order_div = "";
	CString layerno = "";
	CString come_reject_cause = "";


	/* 实体类定义 */
	//CTWM01 twm01(conn);
	//CTWMA0 twma0(conn);
	//CTWMA1 twma1(conn);
	//CTWMA2 twma2(conn);
	CModel twm01 = CModel("TWM01");
	CModel twma0 = CModel("TWMA0");
	CModel twma1 = CModel("TMMCR01");
	CModel twma2 = CModel("TWMA2");

	/* 数据库SQL操作字符串 */
	CString sqlstr = "";
	CString sqlwhere = "";

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);



	//调用物料函数
	EIClass bcls_rec_wmmm99;
	bcls_rec_wmmm99.Tables[0].set_TableName("WMMM99");
	bcls_rec_wmmm99.Tables[0].Columns.Add(DT_STRING, "MAT_NO");
	bcls_rec_wmmm99.Tables[0].Columns.Add(DT_STRING, "STOCK_OPER_ORDER");
	bcls_rec_wmmm99.Tables[0].Columns.Add(DT_STRING, "STOCK_OPER_ORDER_DIV");
	bcls_rec_wmmm99.Tables[0].Columns.Add(DT_STRING, "OLD_STOCK_NO");
	bcls_rec_wmmm99.Tables[0].Columns.Add(DT_STRING, "OLD_STOCK_PLACE_NO");
	bcls_rec_wmmm99.Tables[0].Columns.Add(DT_DECIMAL, "OLD_LAYER_NO");
	bcls_rec_wmmm99.Tables[0].Columns.Add(DT_DECIMAL, "COME_REJECT_CAUSE");
	bcls_rec_wmmm99.Tables[0].Rows.Clear();




	//入库队列
	EIClass bcls_stock_que;
	bcls_stock_que.Tables.Add("WM00QUE");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "STOCK_OPER_ORDER");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "STOCK_OPER_ORDER_DIV");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "MAT_NO");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "MAT_NUM");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "PLAN_NO");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "PLAN_EXEC_SEQ_NO");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "STOCK_NO");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "TRANS_TOOL");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "PRE_UNIT_CODE");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "NEXT_UNIT_CODE");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "MAT_DESTION");
	bcls_stock_que.Tables["WM00QUE"].Columns.Add(DT_STRING, "OPER_FLAG");
	bcls_stock_que.Tables["WM00QUE"].Rows.Clear();


	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			twma0.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			stock_no = bcls_rec->Tables[0].Rows[i]["TO_STOCK_NO"].ToString();
			stock_place_no = bcls_rec->Tables[0].Rows[i]["TO_STOCK_PLACE_NO"].ToString();
			vehicle_no = bcls_rec->Tables[0].Rows[i]["VEHICLE_NO"].ToString();
			come_reject_cause = bcls_rec->Tables[1].Rows[0]["COME_REJECT_CAUSE"].ToString();

			Log::Trace("", __FUNCTION__, "参数赋值mat_no：\t[{0}]", twma0["MAT_NO"].ToString());
			Log::Trace("", __FUNCTION__, "参数赋值twma0.STOCK_OPER_ORDER：\t[{0}]", twma0["STOCK_OPER_ORDER"].ToString());
			Log::Trace("", __FUNCTION__, "参数赋值stock_oper_order_div：\t[{0}]", stock_oper_order_div);
			Log::Trace("", __FUNCTION__, "参数赋值stock_no：\t[{0}]", stock_no);
			Log::Trace("", __FUNCTION__, "参数赋值stock_place_no：\t[{0}]", stock_place_no);
			Log::Trace("", __FUNCTION__, "参数赋值vehicle_no：\t[{0}]", vehicle_no);
			Log::Trace("", __FUNCTION__, "参数赋值come_reject_cause：\t[{0}]", come_reject_cause);



			if (twma0["STOCK_OPER_ORDER"].ToString().Trim() != "1G" &&
				twma0["STOCK_OPER_ORDER"].ToString().Trim() != "1X")
			{
				sprintf(s.msg, "只有转库入库、热送入库可以拒收。");
				throw CApplicationException(-1, s.msg, log.Location);
			}


			twma0["PROC_STATUS"] = "9";
			twma0["STOCK_OPER_ORDER_DIV"] = "D";
			twma0.Update("PROC_STATUS, STOCK_OPER_ORDER_DIV", "MAT_NO, STOCK_OPER_ORDER");


			//调用物料跟踪
			bcls_rec_wmmm99.Tables["WMMM99"].Rows.Add();
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["MAT_NO"] = twma0["MAT_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["STOCK_OPER_ORDER"] = twma0["STOCK_OPER_ORDER"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["STOCK_OPER_ORDER_DIV"] = twma0["STOCK_OPER_ORDER_DIV"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["OLD_STOCK_NO"] = twma2["STOCK_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["OLD_STOCK_PLACE_NO"] = twma2["STOCK_PLACE_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["OLD_LAYER_NO"] = twma2["LAYERNO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["COME_REJECT_CAUSE"] = come_reject_cause;




			//删除队列
			twma0.Query("MAT_NO");

			twma0.Delete("MAT_NO");

			Log::Trace("", __FUNCTION__, "来源库区 twma0.FROM_STOCK_NO[{0}]", twma0["FROM_STOCK_NO"].ToString());

			//插入上分厂入库队列
			twm01["STOCK_NO"] = twma0["FROM_STOCK_NO"];
			if (twm01.Query("STOCK_NO"))
			{
				if (twm01["STOCK_TYPE_CODE"].ToString().Trim() == "0")
				{
					bcls_stock_que.Tables["WM00QUE"].Rows.Add();
					int ii = bcls_stock_que.Tables["WM00QUE"].Rows.get_Count() - 1;
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["MAT_NO"] = twma0["MAT_NO"];
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["MAT_NUM"] = twma0["MAT_NUM"];
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["PLAN_NO"] = " ";
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["PLAN_EXEC_SEQ_NO"] = 0;
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["STOCK_NO"] = twma0["FROM_STOCK_NO"];
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["TRANS_TOOL"] = " ";
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["PRE_UNIT_CODE"] = twma0["UNIT_CODE"];
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["NEXT_UNIT_CODE"] = twma0["NEXT_UNIT_CODE"];
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["OPER_FLAG"] = "I";
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["STOCK_OPER_ORDER"] = "1Q";
					bcls_stock_que.Tables["WM00QUE"].Rows[ii]["STOCK_OPER_ORDER_DIV"] = " ";
				}
			}
		}


		//调用物料函数 需单独配置 操作细分为D
		doFlag = f_WM_mm0099(&bcls_rec_wmmm99, bcls_ret, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//目标库区入库队列
		doFlag = f_wm00_queue(&bcls_stock_que, bcls_ret, conn);
		if (doFlag < 0)
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
