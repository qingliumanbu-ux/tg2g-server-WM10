/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2013
Author:      ljnie
Version:     3.1.1
Date:        2016/7/6 16:08:19
Description: 库位材料信息查询WM17
**************************************************/
/***** C/C++ 的标准头文件部分 *****/
#include "stdafx.h"

/***** 头文件部分 *****/
//#include "twm04.h"
//#include "twma1.h"

/*<remark>=========================================================
/// <summary>
///  库位材料信息查询
/// <para>根据查询条件查询库位材料信息
/// </para>
/// <para>数据库表：</para>
/// </summary>
/// <param name="STOCK_PLACE_NO">库位号  </param>
/// <returns>返回参数：材料数据</returns>
===========================================================</remark>*/
// Service 入口
BM2F_ENTERACE(wm17_inq_mat)

int f_wm17_inq_mat(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	/* 业务变量 */
	CString mat_no("");
	CDecimal  cmd_flag = 0;

	/* 数据库SQL操作字符串 */
	CString sqlstr("");
	CString sql("");
	CString sqlwhere("");
	CString sqlorderby("");

	/* 实体类定义 */
	//CTWM04 twm04(conn);
	//CTWMA1 twma1(conn);
	CModel twm04 = CModel("TWM04");
	CModel twma1 = CModel("TMMCR01");

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	try
	{
		// 获取前台传入参数
		twm04["STOCK_PLACE_NO"] = bcls_rec->Tables[0].Rows[0]["STOCK_PLACE_NO"].ToString().Trim();
		twm04["ROWNO"] = bcls_rec->Tables[0].Rows[0]["ROWNO"].ToString().Trim();
		twm04["COLUMN_NO"] = bcls_rec->Tables[0].Rows[0]["COLUMN_NO"].ToString().Trim();
		twma1["MAT_LINE_TYPE"] = bcls_rec->Tables[0].Rows[0]["MAT_LINE_TYPE"].ToString().Trim();
		//twm04["HALL_NO"] = bcls_rec->Tables[0].Rows[0]["HALL_NO"].ToString().Trim();
		twm04["STOCK_NO"] = bcls_rec->Tables[0].Rows[0]["STOCK_NO"].ToString().Trim();

		/* ***** 打印输入参数 ***** */
		Log::Trace("", __FUNCTION__, "传入参数STOCK_PLACE_NO = [{0}]", twm04["STOCK_PLACE_NO"].ToString());
		Log::Trace("", __FUNCTION__, "传入参数ROWNO = [{0}]", twm04["ROWNO"].ToString());
		Log::Trace("", __FUNCTION__, "传入参数COLUMN_NO = [{0}]", twm04["COLUMN_NO"].ToString());
		Log::Trace("", __FUNCTION__, "传入参数MAT_LINE_TYPE = [{0}]", twma1["MAT_LINE_TYPE"].ToString());
		Log::Trace("", __FUNCTION__, "传入参数HALL_NO = [{0}]", twm04["HALL_NO"].ToString());
		Log::Trace("", __FUNCTION__, "传入参数STOCK_NO = [{0}]", twm04["STOCK_NO"].ToString());

		//		if (!twm04.Query())
		//		{
		//			//CFormattable arguments[] = { twm04.STOCK_PLACE_NO }; // 定义参数列表的数组
		//			//CMessageFormat::Format(s.msg, _RES("YM00S0000551")/*库位[{0}]不存在。*/, arguments, 1);
		//			sprintf(s.msg, "库位[%s]不存在。", (const char*)twm04.STOCK_PLACE_NO);
		//			throw CApplicationException(-1, s.sysmsg, log.Location);
		//		}
		Log::Trace("", __FUNCTION__, "11111111");

		if (twm04["ROWNO"].ToString().Trim() != ""){
			sqlwhere = " AND ROWNO = @rowno";
		}
		if (twm04["COLUMN_NO"].ToString().Trim() != ""){
			sqlwhere = " AND COLUMN_NO = @column_no";
		}

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:			// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:			// MS SQL Server数据库
			sql =
				" SELECT * FROM TMMCR01 A LEFT JOIN TWMA2 B ON A.MAT_NO=B.MAT_NO "
				" WHERE B.STOCK_PLACE_NO = @stock_place_no"
				" AND B.STOCK_NO = @stock_no ";
			sqlorderby = " ORDER BY	CAST(ISNULL(ltrim(rtrim(b.LAYERNO)), '0') AS INT) DESC, a.REC_CREATE_TIME ASC ";
			break;
		case DB_KIND_ORACLE:		// Oracle 数据库
			sql =
				" SELECT *"
				" FROM TMMCR01 A, TWMA2 B"
				" WHERE A.MAT_NO = B.MAT_NO"
				" AND B.STOCK_PLACE_NO = @stock_place_no"
				" AND B.STOCK_NO = @stock_no";

			sqlorderby = " ORDER BY B.LAYERNO DESC";
			break;
		}
		Log::Trace("", __FUNCTION__, "22222222");
		sqlstr = sql + sqlwhere + sqlorderby;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stock_place_no", twm04["STOCK_PLACE_NO"].ToString());
		//cmd_inq.Parameters.Set("mat_line_type", twma1.MAT_LINE_TYPE);
		cmd_inq.Parameters.Set("rowno", twm04["ROWNO"].ToString());
		cmd_inq.Parameters.Set("column_no", twm04["COLUMN_NO"].ToString());
		cmd_inq.Parameters.Set("hall_no", twm04["HALL_NO"].ToString());
		cmd_inq.Parameters.Set("stock_no", twm04["STOCK_NO"].ToString());

		Log::Trace("", __FUNCTION__, "sqlstr = [{0}]", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], 0, -1);  //0,-1：非翻页查询
		cmd_inq.Close();
	}
	catch (CDbException& ex)					//捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000021")/*信息读取失败。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1; //数据库异常时返回-1，事务将被回滚
	}
	catch (const CApplicationException& ex)	//捕获应用错误
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