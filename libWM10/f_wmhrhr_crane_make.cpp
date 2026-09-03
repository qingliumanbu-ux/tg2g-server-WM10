/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:      金权
Version:     1.1.1
Date:        2016-12-13 10:35:08
Description: 吊车命令做成函数
**************************************************/

//#include "WM_Utility.h"
//#include "h_wms0_pub.h"
//#include "twma7.h"
#include "stdafx.h"

BM2_FUNCTION_IMPORT
int f_wmhr_check_upmat(CString mat_no, EIClass * bcls_ret, CDbConnection * conn);		                     //检验钢卷上层是否需要倒跺
int f_wm00_pileinfocal(CString stock_no, CString stock_place_no, EIClass * bcls_ret, CDbConnection * conn);  //垛位最大高度、重量修正


BM2_FUNCTION_EXPORT
int f_wmhr_CraneCmd_C_Make(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	int Flag = 0;
	CString sqlstr = " ";
	CString sqla = " ";
	//应用变量
	int cmdSeq = 0;
	CString matNo = " ";
	CString dateTime = " ";
	CString sqlWhere = " ";
	CString stockOperOrder = " ";
	CString crane_inst_code = " ";
	CString v_table_name = "";

	CDecimal cmdGrpNo = 0;
	CDecimal seqno = 0;
	CDecimal v_count = 0;

	//定义行车命令数据表
	CDataTable dtCraneCmd;

	//定义材料数据表
	CDataTable dtMat;

	//数据库操作类定义
	CDbCommand cmd_inq(conn);

	/* 实体类定义 */
	//CTWMA7 twma7(conn);
	//CTWMA7 twma7_in(conn);
	CModel twma7 = CModel("TWMA7");
	CModel twma7_in = CModel("TWMA7");



	try
	{
		//判断是否存在指定块
		if (bcls_rec->Tables.IndexOf("WM00_CMD") < 0 ||
			bcls_rec->Tables["WM00_CMD"].Rows.get_Count() == 0)
		{
			strcpy(s.msg, "Incoming data block WM00_CMD is not exist.");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		//设置dtCraneCmd列名
		//WM_Utility::SetDataTableColName("TWMA7", dtCraneCmd, conn);


		//取系统时间
		dateTime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		//循环获取传入行车命令块数据
		for (int i = 0; i < bcls_rec->Tables["WM00_CMD"].Rows.get_Count(); i++)
		{
			seqno = 0;
			matNo = bcls_rec->Tables["WM00_CMD"].Rows[i]["MAT_NO"];
			Log::Trace("", __FUNCTION__, "传入材料号:{0}", matNo);
			if (matNo.Trim() == "")
			{
				strcpy(s.msg, "Incoming material no. is null");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}


			sqlstr = " SELECT COUNT(1) FROM TMMSM01"
				" WHERE MAT_NO = @mat_no";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("mat_no", matNo);
			v_count = cmd_inq.ExecuteScalar();
			if (v_count == 1)
			{
				v_table_name = "TMMSM01";
			}
			else
			{
				sqlstr = " SELECT COUNT(1) FROM TMMHR01"
					" WHERE MAT_NO = @mat_no";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("mat_no", matNo);
				v_count = cmd_inq.ExecuteScalar();
				if (v_count == 1)
				{
					v_table_name = "TMMHR01";
				}
				else
				{
					sprintf(s.msg, "板坯/热卷主档找不到材料号.");
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}

			stockOperOrder = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_OPER_ORDER"];
			Log::Trace("", __FUNCTION__, "传入库业务类型:{0}", stockOperOrder);
			if (stockOperOrder.Trim() == "")
			{
				strcpy(s.msg, "Incoming movement type is null");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			crane_inst_code = stockOperOrder.Substring(0, 1);

			if (crane_inst_code == "1")
			{
				Log::Trace("", __FUNCTION__, "##### 入库命令 #####");
				twma7.Reset();
				twma7["MAT_NO"] = matNo;
				if (twma7.Query("MAT_NO"))
				{
					if (twma7["CRANE_INST_STATUS"].ToString() == "0")
					{
						Log::Trace("", __FUNCTION__, "材料存在命令删除");
						seqno = twma7["CMD_SEQ"].ToDecimal();
						twma7.Delete();
						doFlag = f_wm00_pileinfocal(twma7["STOCK_NO"].ToString(),
							twma7["STOCK_PLACE_NO_TO"].ToString(), bcls_ret, conn);
						if (doFlag != 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					else
					{
						Log::Trace("", __FUNCTION__, "材料存在命令不能删除，跳过");
						continue;
					}
				}
				if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_NO_TO") ||
					bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_TO"].ToString().Trim() == "")
				{
					sprintf(s.msg, "Material[%s]has not stock no", (const char*)matNo);/*材料号[%s]没有指定入库库区*/
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				//新增行车命令数据表行	
				twma7_in.Reset();
				twma7_in["REC_CREATOR"] = s.userid;
				twma7_in["REC_CREATE_TIME"] = dateTime;
				twma7_in["MAT_NO"] = matNo;
				twma7_in["STOCK_OPER_ORDER"] = stockOperOrder;
				twma7_in["MOVE_TYPE"] = stockOperOrder;
				twma7_in["CRANE_INST_STATUS"] = "0";
				twma7_in["CRANE_INST_CODE"] = crane_inst_code;
				twma7_in["MAT_NO"] = matNo;

				sqlstr = "SELECT mat_no,mat_kind,mat_shape_flag,mat_act_thick,mat_act_width,mat_act_len,mat_act_wt"
					" FROM " + v_table_name + " WHERE mat_no = '" + matNo + "'";

				/*if (!WM_Utility::QueryData(sqlstr, dtMat, conn))
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}*/
			     Db::QueryTable(sqlstr, dtMat);
				if (dtMat.Rows.get_Count() == 0)
				{
					sprintf(s.msg, "Materials no.[%s]is not exist.", (const char*)matNo);
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				Log::Trace("", __FUNCTION__, "写材料源库位信息");
				twma7_in["STOCK_NO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_TO"];
				twma7_in["STOCK_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_TO"];
				twma7_in["STOCK_NO_FROM"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FROM"];
				if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("HALL_NO_FROM") &&
					bcls_rec->Tables["WM00_CMD"].Rows[i]["HALL_NO_FROM"].ToString().Trim() != "")
				{
					twma7_in["HALL_NO_FR"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["HALL_NO_FROM"];
				}
				else
				{
					twma7_in["HALL_NO_FR"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FROM"];
				}
				twma7_in["STOCK_PLACE_NO_FROM"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_FROM"];
				twma7_in["YARD_LAYER_FROM"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["YARD_LAYER_FROM"];
				if (seqno == 0)
				{
					sqlstr = "values nextval for seqTest";
					twma7_in["CMD_SEQ"] = atoi(Db::QueryCString(sqlstr));
					//twma7_in["CMD_SEQ"] = Db::QueryCDecimal(sqla);
					//twma7_in["CMD_SEQ"] = atoi(WM_Utility::GetSeqence("seqTest", conn));
				}
				else
				{
					twma7_in["CMD_SEQ"] = seqno;
				}
				if (bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_TO"].ToString().Trim() == "")
				{
					sqlstr = "  SELECT a.rowno rownum,a.stock_no,                    "
						"  a.STOCK_PLACE_NO,                     "
						"  a.PILE_MAT_NUM_ACT MAT_NUM,           "
						"  a.PILE_MAT_WT_ACT MAT_ACT_WT,          "
						"  a.rowno,a.column_no,a.LAYERNO              "
						"  ,a.CURRENT_LAYERNO,a.CURRENT_LAYER_NUM  "
						"  FROM TWM04   a, TWM01 b                       "
						" WHERE 1 = 1                          "
						"  AND a.STOCK_PLACE_TYPE != 'D' "//STOCK_PLACE_TYPE库位类型是0
						"  AND a.STOCK_NO = b.STOCK_NO "
						"  AND ((a.MANAGE_ACCU != '5' and a.STOCK_STATUS in ('0', '3', '5') ) "
						"  OR (a.MANAGE_ACCU = '5' and a.STOCK_STATUS = '0' ))";
					//Log::Trace("", __FUNCTION__, "未指定入库库位,调用库位推荐");
					//// 初始化 库位推荐用 输入/输出块
					//EIClass in, out;
					//f_wms_auto_init(&in, &out, conn);

					//// 设置库位推荐输入信息
					//// 设置输入参数块
					//in.Tables[WMS_BLK_IN_PARAMS].Rows.Clear();
					//in.Tables[WMS_BLK_IN_PARAMS].Rows.Add();
					//CDataTable& blkParams = in.Tables[WMS_BLK_IN_PARAMS];

					//// 入出库区分(I：入库、O：出库、M：倒垛)			
					//blkParams.Rows[0][WMS_COL_JOB_IO_DIV] = "I";

					//// 库区作业顺序是否可调(N:不可调、Y:可调整)
					//blkParams.Rows[0][WMS_COL_JOB_ADJUST_FLAG] = "N";

					//// 作业库区号(入库时：入库目标库区、 倒垛时：倒垛库区、 出库时：出库起始库)
					//blkParams.Rows[0][WMS_COL_JOB_STOCK_NO] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FROM"];

					//// 材料形状区分(P:板类、C:卷类)
					//if (dtMat.Rows[0]["MAT_SHAPE_FLAG"].ToString() == "3")
					//{
					//	blkParams.Rows[0][WMS_COL_JOB_MAT_SHAPE_DIV] = "C";
					//}
					//else
					//{
					//	blkParams.Rows[0][WMS_COL_JOB_MAT_SHAPE_DIV] = "P";
					//}
					//// 设置材料参数块（可设置多块材料）
					//int currRow = -1;
					//CDataTable& blkMats = in.Tables[WMS_BLK_IN_MATS];

					//++currRow;
					//blkMats.Rows.Add();
					//blkMats.Rows[currRow][WMS_COL_MAT_GRP_NO] = matNo;					// 材料组号（1个材料组可含有一个或多个材料。当含有多个材料时，这些材料将作为一个整体叠放在同一个垛位上【适用于钢板】。当只有一块材料时，通常 材料组号=材料号。）
					//blkMats.Rows[currRow][WMS_COL_MAT_NO] = matNo;						// 材料号
					//blkMats.Rows[currRow][WMS_COL_MAT_KIND] = " ";                      // 材料种类(HP:中厚板、CR：冷轧、HR：热轧 等等)【印度项目不用，传空格】
					//blkMats.Rows[currRow][WMS_COL_JOB_LARGE_DIV] = stockOperOrder;      // 库区作业大分类：库业务类型(代码WM10)
					//blkMats.Rows[currRow][WMS_COL_JOB_MIDDLE_DIV] = " ";                // 库区作业中分类：【印度项目不用，传空格】
					//if (stockOperOrder == "1B")
					//{
					//	blkMats.Rows[currRow][WMS_COL_FROM_STOCK_DEV_NO] = "C201";   	// 材料入库所用设备垛位。【倒垛、出库时，无需设置（传空格）】
					//}
					//else
					//{
					//	blkMats.Rows[currRow][WMS_COL_FROM_STOCK_DEV_NO] = " ";
					//}

					//// 材料入库所用设备垛位。【倒垛、出库时，无需设置（传空格）】
					////blkMats.Rows[currRow][WMS_COL_FROM_STOCK_DEV_NO] = twma7_in.STOCK_PLACE_NO_FROM;

					//Log::Trace("", __FUNCTION__, "---------------打印库位推荐传入块数据----------------");
					//Log::Trace("", __FUNCTION__, "WMS_BLK_IN_PARAMS：");
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_IO_DIV={0}", blkParams.Rows[0][WMS_COL_JOB_IO_DIV].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_ADJUST_FLAG={0}", blkParams.Rows[0][WMS_COL_JOB_ADJUST_FLAG].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_STOCK_NO={0}", blkParams.Rows[0][WMS_COL_JOB_STOCK_NO].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_MAT_SHAPE_DIV={0}", blkParams.Rows[0][WMS_COL_JOB_MAT_SHAPE_DIV].ToString());

					//Log::Trace("", __FUNCTION__, "WMS_BLK_IN_MATS：");
					//Log::Trace("", __FUNCTION__, "WMS_COL_MAT_GRP_NO={0}", blkMats.Rows[currRow][WMS_COL_MAT_GRP_NO].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_MAT_NO={0}", blkMats.Rows[currRow][WMS_COL_MAT_NO].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_MAT_KIND={0}", blkMats.Rows[currRow][WMS_COL_MAT_KIND].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_LARGE_DIV={0}", blkMats.Rows[currRow][WMS_COL_JOB_LARGE_DIV].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_MIDDLE_DIV={0}", blkMats.Rows[currRow][WMS_COL_JOB_MIDDLE_DIV].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_FROM_STOCK_DEV_NO={0}", blkMats.Rows[currRow][WMS_COL_FROM_STOCK_DEV_NO].ToString());

					////执行库位推荐
					//doFlag = f_wms_auto(&in, &out, conn);
					//if (doFlag == 0)
					//{
					//	WM_Utility::PrintLog("库位推荐成功");
					//	WM_Utility::PrintLog("打印推荐结果目标垛位块");
					//	WM_Utility::PrintDataTable(out.Tables[WMS_BLK_OUT_TARGETS]);
					//	twma7_in["YARD_LAYER_TO"] = out.Tables[WMS_BLK_OUT_TARGETS].Rows[0]["TO_STOCK_LAYER_NO"];
					//	twma7_in["STOCK_PLACE_NO_TO"] = out.Tables[WMS_BLK_OUT_TARGETS].Rows[0]["TO_STOCK_PLACE_NO"];
					//}
					//else
					//{
					//	WM_Utility::PrintLog("库位推荐失败");
					//}
				}
				else
				{
					Log::Trace("", __FUNCTION__, "指定入库库位");
					twma7_in["STOCK_PLACE_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_TO"];
					if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("YARD_LAYER_TO") ||
						bcls_rec->Tables["WM00_CMD"].Rows[i]["YARD_LAYER_TO"].ToString().Trim() == "")
					{
						twma7_in["YARD_LAYER_TO"] = 0;
					}
					else
					{
						twma7_in["YARD_LAYER_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["YARD_LAYER_TO"];;
					}
				}
				twma7_in.Insert();
				Log::Trace("", __FUNCTION__, "刷新库位状态");
				doFlag = f_wm00_pileinfocal(twma7_in["STOCK_NO"].ToString(),
					twma7_in["STOCK_PLACE_NO_TO"].ToString(), bcls_ret, conn);
				if (doFlag != 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

			}
			else
			{
				Log::Trace("", __FUNCTION__, "##### 非入库命令,判断材料是否存在命令 #####");
				twma7.Reset();
				twma7["MAT_NO"] = matNo;
				if (twma7.Query("MAT_NO"))
				{
					if (twma7["CRANE_INST_STATUS"].ToString() == "0")
					{
						Log::Trace("", __FUNCTION__, "材料存在命令删除");
						seqno = twma7["CMD_SEQ"].ToDecimal();
						twma7.Delete();
						Log::Trace("", __FUNCTION__, "刷新库位状态");
						doFlag = f_wm00_pileinfocal(twma7["STOCK_NO"].ToString(),
							twma7["STOCK_PLACE_NO_TO"].ToString(), bcls_ret, conn);
						if (doFlag != 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}
					else
					{
						Log::Trace("", __FUNCTION__, "材料存在命令不能删除");
						if (stockOperOrder == "32" || stockOperOrder == "2B" || stockOperOrder == "2E")
						{
							Log::Trace("", __FUNCTION__, "更新最终库业务类型");
							twma7["STOCK_OPER_ORDER_FIN"] = stockOperOrder;
							twma7.Update("STOCK_OPER_ORDER_FIN", "MAT_NO");
						}
						continue;
					}
				}
				else
				{
					Log::Trace("", __FUNCTION__, "材料不存在命令");
				}

				Log::Trace("", __FUNCTION__, "判断材料是否需要上层倒跺");
				Flag = f_wmhr_check_upmat(matNo, bcls_ret, conn);
				if (Flag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (Flag == 1 || bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_TO"].ToString().Trim() == "")
				{
					//Log::Trace("", __FUNCTION__, "材料需要上层倒跺或推荐库位");
					//// 初始化 库位推荐用 输入/输出块
					//EIClass in, out;
					//f_wms_auto_init(&in, &out, conn);

					//// 设置库位推荐输入信息
					//// 设置输入参数块

					//CDataTable& blkParams = in.Tables[WMS_BLK_IN_PARAMS];
					//in.Tables[WMS_BLK_IN_PARAMS].Rows.Clear();
					//in.Tables[WMS_BLK_IN_PARAMS].Rows.Add();
					//// 入出库区分(I：入库、O：出库、M：倒垛)
					//blkParams.Rows[0][WMS_COL_JOB_IO_DIV] = "M";

					//// 库区作业顺序是否可调(N:不可调、Y:可调整)
					//blkParams.Rows[0][WMS_COL_JOB_ADJUST_FLAG] = "N";

					//// 作业库区号(入库时：入库目标库区、 倒垛时：倒垛库区、 出库时：出库起始库)
					//blkParams.Rows[0][WMS_COL_JOB_STOCK_NO] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FROM"];

					//// 材料形状区分(P:板类、C:卷类)
					//blkParams.Rows[0][WMS_COL_JOB_MAT_SHAPE_DIV] = "C";
					//// 设置材料参数块（可设置多块材料）
					//int currRow = -1;
					//in.Tables[WMS_BLK_IN_MATS].Rows.Clear();

					//CDataTable& blkMats = in.Tables[WMS_BLK_IN_MATS];

					//++currRow;
					//blkMats.Rows.Add();
					//blkMats.Rows[currRow][WMS_COL_MAT_GRP_NO] = matNo;					// 材料组号（1个材料组可含有一个或多个材料。当含有多个材料时，这些材料将作为一个整体叠放在同一个垛位上【适用于钢板】。当只有一块材料时，通常 材料组号=材料号。）
					//blkMats.Rows[currRow][WMS_COL_MAT_NO] = matNo;						// 材料号
					//blkMats.Rows[currRow][WMS_COL_MAT_KIND] = " ";                      // 材料种类(HP:中厚板、CR：冷轧、HR：热轧 等等)【印度项目不用，传空格】
					//blkMats.Rows[currRow][WMS_COL_JOB_LARGE_DIV] = "31";                // 库区作业大分类：库业务类型(代码WM10)
					//blkMats.Rows[currRow][WMS_COL_JOB_MIDDLE_DIV] = " ";                // 库区作业中分类：【印度项目不用，传空格】

					//// 材料入库所用设备垛位。【倒垛、出库时，无需设置（传空格）】
					//blkMats.Rows[currRow][WMS_COL_FROM_STOCK_DEV_NO] = " ";

					//Log::Trace("", __FUNCTION__, "---------------打印库位推荐传入块数据----------------");
					//Log::Trace("", __FUNCTION__, "WMS_BLK_IN_PARAMS：");
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_IO_DIV={0}", blkParams.Rows[0][WMS_COL_JOB_IO_DIV].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_ADJUST_FLAG={0}", blkParams.Rows[0][WMS_COL_JOB_ADJUST_FLAG].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_STOCK_NO={0}", blkParams.Rows[0][WMS_COL_JOB_STOCK_NO].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_MAT_SHAPE_DIV={0}", blkParams.Rows[0][WMS_COL_JOB_MAT_SHAPE_DIV].ToString());

					//Log::Trace("", __FUNCTION__, "WMS_BLK_IN_MATS：");
					//Log::Trace("", __FUNCTION__, "WMS_COL_MAT_GRP_NO={0}", blkMats.Rows[currRow][WMS_COL_MAT_GRP_NO].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_MAT_NO={0}", blkMats.Rows[currRow][WMS_COL_MAT_NO].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_MAT_KIND={0}", blkMats.Rows[currRow][WMS_COL_MAT_KIND].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_LARGE_DIV={0}", blkMats.Rows[currRow][WMS_COL_JOB_LARGE_DIV].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_JOB_MIDDLE_DIV={0}", blkMats.Rows[currRow][WMS_COL_JOB_MIDDLE_DIV].ToString());
					//Log::Trace("", __FUNCTION__, "WMS_COL_FROM_STOCK_DEV_NO={0}", blkMats.Rows[currRow][WMS_COL_FROM_STOCK_DEV_NO].ToString());

					////执行库位推荐
					//doFlag = f_wms_auto(&in, &out, conn);
					//if (doFlag == 0)
					//{
					//	WM_Utility::PrintLog("库位推荐成功");
					//	WM_Utility::PrintLog("推荐结果材料移动块");
					//	WM_Utility::PrintDataTable(out.Tables[WMS_BLK_OUT_MOVES]);
					//	for (int m = 0; m < out.Tables[WMS_BLK_OUT_MOVES].Rows.get_Count(); m++)
					//	{
					//		twma7.Reset();
					//		twma7["MAT_NO"] = out.Tables[WMS_BLK_OUT_MOVES].Rows[m]["MAT_GRP_NO"];
					//		if (twma7.Query("MAT_NO"))
					//		{
					//			Log::Trace("", __FUNCTION__, "材料{0}存在命令,跳过", twma7["MAT_NO"].ToString());
					//			continue;
					//		}
					//		Log::Trace("", __FUNCTION__, "开始生成材料{0}命令", twma7["MAT_NO"].ToString());
					//		twma7_in.Reset();
					//		twma7_in["REC_CREATOR"] = s.userid;
					//		twma7_in["REC_CREATE_TIME"] = dateTime;
					//		twma7_in["MAT_NO"] = twma7["MAT_NO"].ToString();

					//		twma7_in["CRANE_INST_STATUS"] = "0";
					//		twma7_in["CRANE_INST_CODE"] = crane_inst_code;


					//		sqlstr = "SELECT a.mat_no,a.stock_no,a.hall_no,a.stock_place_no,a.layerno,b.mat_kind,"
					//			"b.mat_shape_flag,b.mat_act_thick,b.mat_act_width,b.mat_act_len,b.mat_act_wt "
					//			" FROM twma2 a,twma1 b WHERE a.mat_no = '" + twma7["MAT_NO"].ToString() + "' AND a.mat_no = b.mat_no";

					//		if (!WM_Utility::QueryData(sqlstr, dtMat, conn))
					//		{
					//			throw CApplicationException(-1, s.msg, s.svc_name);
					//		}

					//		if (dtMat.Rows.get_Count() == 0)
					//		{
					//			sprintf(s.msg, "材料号[%s]不在库内", (const char*)matNo);
					//			throw CApplicationException(-1, s.msg, s.svc_name);
					//		}

					//		Log::Trace("", __FUNCTION__, "写材料源库位信息");
					//		twma7_in["STOCK_NO"] = dtMat.Rows[0]["STOCK_NO"];
					//		twma7_in["STOCK_NO_FROM"] = dtMat.Rows[0]["STOCK_NO"];
					//		twma7_in["HALL_NO_FR"] = dtMat.Rows[0]["HALL_NO"];
					//		twma7_in["STOCK_PLACE_NO_FROM"] = dtMat.Rows[0]["STOCK_PLACE_NO"];
					//		twma7_in["YARD_LAYER_FROM"] = dtMat.Rows[0]["LAYERNO"];

					//		Log::Trace("", __FUNCTION__, "写材料信息");
					//		twma7_in["MAT_KIND"] = dtMat.Rows[0]["MAT_KIND"];
					//		twma7_in["MAT_SHAPE_FLAG"] = dtMat.Rows[0]["MAT_SHAPE_FLAG"];
					//		twma7_in["MAT_ACT_THICK"] = dtMat.Rows[0]["MAT_ACT_THICK"];
					//		twma7_in["MAT_ACT_WIDTH"] = dtMat.Rows[0]["MAT_ACT_WIDTH"];
					//		twma7_in["MAT_ACT_LEN"] = dtMat.Rows[0]["MAT_ACT_LEN"];
					//		twma7_in["MAT_ACT_WT"] = dtMat.Rows[0]["MAT_ACT_WT"];

					//		if (seqno != 0)
					//		{
					//			twma7_in["CMD_SEQ"] = seqno;
					//		}
					//		else
					//		{
					//			twma7_in["CMD_SEQ"] = atoi(WM_Utility::GetSeqence("seqTest", conn));
					//		}

					//		if (m == out.Tables[WMS_BLK_OUT_MOVES].Rows.get_Count() - 1)
					//		{
					//			Log::Trace("", __FUNCTION__, "写目的材料信息");
					//			twma7_in["STOCK_OPER_ORDER"] = stockOperOrder;

					//			if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("UNIT_CODE"))
					//			{
					//				twma7_in["UNIT_CODE"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["UNIT_CODE"];
					//			}
					//			else
					//			{
					//				twma7_in["UNIT_CODE"] = " ";
					//			}

					//			if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("MOVE_TYPE") &&
					//				bcls_rec->Tables["WM00_CMD"].Rows[i]["MOVE_TYPE"].ToString().Trim() != "")
					//			{
					//				twma7_in["MOVE_TYPE"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["MOVE_TYPE"];
					//			}
					//			else
					//			{
					//				twma7_in["MOVE_TYPE"] = stockOperOrder;
					//			}
					//			Log::Trace("", __FUNCTION__, "写目标库位信息");
					//			if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_NO_TO") ||
					//				bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_TO"].ToString().Trim() == "")
					//			{
					//				twma7_in["STOCK_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FROM"];
					//			}
					//			else
					//			{
					//				twma7_in["STOCK_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_TO"];
					//			}

					//			if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("HALL_NO_TO") ||
					//				bcls_rec->Tables["WM00_CMD"].Rows[i]["HALL_NO_TO"].ToString().Trim() == "")
					//			{
					//				twma7_in["HALL_NO_TO"] = twma7_in["STOCK_NO_TO"].ToString();
					//			}
					//			else
					//			{
					//				twma7_in["HALL_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["HALL_NO_TO"];
					//			}

					//			if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_NO_FIN") &&
					//				bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FIN"].ToString().Trim() != "")
					//			{
					//				twma7_in["STOCK_NO_FIN"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FIN"];
					//			}
					//			else
					//			{
					//				twma7_in["STOCK_NO_FIN"] = " ";
					//			}

					//			if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("MAIN_MAT_NO") ||
					//				bcls_rec->Tables["WM00_CMD"].Rows[i]["MAIN_MAT_NO"].ToString().Trim() == "")
					//			{
					//				twma7_in["MAIN_MAT_NO"] = " ";
					//			}
					//			else
					//			{
					//				twma7_in["MAIN_MAT_NO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["MAIN_MAT_NO"];
					//			}

					//			if (bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_TO"].ToString().Trim() == "")
					//			{
					//				twma7_in["STOCK_PLACE_NO_TO"] = out.Tables[WMS_BLK_OUT_MOVES].Rows[m]["TO_STOCK_PLACE_NO"];
					//				twma7_in["YARD_LAYER_TO"] = out.Tables[WMS_BLK_OUT_MOVES].Rows[m]["TO_STOCK_LAYER_NO"];
					//			}
					//			else
					//			{
					//				twma7_in["STOCK_PLACE_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_TO"];
					//				if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("YARD_LAYER_TO") ||
					//					bcls_rec->Tables["WM00_CMD"].Rows[i]["YARD_LAYER_TO"].ToString().Trim() == "")
					//				{
					//					twma7_in["YARD_LAYER_TO"] = 0;
					//				}
					//				else
					//				{
					//					twma7_in["YARD_LAYER_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["YARD_LAYER_TO"];;
					//				}
					//			}

					//			if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_PLACE_NO_FIN") &&
					//				bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_FIN"].ToString().Trim() != "")
					//			{
					//				twma7_in["STOCK_PLACE_NO_FIN"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_FIN"].ToString();
					//			}
					//			else
					//			{
					//				twma7_in["STOCK_PLACE_NO_FIN"] = " ";
					//			}

					//			if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_OPER_ORDER_FIN") &&
					//				bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_OPER_ORDER_FIN"].ToString().Trim() != "")
					//			{
					//				twma7_in["STOCK_OPER_ORDER_FIN"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_OPER_ORDER_FIN"].ToString();
					//			}
					//			else
					//			{
					//				twma7_in["STOCK_OPER_ORDER_FIN"] = stockOperOrder;
					//			}

					//		}
					//		else
					//		{
					//			Log::Trace("", __FUNCTION__, "写倒跺材料信息");
					//			twma7_in["STOCK_OPER_ORDER"] = "31";
					//			twma7_in["MOVE_TYPE"] = "31";
					//			twma7_in["STOCK_NO_TO"] = twma7_in["STOCK_NO"].ToString();
					//			twma7_in["HALL_NO_TO"] = twma7_in["STOCK_NO"].ToString();
					//			twma7_in["STOCK_PLACE_NO_TO"] = out.Tables[WMS_BLK_OUT_MOVES].Rows[m]["TO_STOCK_PLACE_NO"];
					//			twma7_in["YARD_LAYER_TO"] = out.Tables[WMS_BLK_OUT_MOVES].Rows[m]["TO_STOCK_LAYER_NO"];
					//		}
					//		twma7_in.TrimOrBlank();
					//		twma7_in.Insert();
					//		Log::Trace("", __FUNCTION__, "刷新库位状态");
					//		doFlag = f_wm00_pileinfocal(twma7_in["STOCK_NO"].ToString(),
					//			twma7_in["STOCK_PLACE_NO_TO"].ToString(), bcls_ret, conn);
					//		if (doFlag != 0)
					//		{
					//			throw CApplicationException(-1, s.msg, log.Location);
					//		}
					//	}
					//}
					//else
					//{
					//	WM_Utility::PrintLog("库位推荐失败");
					//}

				}
				else if (Flag == 0)
				{
					Log::Trace("", __FUNCTION__, "材料不需要上层倒跺");
					//新增行车命令数据表行
					dtCraneCmd.Rows.Clear();
					dtCraneCmd.Rows.Add();
					twma7_in["REC_CREATOR"] = s.userid;
					twma7_in["REC_CREATE_TIME"] = dateTime;
					twma7_in["MAT_NO"] = matNo;
					twma7_in["STOCK_OPER_ORDER"] = stockOperOrder;
					twma7_in["CRANE_INST_STATUS"] = "0";
					twma7_in["CRANE_INST_CODE"] = crane_inst_code;
					if (seqno != 0)
					{
						twma7_in["CMD_SEQ"] = seqno;
					}
					else
					{
						Log::Trace("", __FUNCTION__, "错误定位1");
						sqlstr = "values nextval for seqTest";
						Log::Trace("", __FUNCTION__, "错误定位2");
						twma7_in["CMD_SEQ"] = atoi(Db::QueryCString(sqlstr));
						Log::Trace("", __FUNCTION__, "错误定位3");
						//twma7_in["CMD_SEQ"] = atoi(WM_Utility::GetSeqence("seqTest", conn));
					}

					Log::Trace("", __FUNCTION__, "错误定位");
					sqlstr = "SELECT a.mat_no,a.stock_no,a.hall_no,a.stock_place_no,a.layerno,b.mat_kind,"
						"b.mat_shape_flag,b.mat_act_thick,b.mat_act_width,b.mat_act_len,b.mat_act_wt "
						" FROM twma2 a," + v_table_name + " b WHERE a.mat_no = '" + matNo + "' AND a.mat_no = b.mat_no";

					/*if (!Db::QueryTable(sqlstr, dtMat))
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}*/
					Log::Trace("", __FUNCTION__, "001");
					Db::QueryTable(sqlstr, dtMat);
					Log::Trace("", __FUNCTION__, "002");
					if (dtMat.Rows.get_Count() == 0)
					{
						sprintf(s.msg, "Materials[%s] is not in yard", (const char*)matNo);
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					Log::Trace("", __FUNCTION__, "写材料源库位信息");
					twma7_in["STOCK_NO"] = dtMat.Rows[0]["STOCK_NO"];
					twma7_in["STOCK_NO_FROM"] = dtMat.Rows[0]["STOCK_NO"];
					twma7_in["HALL_NO_FR"] = dtMat.Rows[0]["HALL_NO"];
					twma7_in["STOCK_PLACE_NO_FROM"] = dtMat.Rows[0]["STOCK_PLACE_NO"];
					twma7_in["YARD_LAYER_FROM"] = dtMat.Rows[0]["LAYERNO"];

					//WM_Utility::PrintLog("写材料信息");
					Log::Trace("", __FUNCTION__, "写材料信息");
					twma7_in["MAT_KIND"] = dtMat.Rows[0]["MAT_KIND"];
					twma7_in["MAT_SHAPE_FLAG"] = dtMat.Rows[0]["MAT_SHAPE_FLAG"];
					twma7_in["MAT_ACT_THICK"] = dtMat.Rows[0]["MAT_ACT_THICK"];
					twma7_in["MAT_ACT_WIDTH"] = dtMat.Rows[0]["MAT_ACT_WIDTH"];
					twma7_in["MAT_ACT_LEN"] = dtMat.Rows[0]["MAT_ACT_LEN"];
					twma7_in["MAT_ACT_WT"] = dtMat.Rows[0]["MAT_ACT_WT"];

					if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("UNIT_CODE") &&
						bcls_rec->Tables["WM00_CMD"].Rows[i]["UNIT_CODE"].ToString().Trim() != "")
					{
						twma7_in["UNIT_CODE"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["UNIT_CODE"];
					}
					else
					{
						twma7_in["UNIT_CODE"] = " ";
					}

					if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("MOVE_TYPE") &&
						bcls_rec->Tables["WM00_CMD"].Rows[i]["MOVE_TYPE"].ToString().Trim() != "")
					{
						twma7_in["MOVE_TYPE"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["MOVE_TYPE"];
					}
					else
					{
						twma7_in["MOVE_TYPE"] = stockOperOrder;
					}

					//写目标库位信息
					if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_NO_TO") ||
						bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_TO"].ToString().Trim() == "")
					{
						twma7_in["STOCK_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FROM"];
					}
					else
					{
						twma7_in["STOCK_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_TO"];
					}

					if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("HALL_NO_TO") ||
						bcls_rec->Tables["WM00_CMD"].Rows[i]["HALL_NO_TO"].ToString().Trim() == "")
					{
						twma7_in["HALL_NO_TO"] = twma7_in["STOCK_NO_TO"].ToString();
					}
					else
					{
						twma7_in["HALL_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["HALL_NO_TO"];
					}
					twma7_in["STOCK_PLACE_NO_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_TO"];

					if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("YARD_LAYER_TO") ||
						bcls_rec->Tables["WM00_CMD"].Rows[i]["YARD_LAYER_TO"].ToString().Trim() == "")
					{
						twma7_in["YARD_LAYER_TO"] = 0;
					}
					else
					{
						twma7_in["YARD_LAYER_TO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["YARD_LAYER_TO"];;
					}

					if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_PLACE_NO_FIN") &&
						bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_FIN"].ToString().Trim() != "")
					{
						twma7_in["STOCK_PLACE_NO_FIN"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_PLACE_NO_FIN"].ToString();
					}
					else
					{
						twma7_in["STOCK_PLACE_NO_FIN"] = " ";
					}

					if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_OPER_ORDER_FIN") &&
						bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_OPER_ORDER_FIN"].ToString().Trim() != "")
					{
						twma7_in["STOCK_OPER_ORDER_FIN"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_OPER_ORDER_FIN"].ToString();
					}
					else
					{
						twma7_in["STOCK_OPER_ORDER_FIN"] = stockOperOrder;
					}

					if (bcls_rec->Tables["WM00_CMD"].Columns.Contains("STOCK_NO_FIN") &&
						bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FIN"].ToString().Trim() != "")
					{
						twma7_in["STOCK_NO_FIN"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["STOCK_NO_FIN"];
					}
					else
					{
						twma7_in["STOCK_NO_FIN"] = " ";
					}

					if (!bcls_rec->Tables["WM00_CMD"].Columns.Contains("MAIN_MAT_NO") ||
						bcls_rec->Tables["WM00_CMD"].Rows[i]["MAIN_MAT_NO"].ToString().Trim() == "")
					{
						twma7_in["MAIN_MAT_NO"] = " ";
					}
					else
					{
						twma7_in["MAIN_MAT_NO"] = bcls_rec->Tables["WM00_CMD"].Rows[i]["MAIN_MAT_NO"];
					}
					twma7_in.TrimOrBlank();
					twma7_in.Insert();
					Log::Trace("", __FUNCTION__, "刷新库位状态");
					doFlag = f_wm00_pileinfocal(twma7_in["STOCK_NO"].ToString(),
						twma7_in["STOCK_PLACE_NO_TO"].ToString(), bcls_ret, conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					continue;
				}
				else
				{

				}
			}
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "Database processing error. sqlcode=[{0}]."  /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
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
