/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: f_wm_stock_log
*  程序描述			: 
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化			(ADD)程序建立
*			... ...
* **************************************************************************** */

#include "stdafx.h"

BM2_FUNCTION_EXPORT
int f_wm_stock_log(EIClass * bcls_rec, EIClass * bcls_ret, CString table_name, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int retCnt = 0;
	CString sqlstr = "";
	CString s_resume_seq_no = " ";
	CString s_shift_group = " ", s_shift_no = " ", s_operate_time = " ";


	CDbCommand	execute_sql(conn);
	//定义表实体对象
	CModel twma4 = CModel("TWMA4");
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	try
	{
		if (!bcls_rec->Tables.Contains("WM_STOCK_LOG"))
		{
			sprintf(s.msg, "函数f_wm00_stock_udpate中找不到接收块名[WM_STOCK_UPDATE]");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//获取班组班次信息/////////////////////////////////
		f_epep_get_shift_group("DEFAULT", datetime, s_shift_no, s_shift_group, conn);

		//传入参数检核
		for (int i = 0; i < bcls_rec->Tables["WM_STOCK_LOG"].Rows.get_Count(); i++)
		{
			twma4.MergeFrom(bcls_rec->Tables["WM_STOCK_LOG"].Rows[i]);
			twma4["REC_CREATOR"] = s.userid;
			twma4["REC_CREATE_TIME"] = datetime;
			twma4["RESUME_SEQ_NO"] = CDateTime::Now().ToString("yyyyMMddHHmmssff6");
			twma4["EVENT_TIME"] = datetime;
			twma4["SHIFT_GROUP"] = s_shift_group;
			twma4["SHIFT_NO"] = s_shift_no;
			twma4["FUNC_ID"] = s.svc_name;
			twma4["CLIENT_IP"] = s.fore_ip;

			twma4.TrimOrBlank();
			twma4.Insert();  ///写库位移动履历
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
	catch (const CApplicationException& ex)
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

