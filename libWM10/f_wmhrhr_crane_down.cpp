/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:      张凌辉
Version:     1.1.1
Date:        2016-10-10 14:05:08
Description: 吊车命令卸下函数
**************************************************/

//#include "WM_Utility.h"
#include "stdafx.h"

//调用外部函数
BM2_FUNCTION_IMPORT
//int f_wmcrcr_stock_in(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_wmhrhr_stock_in(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

BM2_FUNCTION_IMPORT
//int f_wmcrcr_stock_out(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_wmhrhr_stock_out(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

BM2_FUNCTION_IMPORT
//int f_wmcrcr_stock_move(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_wmhrhr_stock_move(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);


BM2_FUNCTION_EXPORT
int f_wmcrcr_crane_down(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd_inq(conn);

	//应用变量
	int cmdSeq = 0;
	int tj_count = 0;
	CDecimal layerNoFr = 0;

	CString matNo = " ";
	CString matNo_pre = " ";
	CString sqlWhere = " ";
	CString shiftGroup = " ";
	CString shiftNo = " ";
	CString stockNoFr = " ";
	CString stockNoTo = " ";
	CString stockPlaceNoTo = " ";
	CString tcNo = " ";
	CString hall_no = " ";
	CString stockPlaceType = " ";
	CString devDiv = " ";
	CString stockOperOrder = " ";
	CString enExDiv = " ";
	CString manageAccu = " ";
	CString div = " ";
	CString unit_code = " ";
	CString layerno = " ";
	CString infur_slab_wt = " ";
	CString logic_stock = " ";
	CDecimal mat_act_x = 0;
	CDecimal mat_act_y = 0;
	CDecimal mat_act_z = 0;
	CString op_mode = "";

	CString v_prod_shift_no = "";
	CString v_prod_shift_group = "";
	CString v_prod_date = "";
	int seq = 0;

	CString dateTime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	//定义行车命令数据表
	CDataTable dtCraneCmd;

	//定义垛位数据表
	CDataTable dtStockPlace;

	//定义行车命令履历数据表
	CDataTable dtCraneCmdTrace;

	//定义材料数据表
	CDataTable dtMat;

	//定义电文数据表
	CDataTable dtMessage;

	//定义修改字段表
	CDataTable dtUpdItem;

	//定义表实体对象
	CModel hwm00a7 = CModel("HWM00A7");
	CModel twma7 = CModel("TWMA7");
	CModel twma0 = CModel("TWMA0");
	CModel twma2 = CModel("TWMA2");
	CModel twm04 = CModel("TWM04");
	CModel tophpmms1 = CModel("TWMA1");
	CModel twm05 = CModel("TWM05");
	CModel tmmcr01 = CModel("TMMCR01");
	CModel tmmhr01 = CModel("TMMHR01");

	//调用仓库入库主函数
	EIClass bcls_stock_in;
	bcls_stock_in.Tables[0].set_TableName("WM_STOCK");
	bcls_stock_in.Tables[0].Columns.Add(twma0);
	bcls_stock_in.Tables[0].Columns.Add(twma2);
	bcls_stock_in.Tables[0].Columns.Add(DT_STRING, "OP_MODE");
	bcls_stock_in.Tables[0].Rows.Clear();


	//调用仓库入库主函数
	EIClass bcls_stock_out;
	bcls_stock_out.Tables[0].set_TableName("WM_STOCK");
	bcls_stock_out.Tables[0].Columns.Add(twma0);
	bcls_stock_out.Tables[0].Columns.Add(twma2);
	bcls_stock_out.Tables[0].Columns.Add(DT_STRING, "OP_MODE");
	bcls_stock_out.Tables[0].Rows.Clear();

	//调用仓库入库主函数
	EIClass bcls_stock_move;
	bcls_stock_move.Tables[0].set_TableName("WM_STOCK");
	bcls_stock_move.Tables[0].Columns.Add(twma0);
	bcls_stock_move.Tables[0].Columns.Add(twma2);
	bcls_stock_move.Tables[0].Columns.Add(DT_STRING, "OP_MODE");
	bcls_stock_move.Tables[0].Rows.Clear();

	//调用仓库入库主函数
	EIClass bcls_stock_move_1;
	bcls_stock_move_1.Tables[0].set_TableName("WM_STOCK");
	bcls_stock_move_1.Tables[0].Columns.Add(twma0);
	bcls_stock_move_1.Tables[0].Columns.Add(twma2);
	bcls_stock_move_1.Tables[0].Columns.Add(DT_STRING, "OP_MODE");
	bcls_stock_move_1.Tables[0].Rows.Clear();

	try
	{
		//判断是否存在指定块
		if (bcls_rec->Tables.IndexOf("WM00_DOWN") < 0 ||
			bcls_rec->Tables["WM00_DOWN"].Rows.get_Count() == 0)
		{
			sprintf(s.msg, "函数f_wmhpsm_crane_up中找不到接收块名[WM00_DOWN]或值为空");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		//获取班组班次信息
		f_epep_get_shift_group("CR", dateTime, shiftNo, shiftGroup, conn);

		//循环获取传入行车命令块数据
		for (int i = 0; i < bcls_rec->Tables["WM00_DOWN"].Rows.get_Count(); i++)
		{
			matNo = bcls_rec->Tables["WM00_DOWN"].Rows[i]["MAT_NO"];
			stockPlaceNoTo = bcls_rec->Tables["WM00_DOWN"].Rows[i]["STOCK_PLACE_NO"];
			tmmcr01["MAT_NO"] = matNo;
			tmmhr01["MAT_NO"] = matNo;

			mat_act_x = 0;
			mat_act_y = 0;
			mat_act_z = 0;
			op_mode = "2";
			if (bcls_rec->Tables["WM00_DOWN"].Columns.Contains("MAT_ACT_X"))
			{
				mat_act_x = bcls_rec->Tables["WM00_DOWN"].Rows[i]["MAT_ACT_X"].ToDecimal();
			}
			if (bcls_rec->Tables["WM00_DOWN"].Columns.Contains("MAT_ACT_Y"))
			{
				mat_act_y = bcls_rec->Tables["WM00_DOWN"].Rows[i]["MAT_ACT_Y"].ToDecimal();
			}
			if (bcls_rec->Tables["WM00_DOWN"].Columns.Contains("MAT_ACT_Z"))
			{
				mat_act_z = bcls_rec->Tables["WM00_DOWN"].Rows[i]["MAT_ACT_Z"].ToDecimal();
			}
			if (bcls_rec->Tables["WM00_DOWN"].Columns.Contains("OP_MODE"))
			{
				op_mode = bcls_rec->Tables["WM00_DOWN"].Rows[i]["OP_MODE"].ToString().Trim();
				Log::Debug("", __FUNCTION__, "11111op_mode  = [{0}]", op_mode);
			}

			Log::Debug("", __FUNCTION__, "MAT_NO  = [{0}]", matNo);
			Log::Debug("", __FUNCTION__, "stockPlaceNoTo  = [{0}]", stockPlaceNoTo);
			Log::Debug("", __FUNCTION__, "mat_act_x  = [{0}]", mat_act_x);
			Log::Debug("", __FUNCTION__, "mat_act_y  = [{0}]", mat_act_y);
			Log::Debug("", __FUNCTION__, "mat_act_z  = [{0}]", mat_act_z);
			Log::Debug("", __FUNCTION__, "op_mode  = [{0}]", op_mode);

			if (matNo.Trim() == "")
			{
				strcpy(s.msg, "没有传入材料号");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if (stockPlaceNoTo.Trim() == "")
			{
				strcpy(s.msg, "没有传入目标垛位");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			twma7["MAT_NO"] = matNo;
			if (twma7.QueryCount("MAT_NO") < 1)
			{
				sprintf(s.msg, "材料【%s】的吊车命令不存在。", (const char*)matNo);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			twma7["MAT_NO"] = matNo;
			twma7.Query("MAT_NO");

			//取行车命令相关数据

			stockOperOrder = twma7["STOCK_OPER_ORDER"].ToString();
			Log::Debug("", __FUNCTION__, "stockPlaceNoTo.GetLength  = [{0}]", stockPlaceNoTo.Trim().GetLength());
			Log::Debug("", __FUNCTION__, "stockPlaceNoFr.Trim().Substring(0, 1) = [{0}]", stockPlaceNoTo.Trim().Substring(0, 1));
			if (stockPlaceNoTo.Trim().GetLength() == 5 && stockPlaceNoTo.Trim().Substring(0, 1) == "B")
			{
				layerno = stockPlaceNoTo.Trim().Substring(4, 1);
				stockPlaceNoTo = stockPlaceNoTo.Trim().Substring(0, 4);
			}
			Log::Debug("", __FUNCTION__, "layerno  = [{0}]", layerno);
			Log::Debug("", __FUNCTION__, "stockPlaceNoTo2222  = [{0}]", stockPlaceNoTo);


			twm04["STOCK_PLACE_NO"] = stockPlaceNoTo;
			Log::Trace("", __FUNCTION__, "stockPlaceNoTo33333= [{0}]", twm04["STOCK_PLACE_NO"].ToString());
			if (!twm04.Query("STOCK_PLACE_NO"))
			{
				sprintf(s.msg, "卸下位置 [%s] 不存在。", (const char*)stockPlaceNoTo);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			logic_stock = twm04["LOGIC_STOCK_NO"];
			stockPlaceType = twm04["STOCK_PLACE_TYPE"].ToString().Trim();
			devDiv = twm04["DEV_DIV"].ToString().Trim();
			stockNoTo = twm04["STOCK_NO"].ToString().Trim();
			enExDiv = twm04["ENTRANCE_EXIT_DIV"].ToString().Trim();
			manageAccu = twm04["MANAGE_ACCU"].ToString().Trim();
			hall_no = twm04["HALL_NO"].ToString().Trim();

			//if (stockPlaceType == "D" && devDiv == "3") //卸下位置是台车，判断台车当前位置
			//{
			//	twm05["TR_NO"] = twm04["TR_NO"].ToString();
			//	twm05.Query("TR_NO");
			//	Log::Debug("", __FUNCTION__, "twm05.HALL_NO  = [{0}]", (const char*)twm05["HALL_NO"].ToString());
			//	Log::Debug("", __FUNCTION__, "twm04.HALL_NO  = [{0}]", (const char*)twm04["HALL_NO"].ToString());

			//	if (twm05["HALL_NO"].ToString().Trim() != hall_no)
			//	{
			//		Log::Debug("", __FUNCTION__, "111twm05.HALL_NO  = [{0}]", (const char*)twm05["HALL_NO"].ToString());
			//		Log::Debug("", __FUNCTION__, "111twm04.HALL_NO  = [{0}]", (const char*)twm04["HALL_NO"].ToString());
			//		sprintf(s.msg, "台车[%s]当前不在[%s]跨。", (const char*)twm04["TR_NO"].ToString(), (const char*)twm04["HALL_NO"].ToString());
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}
			//}

			//调用仓库出库主函数
			bcls_stock_out.Tables["WM_STOCK"].Rows.Add();
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["MAT_NO"] = matNo;
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["STOCK_NO"] = stockNoTo;
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["STOCK_PLACE_NO"] = stockPlaceNoTo;
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["ROWNO"] = " ";
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["COLUMN_NO"] = " ";
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["LAYERNO"] = 0;
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["STOCK_PLACE_POSITION"] = " ";
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["MAT_ACT_X"] = mat_act_x;
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["MAT_ACT_Y"] = mat_act_y;
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["MAT_ACT_Z"] = mat_act_z;
			bcls_stock_out.Tables["WM_STOCK"].Rows[i]["OP_MODE"] = op_mode;

			//调用仓库倒垛主函数
			bcls_stock_move.Tables["WM_STOCK"].Rows.Add();
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["MAT_NO"] = matNo;
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["STOCK_NO"] = stockNoTo;
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["STOCK_PLACE_NO"] = stockPlaceNoTo;
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["ROWNO"] = " ";
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["COLUMN_NO"] = " ";
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["LAYERNO"] = 0;
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["STOCK_PLACE_POSITION"] = " ";
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["MAT_ACT_X"] = mat_act_x;
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["MAT_ACT_Y"] = mat_act_y;
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["MAT_ACT_Z"] = mat_act_z;
			bcls_stock_move.Tables["WM_STOCK"].Rows[i]["OP_MODE"] = op_mode;

			//调用仓库入库主函数
			bcls_stock_in.Tables["WM_STOCK"].Rows.Add();
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["MAT_NO"] = matNo;
			//bcls_stock_in.Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER"] = twma0["STOCK_OPER_ORDER"];
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER_DIV"] = " ";
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["STOCK_NO"] = stockNoTo;
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["STOCK_PLACE_NO"] = stockPlaceNoTo;
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["ROWNO"] = " ";
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["COLUMN_NO"] = " ";
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["LAYERNO"] = 0;
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["STOCK_PLACE_POSITION"] = " ";
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["MAT_ACT_X"] = mat_act_x;
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["MAT_ACT_Y"] = mat_act_y;
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["MAT_ACT_Z"] = mat_act_z;
			bcls_stock_in.Tables["WM_STOCK"].Rows[i]["OP_MODE"] = op_mode;



			if (stockPlaceType.Trim() == "D" && devDiv.Trim() != "3")
			{
				//not into yard or transfer car
				if (stockPlaceType.Trim() == "D" && enExDiv.Trim() == "1")
				{
					// production feeding
					bcls_stock_out.Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER"] = "2B";
					if (tmmcr01.Query("MAT_NO"))
					{
					//	doFlag = f_wmcrcr_stock_out(&bcls_stock_out, bcls_ret, conn);
					//	if (doFlag != 0)
					//	{
					//		throw CApplicationException(-1, s.msg, log.Location);
					//	}
					}
					else
					{
						doFlag = f_wmhrhr_stock_out(&bcls_stock_out, bcls_ret, conn);
						if (doFlag != 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
				}
				else
				{
					if (stockOperOrder.Trim().Substring(0, 1) == "2")
					{
						// call stock in function
						bcls_stock_out.Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER"] = stockOperOrder;

						if (tmmcr01.Query("MAT_NO"))
						{
						//	doFlag = f_wmcrcr_stock_out(&bcls_stock_out, bcls_ret, conn);
						//	if (doFlag != 0)
						//	{
						//		throw CApplicationException(-1, s.msg, log.Location);
						//	}
						}
						else
						{
							doFlag = f_wmhrhr_stock_out(&bcls_stock_out, bcls_ret, conn);
							if (doFlag != 0)
							{
								throw CApplicationException(-1, s.msg, log.Location);
							}
						}


					}
					else
					{
						// call stock move function
						bcls_stock_move.Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER"] = "30";

						if (tmmcr01.Query("MAT_NO"))
						{
						//	doflag = f_wmcrcr_stock_move(&bcls_stock_move, bcls_ret, conn);
						//	if (doflag != 0)
						//	{
						//		throw capplicationexception(-1, s.msg, log.location);
						//	}
						}
						else
						{
							doFlag = f_wmhrhr_stock_move(&bcls_stock_move, bcls_ret, conn);
							if (doFlag != 0)
							{
								throw CApplicationException(-1, s.msg, log.Location);
							}
						}
					}
				}
			}
			else
			{
				// lift down in yard or on transfer car
				if (stockOperOrder.Trim().Substring(0, 1) == "1")
				{
					// call stock in function
					bcls_stock_in.Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER"] = stockOperOrder;

					if (tmmcr01.Query("MAT_NO"))
					{
					//	doFlag = f_wmcrcr_stock_in(&bcls_stock_in, bcls_ret, conn);
					//	if (doFlag != 0)
					//	{
					//		throw CApplicationException(-1, s.msg, log.Location);
					//	}
					}
					else
					{
						doFlag = f_wmhrhr_stock_in(&bcls_stock_in, bcls_ret, conn);
						if (doFlag != 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}


				}
				else
				{
					// call stock move function
					bcls_stock_move.Tables["WM_STOCK"].Rows[i]["STOCK_OPER_ORDER"] = "30";

					if (tmmcr01.Query("MAT_NO"))
					{
					//	doFlag = f_wmcrcr_stock_move(&bcls_stock_move, bcls_ret, conn);
					//	if (doFlag != 0)
					//	{
					//		throw CApplicationException(-1, s.msg, log.Location);
					//	}
					}
					else
					{
						doFlag = f_wmhrhr_stock_move(&bcls_stock_move, bcls_ret, conn);
						if (doFlag != 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
				}
			}
		
			hwm00a7.CopyFrom(twma7);
			hwm00a7["SHIFT_GROUP"] = shiftGroup;
			hwm00a7["SHIFT_NO"] = shiftNo;
			hwm00a7["CLIENT_IP"] = s.fore_ip;
			hwm00a7["SVC_NAME"] = s.svc_name;
			hwm00a7["REMARK"] = "Lift down";

			hwm00a7.Insert();
			
}
			
		if (bcls_stock_move_1.Tables["WM_STOCK"].Rows.get_Count() > 0)
		{
			if (tmmcr01.Query("MAT_NO"))
			{
			//	doFlag = f_wmcrcr_stock_move(&bcls_stock_move, bcls_ret, conn);
			//	if (doFlag != 0)
			//	{
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}
			}
			else
			{
				doFlag = f_wmhrhr_stock_move(&bcls_stock_move, bcls_ret, conn);
				if (doFlag != 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
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