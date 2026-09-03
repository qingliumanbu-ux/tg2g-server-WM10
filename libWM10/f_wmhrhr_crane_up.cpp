/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:      
Version:     1.1.1
Date:       
Description: 吊车命令吊起函数
**************************************************/

#include "stdafx.h"

BM2_FUNCTION_IMPORT		//垛位清空
int f_wm00_move_to_null(CString matNo, CString moveType, CDbConnection *conn);

BM2_FUNCTION_EXPORT
int f_wmcrcr_crane_up(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	int cmdSeq = 0;
	CString matNo = " ";
	CString sqlWhere = " ";
	CString sqlWherePlace = " ";
	CString shiftGroup = " ";
	CString shiftNo = " ";
	CString stockNoFr = " ";
	CString stockPlaceNoFr = " ";
	CString tcNo = " ";
	CString logic_stock = " ";
	CString dateTime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	//定义表实体对象
	CModel hwm00a7 = CModel("HWM00A7");
	CModel twma7 = CModel("TWMA7");
	CModel tophpmms1 = CModel("TWMA1");
	CModel twm04 = CModel("TWM04");

	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	try
	{
		//判断是否存在指定块
		if (bcls_rec->Tables.IndexOf("WM00_UP") < 0 ||
			bcls_rec->Tables["WM00_UP"].Rows.get_Count() == 0)
		{
			sprintf(s.msg, "函数f_wmcrcr_crane_up中找不到接收块名[WM00_UP]或值为空");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//获取班组班次信息
		f_epep_get_shift_group("CR", dateTime, shiftNo, shiftGroup, conn);

		//循环获取传入行车命令块数据
		for (int i = 0; i < bcls_rec->Tables["WM00_UP"].Rows.get_Count(); i++)
		{
			matNo = bcls_rec->Tables["WM00_UP"].Rows[i]["MAT_NO"];
			stockPlaceNoFr = bcls_rec->Tables["WM00_UP"].Rows[i]["STOCK_PLACE_NO_FR"];
			Log::Debug("", __FUNCTION__, "MAT_NO  = [{0}]", matNo);
			Log::Debug("", __FUNCTION__, "stockPlaceNoFr  = [{0}]", stockPlaceNoFr);

			if (matNo.Trim() == "")
			{
				strcpy(s.msg, "没有传入材料号");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if (stockPlaceNoFr.Trim() == "")
			{
				strcpy(s.msg, "没有传入起始位置");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			//数据校验
			tophpmms1["MAT_NO"] = matNo;

			if (tophpmms1.QueryCount("MAT_NO") < 1)
			{
				sprintf(s.msg, "材料【%s】不存在。", (const char*)tophpmms1["MAT_NO"]);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			twma7["MAT_NO"] = matNo;
			if (!twma7.Query("MAT_NO"))
			{
				sprintf(s.msg, "材料【%s】的吊车命令不存在。", (const char*)twma7["MAT_NO"]);
				throw CApplicationException(-1, s.msg, log.Location);
			}
			// 吊车命令状态    R:选定确认    S : 吊上     E : 卸下     C : 选定取消

			Log::Debug("", __FUNCTION__, "CRANE_INST_STATUS  = [{0}]", twma7["CRANE_INST_STATUS"].ToString().Trim());
			if (twma7["CRANE_INST_STATUS"].ToString().Trim() == "S") break;
			if (twma7["CRANE_INST_STATUS"].ToString().Trim() == "E")
			{
				sprintf(s.msg, "材料 [%s] 的当前吊车命令状态不允许吊起操作。", (const char*)matNo);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			twm04["STOCK_PLACE_NO"] = stockPlaceNoFr;
			if (!twm04.Query("STOCK_PLACE_NO"))
			{
				sprintf(s.msg, "吊起位置 [%s] 不存在。", (const char*)stockPlaceNoFr);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//校验上层材料

			//Log::Trace("", "", "sqlstr：{0}", sqlstr);
			//if (Db::QueryCDecimal(sqlstr) > 0)
			//{
			//	sprintf(s.msg, "材料 [%s] 上方有材料，无法吊起！", (const char*)matNo);
			//	throw CApplicationException(-1, s.msg, s.svc_name);
			//}

			twma7["MAT_NO"] = matNo;
			twma7["REC_REVISE_TIME"] = datetime;
			twma7["REC_REVISOR"] = s.userid;
			twma7["CRANE_INST_STATUS"] = "S";
			twma7.Update("CRANE_INST_STATUS,REC_REVISE_TIME,REC_REVISOR", "MAT_NO");

			//写吊车命令执行履历

			hwm00a7.Reset();
			//twma4.CopyFrom(twma1_old);
			//dtCraneCmdTrace.Rows[0].Merge(dtCraneCmd.Rows[0]);
			//hwm00a7.CopyFrom(twma7);
			//hwm00a7["REC_CREATOR"] = s.userid;
			//hwm00a7["REC_CREATE_TIME"] = dateTime;
			//hwm00a7["REC_REVISOR"] = " ";
			//hwm00a7["REC_REVISE_TIME"] = " ";
			//hwm00a7["SHIFT_GROUP"] = shiftGroup;
			//hwm00a7["SHIFT_NO"] = shiftNo;
			//hwm00a7["CLIENT_IP"] = s.fore_ip;
			//hwm00a7["SVC_NAME"] = s.svc_name;
			//hwm00a7["REMARK"] = "Lift up";

			//hwm00a7.Insert();
			
			Log::Trace("", __FUNCTION__, "matNo=[{0}]", matNo);

			if (twma7["MAT_NO"])
				doFlag = f_wm00_move_to_null(matNo, "1", conn);  //1:lift up,  2:lift down
			if (doFlag < 0)
			{
				sprintf(s.msg, "[%s] Error occured when clear the from-position", (const char*)matNo);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "Database processing error. sqlcode=[{0}]." /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		Log::Trace("", __FUNCTION__, "数据库SQL出错信息	= [{0}]", str);
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
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
