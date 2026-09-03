/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: wma2_inq
*  程序描述			: 库存信息查询
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化			(ADD)程序建立
*			... ...
* **************************************************************************** */

//框架头文件
#include "stdafx.h"

//函数申明
int f_epes_get_auth_other(const char *iuser, int irestype, EIClass *bcls_ret, CDbConnection * conn);

BM2F_ENTERACE(wma2_inq);
int f_wma2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int doFlag = 0;

	CDecimal rowCount = 0;
	int fetchRowCount = 0;
	CString table_name = " ";
	CString mat_no = "";
	CString stock_no = "";
	CString mat_kind = "";
	CString mat_line_type = "";
	int auto_stock_flag = 0;//是否库区授权  1--是  0--否

	/* 数据库SQL操作字符串 */
	CString sqlstr = "";
	CString sqlstr_count = "";
	CString sqlstr_left = "";
	CString sqlstr_select = " MAT_NO,MAT_LINE_TYPE,MAT_KIND,IN_FLAG";
	CString sqlwhere = "";

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	//系统的分页类信息。
	CPageInfo pageInfo;

	try
	{
		// 获取前台传入参数
		mat_kind = bcls_rec->Tables[0].Rows[0]["MAT_KIND"].ToString();
		mat_line_type = bcls_rec->Tables[0].Rows[0]["MAT_LINE_TYPE"].ToString();
		mat_no = bcls_rec->Tables[0].Rows[0]["MAT_NO"].ToString();
		stock_no = bcls_rec->Tables[0].Rows[0]["STOCK_NO"].ToString();

		Log::Trace("", __FUNCTION__, "参数mat_kind：\t[{0}]", mat_kind);
		Log::Trace("", __FUNCTION__, "参数mat_no：\t[{0}]", mat_no);
		Log::Trace("", __FUNCTION__, "参数mat_line_type：\t[{0}]", mat_line_type);
		Log::Trace("", __FUNCTION__, "参数stock_no：\t[{0}]", stock_no);

		if (mat_no.Trim() != "")
		{
			sqlwhere = sqlwhere + " AND  A.MAT_NO LIKE'" + mat_no.Trim() + "%'";
		}
		if (stock_no.Trim() != "")
		{
			sqlwhere = sqlwhere + " AND  A.STOCK_NO ='" + stock_no.Trim() + "'";
		}

		//获取库区授权
		CString stock_no_auth = "' '";
		EIClass *bcls_auth = new EIClass;
		if (f_epes_get_auth_other(s.userid, 5, bcls_auth, conn) != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		for (int fetchRowCount = 0; fetchRowCount < bcls_auth->Tables[0].Rows.get_Count(); fetchRowCount++)
		{
			stock_no_auth += ", '" + bcls_auth->Tables[0].Rows[fetchRowCount]["name"].ToString() + "' ";
		}
		delete bcls_auth;

		if (auto_stock_flag == 1)
			sqlwhere += " AND A.STOCK_NO IN (" + stock_no_auth + ")";


		//分页信息
		CDataTable& table = bcls_ret->Tables.Add("PAGEINFO");
		table.Columns.Add(DT_DECIMAL, "recordsum");

		//获取分页信息
		if (bcls_rec->Tables.Contains("PageInfo")){
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		else{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = -1;
		}//每页记录数量

		//拼装主档表
		CDataTable T_TABLE_NAME;
		sqlstr = "SELECT Name FROM SYSIBM.SYSTABLES WHERE TID <> 0 AND Name IN( 'TMMCR01','TMMHR01','TMMSM01','TMMHP01','TMMBW01')";
		Db::QueryTable(sqlstr, T_TABLE_NAME);
		if (T_TABLE_NAME.Rows.get_Count() == 0){
			sprintf(s.msg, "后台wm12_inq中未查询到主档表");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		table_name = "(SELECT " + sqlstr_select + " FROM " + T_TABLE_NAME.Rows[0][0].ToString();
		for (int i = 1; i < T_TABLE_NAME.Rows.get_Count(); ++i){
			table_name = table_name + " UNION SELECT " + sqlstr_select + " FROM " + T_TABLE_NAME.Rows[i][0].ToString();
		}
		table_name = table_name + ")";

		sqlstr = "SELECT  B.*,A.STOCK_NO,A.STOCK_PLACE_NO,A.LAYERNO FROM TWMA2 A ," + table_name + " B WHERE A.MAT_NO=B.MAT_NO AND B.IN_FLAG='1'";
		sqlstr_count = "SELECT COUNT(1) FROM TWMA0 A ," + table_name + " B WHERE A.MAT_NO=B.MAT_NO AND B.IN_FLAG='1'";
		sqlstr = sqlstr + sqlwhere;
		Log::Trace("", __FUNCTION__, "sqlstr：\t[{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);  //0,-1：非翻页查询
		cmd_inq.Close();

		//返回记录总数
		rowCount = Db::QueryCDecimal(sqlstr_count + sqlwhere);
		CDataRow& row1 = bcls_ret->Tables["PAGEINFO"].Rows.Add();
		row1["recordsum"] = rowCount.ToInt32();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员", arguments, 1);
		CString str = ex.GetMsg() + "\r\n" + sqlstr;
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);

		/*数据库异常时返回-1，事务将被回滚*/
		s.flag = -1;
		doFlag = -1;
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

