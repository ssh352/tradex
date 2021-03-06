/*
* Copyright (c) 2018-2018 the TradeX authors
* All rights reserved.
*
* The project sponsor and lead author is Xu Rendong.
* E-mail: xrd@ustc.edu, QQ: 277195007, WeChat: ustc_xrd
* See the contributors file for names of other contributors.
*
* Commercial use of this code in source and binary forms is
* governed by a LGPL v3 license. You may get a copy from the
* root directory. Or else you should get a specific written
* permission from the project author.
*
* Individual and educational use of this code in source and
* binary forms is governed by a 3-clause BSD license. You may
* get a copy from the root directory. Certainly welcome you
* to contribute code of all sorts.
*
* Be sure to retain the above copyright notice and conditions.
*/

#include "session_ape.h"
#include "trader_ape_.h"

bool OnSubscibe_JY( HANDLE_CONN api_connect, HANDLE_SESSION api_session, long subscibe, void* data ) {
	Session* session = (Session*)data;
	session->m_call_back_event_lock_jy.lock();
	bool result = session->CallBackEvent( api_connect, api_session, subscibe );
	session->m_call_back_event_lock_jy.unlock();
	return result;
}

Session::Session( TraderAPE_P* trader_ape_p )
	: m_session( 0 )
	, m_username( "" )
	, m_password( "" )
	, m_node_info( "" )
	, m_sys_user_id( "" )
	, m_connect( 0 )
	, m_connect_ok( false )
	, m_subscibe_ok( false )
	, m_subscibe_jy( 0 )
	, m_service_user_running( false )
	, m_log_cate( "<TRADER_APE>" ) {
	m_map_set_field_func = &m_set_field.m_map_set_field_func;
	m_map_get_field_func = &m_get_field.m_map_get_field_func;
	m_trader_ape_p = trader_ape_p; // 不检测是否空指针
}

Session::~Session() {
	StopServiceUser();
}

void Session::CreateServiceUser() {
	std::string log_info;

	FormatLibrary::StandardLibrary::FormatTo( log_info, "创建 会话 {0} 输入输出服务线程完成, 开始进行输入输出服务 ...", m_session );
	m_trader_ape_p->LogPrint( basicx::syslog_level::c_info, log_info );

	try {
		try {
			m_service_user = boost::make_shared<boost::asio::io_service>();
			boost::asio::io_service::work work( *m_service_user );
			m_work_thread_user = boost::make_shared<boost::thread>( boost::bind( &boost::asio::io_service::run, m_service_user ) );
			m_service_user_running = true;
			m_work_thread_user->join();
		}
		catch( std::exception& ex ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "会话 {0} 输入输出服务 初始化 异常：{1}", m_session, ex.what() );
			m_trader_ape_p->LogPrint( basicx::syslog_level::c_error, log_info );
		}
	} // try
	catch( ... ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "会话 {0} 输入输出服务线程发生未知错误！", m_session );
		m_trader_ape_p->LogPrint( basicx::syslog_level::c_fatal, log_info );
	}

	StopServiceUser();

	FormatLibrary::StandardLibrary::FormatTo( log_info, "会话 {0} 输入输出服务线程退出！", m_session );
	m_trader_ape_p->LogPrint( basicx::syslog_level::c_warn, log_info );
}

void Session::HandleRequestMsg() {
	std::string log_info;

	try {
		std::string result_data = "";
		Request* request = &m_list_request.front(); // 肯定有

		//FormatLibrary::StandardLibrary::FormatTo( log_info, "会话 {0} 开始处理 {1} 号任务 ...", m_session, request->m_task_id );
		//m_trader_ape_p->LogPrint( basicx::syslog_level::c_info, log_info );

		int32_t func_id = 0;
		int32_t task_id = 0;
		try {
			if( NW_MSG_CODE_JSON == request->m_code ) {
				func_id = request->m_req_json["function"].asInt();
				task_id = request->m_req_json["task_id"].asInt();
			}
			// func_id > 0 已在 HandleTaskMsg() 中保证

			switch( func_id ) {
			case TD_FUNC_STOCK_ADDSUB:
				result_data = OnSubscibe( request );
				break;
			case TD_FUNC_STOCK_DELSUB:
				result_data = OnUnsubscibe( request );
				break;
			default: // 其他交易类功能编号
				long api_session = Fix_AllocateSession( m_connect );
				result_data = OnTradeRequest( request, api_session );
				//result_data = OnTradeRequest_Simulate( request, api_session );
				Fix_ReleaseSession( api_session );
			}
		}
		catch( ... ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "会话 {0} 处理任务 {1} 时发生未知错误！", m_session, request->m_task_id );
			result_data = m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
		}

		m_trader_ape_p->CommitResult( request->m_task_id, request->m_identity, request->m_code, result_data );

		//FormatLibrary::StandardLibrary::FormatTo( log_info, "会话 {0} 处理 {1} 号任务完成。", m_session, request->m_task_id );
		//m_trader_ape_p->LogPrint( basicx::syslog_level::c_info, log_info );

		m_request_list_lock.lock();
		m_list_request.pop_front();
		bool write_on_progress = !m_list_request.empty();
		m_request_list_lock.unlock();

		if( write_on_progress && true == m_service_user_running ) { // m_service_user_running
			m_service_user->post( boost::bind( &Session::HandleRequestMsg, this ) );
		}
	}
	catch( std::exception& ex ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "会话 {0} 处理 Request 消息 异常：{1}", m_session, ex.what() );
		m_trader_ape_p->LogPrint( basicx::syslog_level::c_error, log_info );
	}
}

