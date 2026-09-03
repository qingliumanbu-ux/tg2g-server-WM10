/* **************************************************************************
*	Copyright (c) Baosight Corporation 2008 . All Rights Reserved.
*  	BM2PES 宝信生产执行系统
*****************************************************************************
*  程序名称			: f_wm00_stock_udpate
*  程序描述			: 更新库位跟踪表
*  备注说明			:
*  修改历史			:
*  		2020-09-19 仓库产品化			(ADD)程序建立
*			... ...
* **************************************************************************** */
/* C/C++ 的标准头文件部分 */
#include "stdafx.h"		// 框架头，不可删除 

//垛位最大高度、重量修正
int f_wm00_pileinfocal(CString stock_no, CString stock_place_no, EIClass * bcls_ret, CDbConnection * conn);

//校验库位规则
int f_wm00_stock_chk(CString stockPlaceNo, CDbConnection *conn);

//记录操作日志
int f_wm_stock_log(EIClass * bcls_rec, EIClass * bcls_ret, CString table_name, CDbConnection * conn);

//调用出入库函数，产品化不做任何操作，具体逻辑项目组实现
int f_wm_stock_in(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn);
int f_wm_stock_out(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn);
int f_wm_stock_move(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn);


BM2_FUNCTION_EXPORT
int f_wm_stock_update(EIClass *bcls_rec, EIClass *bcls_ret, CString table_name, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* ***** 程序变量 ***** */
	int doFlag = 0;
	int auto_down_flag = 1;   //中间抽取后自动计算标志：1--计算，0--不计算
	CString c_mat_no = "";
	CString c_stock_place_no = "";
	CString c_stock_oper_order = "";
	CString v_datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal c_layerno = 0;
	CDataTable t_up_mat;
	CModel twma2_old=CModel("TWMA2");
	CModel twma2_new = CModel("TWMA2");
	CModel twm04 = CModel("TWM04");
	CModel twma4 = CModel("TWMA4");
	CModel twma1 = CModel(table_name);
	/* ***** 数据库SQL操作字符串 ***** */
	CString	sqlstr = " ";

	//记录履历
	EIClass bcls_rec_stock_log;
	bcls_rec_stock_log.Tables[0].set_TableName("WM_STOCK_LOG");
	bcls_rec_stock_log.Tables[0].Rows.Clear();

	try
	{
		c_stock_oper_order = bcls_rec->Tables["WM_STOCK"].Rows[0]["STOCK_OPER_ORDER"].ToString();

		if (c_stock_oper_order.Trim() == ""){
			sprintf(s.msg, "函数f_wm_stock_update中STOCK_OPER_ORDER不能为空");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (c_stock_oper_order[0] == '1')
		{
			//调用入库函数
			doFlag = f_wm_stock_in(bcls_rec, bcls_ret, table_name, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		if (c_stock_oper_order[0] == '2')
		{
			//调用出库函数
			doFlag = f_wm_stock_out(bcls_rec, bcls_ret, table_name, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		if (c_stock_oper_order[0] == '3')
		{
			//调用倒跺函数
			doFlag = f_wm_stock_move(bcls_rec, bcls_ret, table_name, conn);
			if (doFlag != 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		for (int i = 0; i < bcls_rec->Tables["WM_STOCK"].Rows.get_Count(); ++i)
		{			
			c_mat_no = bcls_rec->Tables["WM_STOCK"].Rows[i]["MAT_NO"].ToString();		
			c_stock_oper_order = bcls_rec->Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER"].ToString();
			if (bcls_rec->Tables["WM_STOCK"].Columns.Contains("LAYERNO"))
				c_layerno = bcls_rec->Tables["WM_STOCK"].Rows[i]["LAYERNO"].ToDecimal();

			if (bcls_rec->Tables["WM_STOCK"].Columns.Contains("STOCK_PLACE_NO"))
				c_stock_place_no = bcls_rec->Tables["WM_STOCK"].Rows[i]["STOCK_PLACE_NO"].ToString();

			Log::Trace("", __FUNCTION__, "传入参数MAT_NO[{0}]", c_mat_no);
			Log::Trace("", __FUNCTION__, "传入参数LAYERNO[{0}]", c_layerno);
			Log::Trace("", __FUNCTION__, "传入参数STOCK_PLACE_NO[{0}]", c_stock_place_no);

			twma1["MAT_NO"] = c_mat_no;
			twma1.Query("MAT_NO");

			/*一、-----处理老库位-----*/
			twma2_old.Reset();
			twma2_old["MAT_NO"] = c_mat_no;
			if (twma2_old.Query("MAT_NO"))
			{
				Log::Trace("", __FUNCTION__, "-----处理老库位-----");
				//删除老库位上材料信息
				twma2_old.Delete("MAT_NO");

				twm04.Reset();
				twm04["STOCK_PLACE_NO"] = twma2_old["STOCK_PLACE_NO"];
				twm04["STOCK_NO"] = twma2_old["STOCK_NO"];				
				if (twm04.Query("STOCK_PLACE_NO,STOCK_NO"))
				{
					/*如果库位管理方式是：MANAGE_ACCU "1" ，且需要自动下落，
					则将该垛位上被移走材料上面的层号 - 1*/
					if (twm04["MANAGE_ACCU"].ToString().Trim() == "1" && auto_down_flag==1)
					{
						t_up_mat.Clear();
						sqlstr = "select a.mat_no,a.layerno from twma2 a where "
							" stock_no = '" + twm04["STOCK_NO"].ToString().Trim() + "'"
							" and stock_place_no = '" + twm04["STOCK_PLACE_NO"].ToString().Trim() + "' "
							" and layerno > " + twma2_old["LAYERNO"].ToString().Trim() + " "
							" order by layerno asc";
						Db::QueryTable(sqlstr,t_up_mat);
						for (int j = 0; j < t_up_mat.Rows.get_Count();++j)
						{
							sqlstr = "update twma2 set LAYERNO=" + (twma2_old["LAYERNO"].ToDecimal() + j).ToString() + " where mat_no='" + t_up_mat.Rows[j]["MAT_NO"].ToString() + "'";
							Db::Execute(sqlstr);
						}					
					}

					/*计算老库位的状态*/ 
					doFlag = f_wm00_pileinfocal(twma2_old["STOCK_NO"].ToString(),twma2_old["STOCK_PLACE_NO"].ToString(), bcls_ret, conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				else
				{
					sprintf(s.msg, "函数f_wm_stock_update中库位号[" + twma2_old["STOCK_PLACE_NO"].ToString() + "]不存在");
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			/*二、-----处理新库位-----*/
			twma2_new.Reset();
			twma2_new["MAT_NO"] = c_mat_no;
			twma2_new["STOCK_PLACE_NO"] = c_stock_place_no;
			if (c_stock_place_no.Trim() != "")
			{
				twm04.Reset();
				twm04["STOCK_PLACE_NO"] = c_stock_place_no;
				if (!twm04.Query("STOCK_PLACE_NO"))
				{
					sprintf(s.msg, "函数f_wm_stock_update中库位号[" + twm04["STOCK_PLACE_NO"].ToString() + "]不存在");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				twma2_new["REC_CREATOR"] = twma2_old["REC_CREATOR"];
				twma2_new["REC_CREATE_TIME"] = twma2_old["REC_CREATE_TIME"];
				twma2_new["REC_REVISOR"] = s.userid;
				twma2_new["REC_REVISE_TIME"] = v_datetime;
				twma2_new["STOCK_NO"] = twm04["STOCK_NO"];
				twma2_new["STOCK_OPER_TIME"] = v_datetime;

				//计算层号
				if (c_layerno==0)
				{
					if (twm04["MANAGE_ACCU"].ToString().Trim() == "1"){
						c_layerno = Db::QueryCDecimal("SELECT COUNT(1) FROM TWMA2 WHERE STOCK_PLACE_NO='" + c_stock_place_no + "'") + 1;
					}
					else c_layerno = 1;
					
				}
				twma2_new["LAYERNO"] = c_layerno;
				twma2_new.TrimOrBlank();
				twma2_new.Insert();

				///校验库位是否超限
				doFlag = f_wm00_stock_chk(twma2_new["STOCK_PLACE_NO"].ToString(), conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
				/*计算新库位的状态*/ 
				doFlag = f_wm00_pileinfocal(twma2_new["STOCK_NO"].ToString(), twma2_new["STOCK_PLACE_NO"].ToString(), bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}

			//13. 写A4
			twma4.CopyFrom(twma1);
			twma4["STOCK_OPER_ORDER"] = c_stock_oper_order;
			twma4["STOCK_NO"] = twma2_new["STOCK_NO"];
			twma4["STOCK_PLACE_NO"] = twma2_new["STOCK_PLACE_NO"];
			twma4["LAYERNO"] = twma2_new["LAYERNO"];
			twma4["FROM_STOCK_NO"] = twma2_old["STOCK_NO"];
			twma4["FROM_STOCK_PLACE_NO"] = twma2_old["STOCK_PLACE_NO"];
			twma4.MergeTo(bcls_rec_stock_log.Tables["WM_STOCK_LOG"], false);
		}

		//调用履历函数
		doFlag = f_wm_stock_log(&bcls_rec_stock_log, bcls_ret, table_name, conn);
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
