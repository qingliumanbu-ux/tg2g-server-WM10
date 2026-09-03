/*************************************************
Copyright:		Baosight Software LTD.co Copyright (c) 2011
Author:			wuxin
Version:		1.0
Date:			2016-08-25
Description:	修正材料被异常抽走后上面材料的层号
**************************************************/

#include "stdafx.h"

BM2_FUNCTION_IMPORT
int f_WM_mm0099(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);

BM2_FUNCTION_EXPORT
int f_WM_modi_pile_layerno(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* ***** 静态变量定义 ***** */
	int doFlag = 0;
	CString s_stock_place_no = "", s_stock_no = " ", s_mat_no = " ", s_message = " ", s_svc_name = " ";
	CDecimal d_layerno = 0, i_layerno = 0;
	CDecimal old_layerno = 0;

	/*数据库SQL操作字符串，用于捕获数据库操作异常情况*/
	CString sqlstr = "";
	CDbCommand execute_sql(conn);

	//定义表实体对象

	CModel twma1 = CModel("TMMCR01");
	CModel twma2 = CModel("TWMA2");
	CModel twm04 = CModel("TWM04");

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


	try
	{

		//计算层数
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）		 
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库			 
		default: // 所有数据库适用，通用SQL语句					
			sqlstr =
				" select * "
				" from twma2 "
				" where stock_no = @stock_no "
				" and stock_place_no = @stock_place_no "
				" and layerno > @layerno"
				" order by layerno asc "
				;
			break;
		}
		///////////////////////////////////////////////////获取输入参数///////////////////////////////////////////// 
		s_stock_place_no = bcls_rec->Tables[0].Rows[0]["STOCK_PLACE_NO"].ToString();
		s_stock_no = bcls_rec->Tables[0].Rows[0]["STOCK_NO"].ToString();
		d_layerno = bcls_rec->Tables[0].Rows[0]["LAYERNO"].ToDecimal();

		Log::Trace("", __FUNCTION__, "传入参数s_stock_place_no					= [{0}]", s_stock_place_no);
		Log::Trace("", __FUNCTION__, "传入参数s_stock_no					= [{0}]", s_stock_no);
		Log::Trace("", __FUNCTION__, "传入参数d_layerno					= [{0}]", d_layerno);

		twm04["STOCK_NO"] = s_stock_no;
		twm04["STOCK_PLACE_NO"] = s_stock_place_no;

		if (!twm04.Query("STOCK_NO, STOCK_PLACE_NO"))
		{
			sprintf(s.msg, "库位号不存在");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		////MANAGE_ACCU "1" 按垛管理，跟踪到层 
		////MANAGE_ACCU "2" 按垛管理，不跟踪到层 
		////MANAGE_ACCU "3" 按列管理，跟踪到层 
		////MANAGE_ACCU "4" 按列管理，不跟踪到层 
		////MANAGE_ACCU "5" 一品一地

		if (twm04["MANAGE_ACCU"].ToString() != "1")
		{
			return doFlag;
		}

		////////////////////////////////////////////////////////////////////////////////////
		execute_sql.SetCommandText(sqlstr);
		execute_sql.Parameters.Set("stock_place_no", s_stock_place_no);
		execute_sql.Parameters.Set("stock_no", s_stock_no);
		execute_sql.Parameters.Set("layerno", d_layerno);
		execute_sql.ExecuteReader();
		int Row_MM = 0;

		while (execute_sql.Read())
		{
			i_layerno = i_layerno + 1;
			execute_sql.Fetch(twma2);

			twma1["MAT_NO"] = twma2["MAT_NO"];
			if (!twma1.Query("MAT_NO"))
			{
				s_message = "材料号：" + twma1["MAT_NO"].ToString() + "在物料主档中不存在！";
				sprintf(s.msg, s_message);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			twma2.Query("MAT_NO");
			old_layerno = twma2["LAYERNO"].ToDecimal();

			twma2["LAYERNO"] = d_layerno + i_layerno - 1;
			twma2.Update("LAYERNO", "MAT_NO");

			//调用物料跟踪
			bcls_rec_wmmm99.Tables["WMMM99"].Rows.Add();
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[Row_MM]["MAT_NO"] = twma1["MAT_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[Row_MM]["STOCK_OPER_ORDER"] = "30";
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[Row_MM]["OLD_STOCK_NO"] = twma2["STOCK_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[Row_MM]["OLD_STOCK_PLACE_NO"] = twma2["STOCK_PLACE_NO"];
			bcls_rec_wmmm99.Tables["WMMM99"].Rows[Row_MM]["OLD_LAYER_NO"] = old_layerno;
			Row_MM++;

		}
		execute_sql.Close();

		Log::Trace("", __FUNCTION__, "--------------------------调用WM00_MM0099函数开始------------------------------");
		//调用入库确认函数
		if (Row_MM > 0)
		{
			doFlag = f_WM_mm0099(&bcls_rec_wmmm99, bcls_ret, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		Log::Trace("", __FUNCTION__, "--------------------------调用WM00_MM0099函数开始------------------------------");

	}
	/*捕获数据库操作异常*/
	catch (CDbException& ex)
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
	/*捕获应用错误*/
	catch (CApplicationException& ex)
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

	s.flag = doFlag;

	return(doFlag);
}