void Session::StopServiceUser() {
	if( true == m_service_user_running ) {
		m_service_user_running = false;
		m_service_user->stop();
	}
}

std::string Session::OnSubscibe( Request* request ) {
	std::string log_info;

	int32_t task_id = 0;
	std::string s_password = "";
	if( NW_MSG_CODE_JSON == request->m_code ) {
		task_id = request->m_req_json["task_id"].asInt();
		s_password = request->m_req_json["password"].asString();
	}
	if( "" == s_password ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "用户订阅时 密码 为空！session：{0}", m_session );
		return m_trader_ape_p->OnErrorResult( TD_FUNC_STOCK_ADDSUB, -1, log_info, task_id, request->m_code );
	}
	if( m_password != s_password ) { // 需要密码才能订阅
		FormatLibrary::StandardLibrary::FormatTo( log_info, "用户订阅时 密码 错误！session：{0}", m_session );
		return m_trader_ape_p->OnErrorResult( TD_FUNC_STOCK_ADDSUB, -1, log_info, task_id, request->m_code );
	}

	if( false == m_subscibe_ok ) {
		char c_password[64] = { 0 };
		strcpy_s( c_password, 64, m_password.c_str() );
		//Fix_Encode( c_password ); // 接口 APE 不需要，不然会导致订阅返回为零
		m_subscibe_jy = Fix_SubscibeByCustomer( m_connect, 1000050, OnSubscibe_JY, this, m_username.c_str(), c_password );
		if( m_subscibe_jy <= 0 ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "订阅 1000050 交易回报 异常！session：{0}", m_session );
			return m_trader_ape_p->OnErrorResult( TD_FUNC_STOCK_ADDSUB, -1, log_info, task_id, request->m_code );
		}
		m_subscibe_ok = true;
	}
	m_sub_endpoint_map_lock.lock();
	m_map_sub_endpoint[request->m_identity] = request->m_identity;
	m_sub_endpoint_map_lock.unlock();

	FormatLibrary::StandardLibrary::FormatTo( log_info, "用户订阅成功。session：{0}", m_session );
	m_trader_ape_p->LogPrint( basicx::syslog_level::c_info, log_info );

	if( NW_MSG_CODE_JSON == request->m_code ) {
		Json::Value results_json;
		results_json["ret_func"] = TD_FUNC_STOCK_ADDSUB;
		results_json["ret_code"] = 0;
		results_json["ret_info"] = basicx::StringToUTF8( log_info );
		results_json["ret_task"] = task_id;
		results_json["ret_last"] = true;
		results_json["ret_numb"] = 0;
		results_json["ret_data"] = "";
		return Json::writeString( m_json_writer, results_json );
	}

	return "";
}

std::string Session::OnUnsubscibe( Request* request ) {
	std::string log_info;

	int32_t task_id = 0;
	if( NW_MSG_CODE_JSON == request->m_code ) {
		task_id = request->m_req_json["task_id"].asInt();
	}

	m_sub_endpoint_map_lock.lock();
	m_map_sub_endpoint.erase( request->m_identity );
	m_sub_endpoint_map_lock.unlock();

	if( m_map_sub_endpoint.empty() ) { // 订阅已无用户
		if( true == m_subscibe_ok ) {
			m_subscibe_ok = false;
			Fix_UnSubscibeByHandle( m_subscibe_jy );
		}
	}

	FormatLibrary::StandardLibrary::FormatTo( log_info, "用户退订成功。session：{0}", m_session );
	m_trader_ape_p->LogPrint( basicx::syslog_level::c_info, log_info );

	if( NW_MSG_CODE_JSON == request->m_code ) {
		Json::Value results_json;
		results_json["ret_func"] = TD_FUNC_STOCK_DELSUB;
		results_json["ret_code"] = 0;
		results_json["ret_info"] = basicx::StringToUTF8( log_info );
		results_json["ret_task"] = task_id;
		results_json["ret_last"] = true;
		results_json["ret_numb"] = 0;
		results_json["ret_data"] = "";
		return Json::writeString( m_json_writer, results_json );
	}

	return "";
}

