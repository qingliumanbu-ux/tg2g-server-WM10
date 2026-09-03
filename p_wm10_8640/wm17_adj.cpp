/*************************************************************************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   wushan
Version:    3.1.1
Date:     2015-02-15
Description: 厚板钢板库位堆放的层号调整
**************************************************************************************************************/
//框架公用头文件，勿删
#include "stdafx.h"



/*<remark>=========================================================
/// <summary>
/// 1.查询到某个垛位下所有的材料
/// 2.根据传入的次序更新层号
/// <para>
/// </para>
/// <para>数据库表：TMMHP01(厚板物料主表)          </para>
/// <para>主调用函数： 前台YMHP012画面F6 层调整调用     </para>
/// </summary>
/// <param name="param1">参数1  </param>
/// <param name="param2">参数2  </param>
/// <returns>返回参数：0（成功）；-1（失败）  </returns>
===========================================================</remark>*/



BM2_FUNCTION_IMPORT
int f_WM_mm0099(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);



// service入口
BM2F_ENTERACE(WM17_adj)
//-EP_SYSTEM_HEAD_END
int f_WM17_adj(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志

	/* 程序内部变量 */
	int doFlag = 0;
	int blkNum = 0;
	int i, rows;
	CDecimal rowCount = 0;

	/* 业务变量 */
	CString update_data = "";  //修改的字段信息
	CString condition_data = "";  //过滤的字段信息。
	CDecimal layer_no;
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString mat_kind = "";
	CString stock_no = "";
	CString stock_place_no = "";
	CString mat_line_type = "";
	CString sqlstr;
	CDecimal layerno1 = 0;

	/* 实体类定义 */
	//CTWMA1 twma1(conn);
	//CTWMA2 twma2(conn), twma2_new(conn);
	//CHWMA2 hwma2(conn);
	//CTWM04 twm04(conn);

	CModel twma1 = CModel("TMMCR01");
	CModel twma2 = CModel("TWMA2");
	CModel twma2_new = CModel("TWMA2");
	CModel hwma2 = CModel("HWMA2");
	CModel twm04 = CModel("TWM04");

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
	bcls_rec_wmmm99.Tables[0].Rows.Clear();


	//调用计划内的钢卷位置变化电文发送传入数据块
	EIClass bcls_rec_mat_pos_chg;
	if (!bcls_rec_mat_pos_chg.Tables.Contains("MAT_POS_CHG"))
	{
		bcls_rec_mat_pos_chg.Tables[0].set_TableName("MAT_POS_CHG");
		bcls_rec_mat_pos_chg.Tables[0].Columns.Add(DT_STRING, "COIL_ID");
		bcls_rec_mat_pos_chg.Tables[0].Columns.Add(DT_STRING, "POSITION");
	}

	try
	{
		//获取前台传入参数
		rows = bcls_rec->Tables[0].Rows.get_Count();
		Log::Trace("", __FUNCTION__, "传入记录数 = [{0}]", rows);

		/*stock_no = bcls_rec->Tables[0].Rows[0]["STOCK_NO"].ToString().Trim();
		stock_place_no = bcls_rec->Tables[0].Rows[0]["STOCK_PLACE_NO"].ToString().Trim();
		layer_no = bcls_rec->Tables[0].Rows[0]["LAYER_NO"];

		Log::Trace("", __FUNCTION__, "传入参数STOCK_NO	   = [{0}]", stock_no);
		Log::Trace("", __FUNCTION__, "传入参数STOCK_PLACE_NO = [{0}]", stock_place_no);
		Log::Trace("", __FUNCTION__, "传入参数LAYER_NO = [{0}]", layer_no);*/

		for (i = 0; i < rows; i++)
		{
			twma1.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			twma2_new.MergeFrom(bcls_rec->Tables[0].Rows[i]);


			twma2["MAT_NO"] = twma1["MAT_NO"];
			twma2.Query("MAT_NO");
			layerno1 = twma2["LAYERNO"];

			twma2["LAYERNO"] = rows - i;
			Log::Trace("", __FUNCTION__, "传入参数 MAT_NO	   = [{0}]", twma1["MAT_NO"].ToString());
			Log::Trace("", __FUNCTION__, "传入参数 LAYERNO	   = [{0}]", rows - i);
			Log::Trace("", __FUNCTION__, "传入参数 STOCK_NO	   = [{0}]", twma1["STOCK_NO"].ToString());
			Log::Trace("", __FUNCTION__, "传入参数 STOCK_PLACE_NO	   = [{0}]", twma2["STOCK_PLACE_NO"].ToString());
			Log::Trace("", __FUNCTION__, "传入参数 STOCK_PLACE_POSITION	   = [{0}]", twma2["STOCK_PLACE_POSITION"].ToString());
			stock_place_no = twma2["STOCK_PLACE_NO"];
			//Log::Trace("", __FUNCTION__, "传入参数 MAT_LINE_TYPE	   = [{0}]", twma1.MAT_LINE_TYPE);

			twma1.Query("MAT_NO");

			Log::Trace("", __FUNCTION__, "传入参数 MAT_LINE_TYPE	   = [{0}]", twma1["MAT_LINE_TYPE"].ToString());

			twma2.Reset();
			twma2["MAT_NO"] = twma1["MAT_NO"];
			twma2["STOCK_NO"] = twma1["STOCK_NO"];
			Log::Trace("", "", "传入参数 STOCK_PLACE_NO	   = [{0}]", stock_place_no);
			twma2["STOCK_PLACE_NO"] = stock_place_no;
			twma2["STOCK_PLACE_POSITION"] = twma2_new["STOCK_PLACE_POSITION"];

			twma2["LAYERNO"] = rows - i;
			twma2["LOGIC_STOCK_NO"] = twma1["LOGIC_STOCK_NO"];
			//twma2.STOCK_PLACE_POSITION = twma2.STOCK_PLACE_POSITION;
			//row_update.Merge(twma2);
			//调用更新库位函数
			//doFlag = f_wm00_update_stock_place(&bcls_rec_update, bcls_ret, conn);
			//if (doFlag != 0)
			//{
			//	throw CApplicationException(-1, s.msg, log.Location);
			//}

			twma2.Update("LAYERNO, STOCK_PLACE_POSITION", "MAT_NO");

			hwma2.CopyFrom(twma2);
			hwma2["REC_CREATOR"] = s.formname;
			hwma2["REC_REVISE_TIME"] = datetime;
			hwma2.Insert();


			//调用物料跟踪
			bcls_rec_wmmm99.Tables["WMMM99"].Rows.Add();
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["MAT_NO"] = twma1["MAT_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["STOCK_OPER_ORDER"] = "30";
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["OLD_STOCK_NO"] = twma2["STOCK_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["OLD_STOCK_PLACE_NO"] = twma2["STOCK_PLACE_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[i]["OLD_LAYER_NO"] = layerno1;


			twm04["STOCK_PLACE_NO"] = twma2["STOCK_PLACE_NO"];
			twm04.Query("STOCK_PLACE_NO");
		}


		Log::Trace("", __FUNCTION__, "--------------------------调用WM00_MM0099函数开始------------------------------");
		//调用入库确认函数
		doFlag = f_WM_mm0099(&bcls_rec_wmmm99, bcls_ret, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		Log::Trace("", __FUNCTION__, "--------------------------调用WM00_MM0099函数开始------------------------------");

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
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

	return(doFlag);
}