std::string Session::OnTradeRequest( Request* request, HANDLE_SESSION api_session ) {
	std::string log_info = "";

	int32_t func_id = 0;
	int32_t task_id = 0;
	if( NW_MSG_CODE_JSON == request->m_code ) {
		func_id = request->m_req_json["function"].asInt();
		task_id = request->m_req_json["task_id"].asInt();
	}

	auto it_set_field_func = m_map_set_field_func->find( func_id );
	if( it_set_field_func == m_map_set_field_func->end() ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 没有对应 填值 函数！", func_id );
		return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
	}

	auto it_get_field_func = m_map_get_field_func->find( func_id );
	if( it_get_field_func == m_map_get_field_func->end() ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 没有对应 取值 函数！", func_id );
		return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
	}

	int32_t func_type = 0;
	switch( func_id ) {
	case 120001: // 单个委托下单
		func_type = 204501;
		break;
	case 120002: // 单个委托撤单
		func_type = 204502;
		break;
	case 120003: // 批量委托下单
		func_type = 204513;
		break;
	case 120004: // 批量委托撤单
		func_type = 204511;
		break;
	case 120005: // 港股通买卖委托
		func_type = 204545;
		break;
	case 120006: // 港股通委托撤单
		func_type = 204546;
		break;
	case 130002: // 查询客户资金
		func_type = 303002;
		break;
	case 130004: // 查询客户持仓
		func_type = 304101;
		break;
	case 130005: // 查询客户当日委托
		func_type = 304103;
		break;
	case 130006: // 查询客户当日成交
		func_type = 304110;
		break;
	case 130008: // 查询ETF基本信息
		func_type = 104105;
		break;
	case 130009: // 查询ETF成分股信息
		func_type = 104106;
		break;
	}

	if( 120001 == func_id || 120002 == func_id || 120005 == func_id || 120006 == func_id ) { // 风控检查 // 目前未对批量委托做风控检查
		std::string asset_account = request->m_req_json["asset_account"].asString();
		int32_t risk_ret = m_risker->HandleRiskCtlCheck( asset_account, func_id, task_id, request, log_info );
		if( risk_ret < 0 ) {
			return m_trader_ape_p->OnErrorResult( func_id, risk_ret, log_info, task_id, request->m_code );
		}
	}

	Fix_SetNode( api_session, m_node_info.c_str() ); //
	int32_t ret = Fix_CreateReq( api_session, func_type ); // Fix_CreateHead、Fix_CreateReq 都可以
	if( ret < 0 ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 创建 Fix_CreateReq 失败！", func_id );
		return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
	}

	if( 120001 == func_id || 120002 == func_id || 120003 == func_id || 120004 == func_id || 120005 == func_id || 120006 == func_id || 130002 == func_id || 130004 == func_id || 130005 == func_id || 130006 == func_id ) {
		Fix_SetString( api_session, FID_KHH, m_username.c_str() ); // 客户号
		char c_password[64] = { 0 };
		strcpy_s( c_password, 64, m_password.c_str() );
		Fix_Encode( c_password ); // 接口 APE 不使用也可以
		Fix_SetString( api_session, FID_JYMM, c_password ); // 交易密码
		Fix_SetString( api_session, FID_JMLX, "0" ); // 加密类型 // 接口 APE 不指定也可以
//		Fix_SetString( api_session, FID_WTGY, m_sys_user_id.c_str() ); // 委托柜员 // 接口 APE 不能填，不然查不到当日委托
	}
	
	SetField::SetFieldFunc set_field_func = it_set_field_func->second;
	if( !(m_set_field.*set_field_func)( api_session, request ) ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 填值 异常！", func_id );
		return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
	}

	m_fid_code = 0;
	memset( &m_fid_message, 0, APE_FID_MESSAGE_LENGTH );

	if( Fix_Run( api_session ) ) {
		if( 120001 == func_id || 120005 == func_id ) { // 缓存本地委托号与委托请求的映射供交易回报获取对应用户请求用
			long ret_count = Fix_GetCount( api_session ); // 业务执行出错时 ret_count == 0
			if( ret_count > 0 ) {
				for( int32_t i = 0; i < ret_count; i++ ) { // 其实只有一条的
					int32_t order_ref = Fix_GetLong( api_session, FID_WTH, i ); // 委托号 Int
					m_order_ref_request_map_lock.lock();
					m_map_order_ref_request[order_ref] = *request;
					m_order_ref_request_map_lock.unlock();
					break;
				}
			}
		}

		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 提交成功。", func_id );
		m_trader_ape_p->LogPrint( basicx::syslog_level::c_info, log_info );

		std::string results = "";
		GetField::GetFieldFunc get_field_func = it_get_field_func->second;
		if( !(m_get_field.*get_field_func)( api_session, request, results ) ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 取值 异常！", func_id );
			return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
		}

		//m_trader_ape_p->LogPrint( basicx::syslog_level::c_debug, results, FILE_LOG_ONLY ); // 查询类返回的数据可能会非常多

		if( 120001 == func_id || 120002 == func_id || 120005 == func_id || 120006 == func_id ) { // 风控检查 // 目前未对批量委托做风控检查
			std::string asset_account = request->m_req_json["asset_account"].asString();
			m_risker->CheckTradeResultForRisk( asset_account, func_id, task_id, results );
		}

		return results;
	}
	else {
		m_fid_code = Fix_GetCode( api_session );
		Fix_GetErrMsg( api_session, m_fid_message, APE_FID_MESSAGE_LENGTH );
		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 执行 Fix_Run 失败！{1}", func_id, m_fid_message );
		return m_trader_ape_p->OnErrorResult( func_id, m_fid_code, log_info, task_id, request->m_code );
	}

	return "";
}

std::string Session::OnTradeRequest_Simulate( Request* request, HANDLE_SESSION api_session ) {
	std::string log_info = "";

	int32_t func_id = 0;
	int32_t task_id = 0;
	if( NW_MSG_CODE_JSON == request->m_code ) {
		func_id = request->m_req_json["function"].asInt();
		task_id = request->m_req_json["task_id"].asInt();
	}

	auto it_set_field_func = m_map_set_field_func->find( func_id );
	if( it_set_field_func == m_map_set_field_func->end() ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 没有对应 填值 函数！", func_id );
		return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
	}

	auto it_get_field_func = m_map_get_field_func->find( func_id );
	if( it_get_field_func == m_map_get_field_func->end() ) {
		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 没有对应 取值 函数！", func_id );
		return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
	}

	int32_t func_type = 0;
	switch( func_id ) {
	case 120001: // 单个委托下单
		func_type = 204501;
		break;
	case 120002: // 单个委托撤单
		func_type = 204502;
		break;
	case 120003: // 批量委托下单
		func_type = 204513;
		break;
	case 120004: // 批量委托撤单
		func_type = 204511;
		break;
	case 120005: // 港股通买卖委托
		func_type = 204545;
		break;
	case 120006: // 港股通委托撤单
		func_type = 204546;
		break;
	case 130002: // 查询客户资金
		func_type = 303002;
		break;
	case 130004: // 查询客户持仓
		func_type = 304101;
		break;
	case 130005: // 查询客户当日委托
		func_type = 304103;
		break;
	case 130006: // 查询客户当日成交
		func_type = 304110;
		break;
	case 130008: // 查询ETF基本信息
		func_type = 104105;
		break;
	case 130009: // 查询ETF成分股信息
		func_type = 104106;
		break;
	}

	if( 120001 == func_id || 120002 == func_id || 120005 == func_id || 120006 == func_id ) { // 风控检查 // 目前未对批量委托做风控检查
		std::string asset_account = request->m_req_json["asset_account"].asString();
		int32_t risk_ret = m_risker->HandleRiskCtlCheck( asset_account, func_id, task_id, request, log_info );
		if( risk_ret < 0 ) {
			return m_trader_ape_p->OnErrorResult( func_id, risk_ret, log_info, task_id, request->m_code );
		}
	}

	if( 120001 == func_id || 120002 == func_id || 120003 == func_id || 120004 == func_id || 120005 == func_id || 120006 == func_id ) { // 模拟成交
		std::string results = "";

		FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 提交成功。", func_id );
		m_trader_ape_p->LogPrint( basicx::syslog_level::c_info, log_info );

		if( 120001 == func_id ) {
			{
				Json::Value results_json;
				results_json["ret_func"] = 120001;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "业务提交成功。" );
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_numb"] = 1;
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = 1;
				ret_data_json["otc_info"] = basicx::StringToUTF8( "委托下单提交成功。" );
				ret_data_json["order_id"] = request->m_req_json["task_id"].asInt();
				results_json["ret_data"].append( ret_data_json );
				results = Json::writeString( m_json_writer, results_json );
			}

			//////////////////// 发送给风控服务端 //////////////////// 委托回报
			{
				Json::Value results_json;
				results_json["ret_func"] = TD_FUNC_RISKS_ORDER_REPORT_STK;
				results_json["task_id"] = 0;
				results_json["asset_account"] = request->m_req_json["asset_account"].asString();
				results_json["account"] = m_username; // 交易账号
				results_json["order_id"] = request->m_req_json["task_id"].asInt();
				results_json["exch_side"] = request->m_req_json["exch_side"].asInt();
				results_json["symbol"] = request->m_req_json["symbol"].asCString();
				results_json["security_type"] = "A0"; // A股
				results_json["exchange"] = request->m_req_json["exchange"].asCString();
				results_json["cxl_qty"] = 0;
				results_json["commit_ret"] = 6; // 全部成交
				results_json["commit_msg"] = basicx::StringToUTF8( "全部成交" );
				results_json["total_fill_qty"] = request->m_req_json["amount"].asInt();
				m_risker->CommitResult( NW_MSG_CODE_JSON, Json::writeString( m_json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
			}
			//////////////////// 发送给风控服务端 ////////////////////

			//////////////////// 发送给风控服务端 //////////////////// 成交回报
			{
				Json::Value results_json;
				results_json["ret_func"] = TD_FUNC_RISKS_TRANSACTION_REPORT_STK;
				results_json["task_id"] = 0;
				results_json["asset_account"] = request->m_req_json["asset_account"].asString();
				results_json["account"] = m_username; // 交易账号
				results_json["order_id"] = request->m_req_json["task_id"].asInt();
				results_json["exch_side"] = request->m_req_json["exch_side"].asInt();
				results_json["trans_id"] = "20171231";
				results_json["symbol"] = request->m_req_json["symbol"].asCString();
				results_json["security_type"] = "A0"; // A股
				results_json["exchange"] = request->m_req_json["exchange"].asCString();
				results_json["fill_qty"] = request->m_req_json["amount"].asInt();
				results_json["fill_price"] = request->m_req_json["price"].asDouble();
				results_json["fill_time"] = "11:29:59";
				results_json["cxl_qty"] = 0;
				m_risker->CommitResult( NW_MSG_CODE_JSON, Json::writeString( m_json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
			}
			//////////////////// 发送给风控服务端 ////////////////////

			//////////////////// 发送给交易客户端 //////////////////// 委托回报
			{
				Json::Value results_json; // 回报统一用 NW_MSG_CODE_JSON 编码
				results_json["ret_func"] = 190001;
				results_json["task_id"] = request->m_req_json["task_id"].asInt();
				results_json["order_id"] = request->m_req_json["task_id"].asInt();
				results_json["exch_side"] = request->m_req_json["exch_side"].asInt();
				results_json["symbol"] = request->m_req_json["symbol"].asCString();
				results_json["security_type"] = "A0"; // A股
				results_json["exchange"] = request->m_req_json["exchange"].asCString();
				results_json["cxl_qty"] = 0;
				results_json["commit_ret"] = 6; // 全部成交
				results_json["commit_msg"] = basicx::StringToUTF8( "全部成交" );
				m_trader_ape_p->CommitResult( 1, request->m_identity, NW_MSG_CODE_JSON, Json::writeString( m_json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
			}
			//////////////////// 发送给交易客户端 ////////////////////

			//////////////////// 发送给交易客户端 //////////////////// 成交回报
			{
				Json::Value results_json; // 回报统一用 NW_MSG_CODE_JSON 编码
				results_json["ret_func"] = 190002;
				results_json["task_id"] = request->m_req_json["task_id"].asInt();
				results_json["order_id"] = request->m_req_json["task_id"].asInt();
				results_json["exch_side"] = request->m_req_json["exch_side"].asInt();
				results_json["trans_id"] = "20171231";
				results_json["symbol"] = request->m_req_json["symbol"].asCString();
				results_json["security_type"] = "A0"; // A股
				results_json["exchange"] = request->m_req_json["exchange"].asCString();
				results_json["fill_qty"] = request->m_req_json["amount"].asInt();
				results_json["fill_price"] = request->m_req_json["price"].asDouble();
				results_json["fill_time"] = "11:29:59";
				results_json["cxl_qty"] = 0;
				m_trader_ape_p->CommitResult( 1, request->m_identity, NW_MSG_CODE_JSON, Json::writeString( m_json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
			}
			//////////////////// 发送给交易客户端 ////////////////////
		}

		if( 120002 == func_id ) {
			{
				Json::Value results_json;
				results_json["ret_func"] = 120002;
				results_json["ret_code"] = 0;
				results_json["ret_info"] = basicx::StringToUTF8( "业务提交成功。" );
				results_json["ret_task"] = request->m_req_json["task_id"].asInt();
				results_json["ret_last"] = true;
				results_json["ret_numb"] = 1;
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = 1;
				ret_data_json["otc_info"] = basicx::StringToUTF8( "委托撤单提交成功。" );
				ret_data_json["order_id"] = request->m_req_json["task_id"].asInt();
				results_json["ret_data"].append( ret_data_json );
				results = Json::writeString( m_json_writer, results_json );
			}

			//////////////////// 发送给风控服务端 //////////////////// 撤单回报
			{
				Json::Value results_json;
				results_json["ret_func"] = TD_FUNC_RISKS_ORDER_REPORT_STK;
				results_json["task_id"] = 0;
				results_json["asset_account"] = request->m_req_json["asset_account"].asString();
				results_json["account"] = m_username; // 交易账号
				results_json["order_id"] = request->m_req_json["task_id"].asInt();
				results_json["exch_side"] = request->m_req_json["exch_side"].asInt();
				results_json["symbol"] = request->m_req_json["symbol"].asCString();
				results_json["security_type"] = "A0"; // A股
				results_json["exchange"] = request->m_req_json["exchange"].asCString();
				results_json["cxl_qty"] = 0;
				results_json["commit_ret"] = 6; // 全部成交
				results_json["commit_msg"] = basicx::StringToUTF8( "全部成交" );
				results_json["total_fill_qty"] = request->m_req_json["amount"].asInt();
				m_risker->CommitResult( NW_MSG_CODE_JSON, Json::writeString( m_json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
			}
			//////////////////// 发送给风控服务端 ////////////////////

			//////////////////// 发送给交易客户端 //////////////////// 撤单回报
			{
				Json::Value results_json; // 回报统一用 NW_MSG_CODE_JSON 编码
				results_json["ret_func"] = 190003;
				results_json["task_id"] = request->m_req_json["task_id"].asInt();
				results_json["order_id"] = request->m_req_json["task_id"].asInt();
				results_json["exch_side"] = request->m_req_json["exch_side"].asInt();
				results_json["symbol"] = request->m_req_json["symbol"].asCString();
				results_json["security_type"] = "A0"; // A股
				results_json["exchange"] = request->m_req_json["exchange"].asCString();
				results_json["cxl_qty"] = 0;
				results_json["total_fill_qty"] = request->m_req_json["amount"].asInt();
				m_trader_ape_p->CommitResult( 1, request->m_identity, NW_MSG_CODE_JSON, Json::writeString( m_json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
			}
			//////////////////// 发送给交易客户端 ////////////////////
		}

		//if( 120003 == func_id ) {}

		//if( 120004 == func_id ) {}

		//if( 120005 == func_id ) {}

		//if( 120006 == func_id ) {}

		if( 120001 == func_id || 120002 == func_id || 120005 == func_id || 120006 == func_id ) { // 风控检查 // 目前未对批量委托做风控检查
			std::string asset_account = request->m_req_json["asset_account"].asString();
			m_risker->CheckTradeResultForRisk( asset_account, func_id, task_id, results );
		}

		return results;
	}
	else { // 其他走柜台
		Fix_SetNode( api_session, m_node_info.c_str() ); //
		int32_t ret = Fix_CreateReq( api_session, func_type );
		if( ret < 0 ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 创建 Fix_CreateReq 失败！", func_id );
			return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
		}

		if( 130002 == func_id || 130004 == func_id || 130005 == func_id || 130006 == func_id ) {
			Fix_SetString( api_session, FID_KHH, m_username.c_str() ); // 客户号
			char c_password[64] = { 0 };
			strcpy_s( c_password, 64, m_password.c_str() );
			Fix_Encode( c_password ); // 接口 APE 不使用也可以
			Fix_SetString( api_session, FID_JYMM, c_password ); // 交易密码
			Fix_SetString( api_session, FID_JMLX, "0" ); // 加密类型 // 接口 APE 不指定也可以
		}

		SetField::SetFieldFunc set_field_func = it_set_field_func->second;
		if( !( m_set_field.*set_field_func )( api_session, request ) ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 填值 异常！", func_id );
			return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
		}

		m_fid_code = 0;
		memset( &m_fid_message, 0, APE_FID_MESSAGE_LENGTH );

		if( Fix_Run( api_session ) ) {
			FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 提交成功。", func_id );
			m_trader_ape_p->LogPrint( basicx::syslog_level::c_info, log_info );

			std::string results = "";
			GetField::GetFieldFunc get_field_func = it_get_field_func->second;
			if( !( m_get_field.*get_field_func )( api_session, request, results ) ) {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 取值 异常！", func_id );
				return m_trader_ape_p->OnErrorResult( func_id, -1, log_info, task_id, request->m_code );
			}

			//m_trader_ape_p->LogPrint( basicx::syslog_level::c_debug, results, FILE_LOG_ONLY ); // 查询类返回的数据可能会非常多

			return results;
		}
		else {
			m_fid_code = Fix_GetCode( api_session );
			Fix_GetErrMsg( api_session, m_fid_message, APE_FID_MESSAGE_LENGTH );
			FormatLibrary::StandardLibrary::FormatTo( log_info, "业务 {0} 执行 Fix_Run 失败！{1}", func_id, m_fid_message );
			return m_trader_ape_p->OnErrorResult( func_id, m_fid_code, log_info, task_id, request->m_code );
		}
	}

	return "";
}

Request* Session::GetRequestByOrderRef( int32_t order_ref ) {
	Request* request = nullptr;
	m_order_ref_request_map_lock.lock();
	std::map<int32_t, Request>::iterator it_r = m_map_order_ref_request.find( order_ref );
	if( it_r != m_map_order_ref_request.end() ) {
		request = &( it_r->second);
	}
	m_order_ref_request_map_lock.unlock();
	return request;
}

bool Session::CallBackEvent( HANDLE_CONN api_connect, HANDLE_SESSION api_session, long subscibe ) {
	std::string log_info;

	char field_value_short[256];
	long ret_count = Fix_GetCount( api_session );
	if( ret_count > 0 ) {
		for( int32_t i = 0; i < ret_count; i++ ) { // 其实只有一条的
			memset( field_value_short, 0, FIELD_VALUE_SHORT );
			std::string fid_cxbz = Fix_GetItem( api_session, FID_CXBZ, field_value_short, FIELD_VALUE_SHORT, i ); // 撤销标志，"W"表示撤单记录，其它为委托记录
			memset( field_value_short, 0, FIELD_VALUE_SHORT );
			std::string fid_ywlb = Fix_GetItem( api_session, FID_YWLB, field_value_short, FIELD_VALUE_SHORT, i ); // 业务类别
			int32_t fid_cjsl = Fix_GetLong( api_session, FID_CJSL, i ); // 成交数量

			// 中午休市时间段，貌似委托缓存在柜台，可以从柜台撤单，但不会收到报单和撤单的回报，只有报单和撤单的应答
			FormatLibrary::StandardLibrary::FormatTo( log_info, "交易回报：{0} {1} {2} {3}", fid_cxbz, fid_ywlb, fid_cjsl, i );
			m_trader_ape_p->LogPrint( basicx::syslog_level::c_debug, log_info );

			std::string results = "";
			std::string asset_account = "";
			Json::StreamWriterBuilder json_writer; //
			int32_t order_ref = Fix_GetLong( api_session, FID_WTH, i ); // 委托号 Int
			Request* request = GetRequestByOrderRef( order_ref );
			if( request != nullptr ) {
				asset_account = request->m_req_json["asset_account"].asString();

				if( "O" == fid_cxbz && "Q" == fid_ywlb && 0 == fid_cjsl ) { // 申报回报
					try {
						Json::Value results_json; // 回报统一用 NW_MSG_CODE_JSON 编码
						results_json["ret_func"] = 190001;
						results_json["task_id"] = request->m_req_json["task_id"].asInt();
						results_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // 委托号 Int
						results_json["exch_side"] = Fix_GetLong( api_session, FID_WTLB, i ); // 委托类别 Int
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["symbol"] = Fix_GetItem( api_session, FID_ZQDM, field_value_short, FIELD_VALUE_SHORT, i ); // 证券代码 Char 6
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["security_type"] = Fix_GetItem( api_session, FID_ZQLB, field_value_short, FIELD_VALUE_SHORT, i ); // 证券类别 Char 2
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["exchange"] = Fix_GetItem( api_session, FID_JYS, field_value_short, FIELD_VALUE_SHORT, i ); // 交易所 Char 2 // "SH"，"SZ"，"HK"
//						results_json["cxl_qty"] = Fix_GetLong( api_session, FID_CDSL, i ); // 撤单数量 Int // 测试：0
						results_json["cxl_qty"] = 0;
						// 0：未申报，1：正在申报，2：已申报未成交，3：非法委托，4：申请资金授权中，
						// 5：部分成交，6：全部成交，7：部成部撤，8：全部撤单，9：撤单未成，10：等待人工申报
//						results_json["commit_ret"] = Fix_GetLong( api_session, FID_SBJG, i ); // 申报结果 Int // 测试：0
						results_json["commit_ret"] = 2;
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
//						results_json["commit_msg"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_JGSM, field_value_short, FIELD_VALUE_SHORT, i ) ); // 结果说明 Char 64 // 测试：""
						results_json["commit_msg"] = "";
						results = Json::writeString( json_writer, results_json );
						if( results != "" ) {
							m_trader_ape_p->CommitResult( 1, request->m_identity, NW_MSG_CODE_JSON, results ); // 回报统一用 NW_MSG_CODE_JSON 编码 // Trade：1、Risks：2
						}
						//////////////////// 发送给风控服务端 ////////////////////
						results_json["ret_func"] = TD_FUNC_RISKS_ORDER_REPORT_STK;
						results_json["task_id"] = 0;
						results_json["asset_account"] = asset_account; // 产品账号
						results_json["account"] = m_username; // 交易账号
						results_json["total_fill_qty"] = Fix_GetLong( api_session, FID_CJSL, i ); // 成交数量 Int
						m_risker->CommitResult( NW_MSG_CODE_JSON, Json::writeString( json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
						//////////////////// 发送给风控服务端 ////////////////////
						m_trader_ape_p->LogPrint( basicx::syslog_level::c_debug, results, FILE_LOG_ONLY );
					}
					catch( ... ) {
						log_info = "申报 回报取值异常！";
						m_trader_ape_p->LogPrint( basicx::syslog_level::c_error, log_info );
					}
				}
				else if( "W" == fid_cxbz && "C" == fid_ywlb && fid_cjsl < 0 ) { // 撤单回报
					try {
						Json::Value results_json; // 回报统一用 NW_MSG_CODE_JSON 编码
						results_json["ret_func"] = 190003;
						results_json["task_id"] = request->m_req_json["task_id"].asInt();
						results_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // 委托号 Int
						results_json["exch_side"] = Fix_GetLong( api_session, FID_WTLB, i ); // 委托类别 Int
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["symbol"] = Fix_GetItem( api_session, FID_ZQDM, field_value_short, FIELD_VALUE_SHORT, i ); // 证券代码 Char 6
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["security_type"] = Fix_GetItem( api_session, FID_ZQLB, field_value_short, FIELD_VALUE_SHORT, i ); // 证券类别 Char 2
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["exchange"] = Fix_GetItem( api_session, FID_JYS, field_value_short, FIELD_VALUE_SHORT, i ); // 交易所 Char 2 // "SH"，"SZ"，"HK"
//						results_json["cxl_qty"] = Fix_GetLong( api_session, FID_CDSL, i ); // 撤单数量 Int // 测试：0
//						results_json["total_fill_qty"] = Fix_GetLong( api_session, FID_CJSL, i ); // 成交数量 Int // 全部撤单则为 -amount
						int32_t amount = request->m_req_json["amount"].asInt();
						int32_t total_fill_qty = amount + Fix_GetLong( api_session, FID_CJSL, i ); // 成交数量 Int // 全部撤单则为 -amount
						results_json["cxl_qty"] = amount - total_fill_qty;
						results_json["total_fill_qty"] = total_fill_qty;
						results = Json::writeString( json_writer, results_json );
						if( results != "" ) {
							m_trader_ape_p->CommitResult( 1, request->m_identity, NW_MSG_CODE_JSON, results ); // 回报统一用 NW_MSG_CODE_JSON 编码 // Trade：1、Risks：2
						}
						//////////////////// 发送给风控服务端 ////////////////////
						results_json["ret_func"] = TD_FUNC_RISKS_ORDER_REPORT_STK;
						results_json["task_id"] = 0;
						results_json["asset_account"] = asset_account; // 产品账号
						results_json["account"] = m_username; // 交易账号
						// 0：未申报，1：正在申报，2：已申报未成交，3：非法委托，4：申请资金授权中，
						// 5：部分成交，6：全部成交，7：部成部撤，8：全部撤单，9：撤单未成，10：等待人工申报
//						results_json["commit_ret"] = Fix_GetLong( api_session, FID_SBJG, i ); // 申报结果 Int // 测试：0
						// 如果已全部成交，则只会有一般的撤单应答，不会有撤单回报和撤单失败应答
						if( total_fill_qty < amount ) {
							results_json["commit_ret"] = 7; // 部成部撤
						}
						else {
							results_json["commit_ret"] = 8; // 全部撤单
						}
//						results_json["commit_msg"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_JGSM, field_value_short, FIELD_VALUE_SHORT, i ) ); // 测试：""
						results_json["commit_msg"] = "";
						m_risker->CommitResult( NW_MSG_CODE_JSON, Json::writeString( json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
						//////////////////// 发送给风控服务端 ////////////////////
						m_trader_ape_p->LogPrint( basicx::syslog_level::c_debug, results, FILE_LOG_ONLY );
					}
					catch( ... ) {
						log_info = "撤单 回报取值异常！";
						m_trader_ape_p->LogPrint( basicx::syslog_level::c_error, log_info );
					}
				}
				else if( "O" == fid_cxbz && "1" == fid_ywlb && fid_cjsl > 0 ) { // 成交回报
					try {
						Json::Value results_json; // 回报统一用 NW_MSG_CODE_JSON 编码
						results_json["ret_func"] = 190002;
						results_json["task_id"] = request->m_req_json["task_id"].asInt();
						results_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // 委托号 Int
						results_json["exch_side"] = Fix_GetLong( api_session, FID_WTLB, i ); // 委托类别 Int
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["trans_id"] = Fix_GetItem( api_session, FID_CJBH, field_value_short, FIELD_VALUE_SHORT, i ); // 成交编号 Char 16
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["symbol"] = Fix_GetItem( api_session, FID_ZQDM, field_value_short, FIELD_VALUE_SHORT, i ); // 证券代码 Char 6
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["security_type"] = Fix_GetItem( api_session, FID_ZQLB, field_value_short, FIELD_VALUE_SHORT, i ); // 证券类别 Char 2
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["exchange"] = Fix_GetItem( api_session, FID_JYS, field_value_short, FIELD_VALUE_SHORT, i ); // 交易所 Char 2 // "SH"，"SZ"，"HK"
						results_json["fill_qty"] = Fix_GetLong( api_session, FID_CJSL, i ); // 本次成交数量 Int
						results_json["fill_price"] = Fix_GetDouble( api_session, FID_CJJG, i ); // 本次成交价格 Numeric 9,3
						memset( field_value_short, 0, FIELD_VALUE_SHORT );
						results_json["fill_time"] = Fix_GetItem( api_session, FID_CJSJ, field_value_short, FIELD_VALUE_SHORT, i ); // 成交时间 Char 8
//						results_json["cxl_qty"] = Fix_GetLong( api_session, FID_CDSL, i ); // 撤单数量 Int // 测试：0
						results_json["cxl_qty"] = 0;
						results = Json::writeString( json_writer, results_json );
						if( results != "" ) {
							m_trader_ape_p->CommitResult( 1, request->m_identity, NW_MSG_CODE_JSON, results ); // 回报统一用 NW_MSG_CODE_JSON 编码 // Trade：1、Risks：2
						}
						//////////////////// 发送给风控服务端 ////////////////////
						results_json["ret_func"] = TD_FUNC_RISKS_TRANSACTION_REPORT_STK;
						results_json["task_id"] = 0;
						results_json["asset_account"] = asset_account; // 产品账号
						results_json["account"] = m_username; // 交易账号
						m_risker->CommitResult( NW_MSG_CODE_JSON, Json::writeString( json_writer, results_json ) ); // 回报统一用 NW_MSG_CODE_JSON 编码
						//////////////////// 发送给风控服务端 ////////////////////
						m_trader_ape_p->LogPrint( basicx::syslog_level::c_debug, results, FILE_LOG_ONLY );
					}
					catch( ... ) {
						log_info = "成交 回报取值异常！";
						m_trader_ape_p->LogPrint( basicx::syslog_level::c_error, log_info );
					}
				}
				else {
					FormatLibrary::StandardLibrary::FormatTo( log_info, "交易回报类型未知！{0} {1} {2}", fid_cxbz, fid_ywlb, fid_cjsl );
					m_trader_ape_p->LogPrint( basicx::syslog_level::c_error, log_info );
				}
			}
			else {
				FormatLibrary::StandardLibrary::FormatTo( log_info, "交易回报 查找 委托请求 失败！{0}", order_ref );
				m_trader_ape_p->LogPrint( basicx::syslog_level::c_warn, log_info );
			}
		} // for
	}
	else {
		return false;
	}

	return true;
}
