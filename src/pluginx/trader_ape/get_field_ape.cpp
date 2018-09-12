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

#include "get_field_ape.h"

GetField::GetField() {
	m_syslog = basicx::SysLog_S::GetInstance();

	m_map_get_field_func[120001] = &GetField::GetField_120001_204501;
	m_map_get_field_func[120002] = &GetField::GetField_120002_204502;
	m_map_get_field_func[120003] = &GetField::GetField_120003_204513;
	m_map_get_field_func[120004] = &GetField::GetField_120004_204511;
	m_map_get_field_func[120005] = &GetField::GetField_120005_204545;
	m_map_get_field_func[120006] = &GetField::GetField_120006_204546;
	m_map_get_field_func[130002] = &GetField::GetField_130002_303002;
	m_map_get_field_func[130004] = &GetField::GetField_130004_304101;
	m_map_get_field_func[130005] = &GetField::GetField_130005_304103;
	m_map_get_field_func[130006] = &GetField::GetField_130006_304110; // Ӧʹ�� 304110 ���� 304109 ���� security_type��trans_id ��ֵ
	m_map_get_field_func[130008] = &GetField::GetField_130008_104105;
	m_map_get_field_func[130009] = &GetField::GetField_130009_104106;
}

GetField::~GetField() {
}

// ��ʹ�� VIP ��ͬ��ҵ��ִ�г���ʱ ret_numb == 1������Ӱ�� FillHead() �� ret_numb ��ֵ��Ҳ��Ӱ���µ����������� int32_t i = ret_numb == 0 ? -1 : 0; ���

void GetField::FillHead( Json::Value& results_json, int32_t ret_func, int32_t ret_numb, Request* request ) {
	results_json["ret_func"] = ret_func;
	results_json["ret_code"] = 0;
	results_json["ret_info"] = basicx::StringToUTF8( "ҵ���ύ�ɹ���" );
	results_json["ret_task"] = request->m_req_json["task_id"].asInt();
	results_json["ret_last"] = true;
	results_json["ret_numb"] = ret_numb == 0 ? 1 : ret_numb;
	//results_json["ret_data"] = "";
}

void GetField::FillHeadQuery( Json::Value& results_json, int32_t ret_func, int32_t ret_numb, Request* request ) {
	results_json["ret_func"] = ret_func;
	results_json["ret_code"] = 0;
	if( ret_numb > 0 ) {
		results_json["ret_info"] = basicx::StringToUTF8( "ҵ���ύ�ɹ���" );
	}
	else {
		results_json["ret_info"] = basicx::StringToUTF8( "ҵ���ύ�ɹ����޽�����ݣ�" );
	}
	results_json["ret_task"] = request->m_req_json["task_id"].asInt();
	results_json["ret_last"] = true;
	results_json["ret_numb"] = ret_numb;
	//results_json["ret_data"] = "";
}

bool GetField::GetField_120001_204501( int32_t api_session, Request* request, std::string& results ) { // ����ί���µ� // ����ͬ VIP �ӿ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // �� VIP ��ͬ��ҵ��ִ�г���ʱ ret_numb == 1
			FillHead( results_json, 120001, ret_numb, request ); // 120001 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int // �ڷ�� CheckTradeResultForRisk() �м��
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
				ret_data_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // ί�к� Int
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_120002_204502( int32_t api_session, Request* request, std::string& results ) { // ����ί�г��� // ������� VIP �ӿ����� FID_WTH
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // �� VIP ��ͬ��ҵ��ִ�г���ʱ ret_numb == 1
			FillHead( results_json, 120002, ret_numb, request ); // 120002 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int // �ڷ�� CheckTradeResultForRisk() �м��
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
				ret_data_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // ί�к�(����) Int // ���ԣ�0
//				ret_data_json["order_id"] = 0;
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_120003_204513( int32_t api_session, Request* request, std::string& results ) { // ����ί���µ� // ����ͬ VIP �ӿ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // �� VIP ��ͬ��ҵ��ִ�г���ʱ ret_numb == 1
			FillHead( results_json, 120003, ret_numb, request ); // 120003 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int // �ڷ�� CheckTradeResultForRisk() �м��
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
				ret_data_json["batch_id"] = Fix_GetLong( api_session, FID_WTPCH, i ); // ί�����κ� Int
				ret_data_json["batch_ht"] = Fix_GetItem( api_session, FID_EN_WTH, m_field_value_huge, FIELD_VALUE_HUGE, i ); // ί�к�ͬ�� Char 6000
				// FID_COUNT ί�гɹ����� Int
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_120004_204511( int32_t api_session, Request* request, std::string& results ) { // ����ί�г��� // ����ͬ VIP �ӿ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // �� VIP ��ͬ��ҵ��ִ�г���ʱ ret_numb == 1
			FillHead( results_json, 120004, ret_numb, request ); // 120004 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int // �ڷ�� CheckTradeResultForRisk() �м��
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
				// FID_SBBS 1297 N �ɹ����볷���ı���
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_120005_204545( int32_t api_session, Request* request, std::string& results ) { // �۹�ͨ����ί�� // �� GetField_120005_204545 ����ί���µ� һ��
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // �� VIP ��ͬ��ҵ��ִ�г���ʱ ret_numb == 1
			FillHead( results_json, 120005, ret_numb, request ); // 120005 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int // �ڷ�� CheckTradeResultForRisk() �м��
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
				ret_data_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // ί�к� Int
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_120006_204546( int32_t api_session, Request* request, std::string& results ) { // �۹�ͨί�г��� // �� GetField_120006_204546 ����ί�г��� һ��
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // �� VIP ��ͬ��ҵ��ִ�г���ʱ ret_numb == 1
			FillHead( results_json, 120006, ret_numb, request ); // 120006 // FillHead
			int32_t i = ret_numb == 0 ? -1 : 0; // ���ף�ҵ��ʧ����ȡ��һ��
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int // �ڷ�� CheckTradeResultForRisk() �м��
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
				ret_data_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // ί�к�(����) Int // ���ԣ�0
//				ret_data_json["order_id"] = 0;
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130002_303002( int32_t api_session, Request* request, std::string& results ) { // ��ѯ�ͻ��ʽ� // ������� VIP �ӿ����� FID_CODE��FID_MESSAGE
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130002, ret_numb, request ); // 130002 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int
//				ret_data_json["otc_code"] = 0;
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
//				ret_data_json["otc_info"] = "";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["account"] = Fix_GetItem( api_session, FID_ZJZH, m_field_value_short, FIELD_VALUE_SHORT, i ); // �ʽ��˺� Char 20
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["currency"] = Fix_GetItem( api_session, FID_BZ, m_field_value_short, FIELD_VALUE_SHORT, i ); // ���� Char  3
				ret_data_json["available"] = Fix_GetDouble( api_session, FID_KYZJ, i ); // �����ʽ� Numric 16,2 // FID_EXFLG = 1 ʱ��Ч
				ret_data_json["balance"] = Fix_GetDouble( api_session, FID_ZHYE, i ); // �˻���� Numric 16,2
				ret_data_json["frozen"] = Fix_GetDouble( api_session, FID_DJJE, i ); // ������ Numric 16,2
				// FID_ZZHBZ 727 N ���˻���־ // 1 ��ʾ���˻�
				// FID_ZHLB 929 N �˻���� // 1 ��ͨ�ʻ���2 ������ȯ�ʻ�
				// FID_ZHZT 710 N �˻�״̬ // 0 ����������Ϊ�쳣
				// FID_KQZJ 617 R ��ȡ�ʽ� // FID_EXFLG = 1 ʱ��Ч
				// FID_OFSS_JZ 2100 R ������ֵ // FID_EXFLG = 1 ʱ��Ч
				// FID_ZXSZ 760 R	 ֤ȯ��ֵ // FID_EXFLG = 1 ʱ��Ч
				// FID_QTZC 1254 R �����ʲ� // FID_EXFLG = 1 ʱ��Ч
				// FID_ZZC 1255 R ���ʲ� // FID_EXFLG = 1 ʱ��Ч
				// FID_KYZJ2 1656 R �۹ɿ����ʽ� // FID_EXFLG = 1 ʱ��Ч
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130004_304101( int32_t api_session, Request* request, std::string& results ) { // ��ѯ�ͻ��ֲ� // ������� VIP �ӿ����� FID_CODE��FID_MESSAGE��FID_ZQLB��FID_DRMCCJJE��FID_DRMRCJJE��FID_SGCJSL��FID_SHCJSL
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130004, ret_numb, request ); // 130004 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int
//				ret_data_json["otc_code"] = 0;
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
//				ret_data_json["otc_info"] = "";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["holder"] = Fix_GetItem( api_session, FID_GDH, m_field_value_short, FIELD_VALUE_SHORT, i ); // �ɶ��� Char 10
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, FID_JYS, m_field_value_short, FIELD_VALUE_SHORT, i ); // ���������� Char 2 // "SH"��"SZ"��"HK"
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["currency"] = Fix_GetItem( api_session, FID_BZ, m_field_value_short, FIELD_VALUE_SHORT, i ); // ���� Char 3
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["symbol"] = Fix_GetItem( api_session, FID_ZQDM, m_field_value_short, FIELD_VALUE_SHORT, i ); // ֤ȯ���� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_type"] = Fix_GetItem( api_session, FID_ZQLB, m_field_value_short, FIELD_VALUE_SHORT, i ); // ֤ȯ��� Char 2 // ���ԣ���������ȡ
//				ret_data_json["security_type"] = "A0";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_name"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_ZQMC, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ֤ȯ���� Char 8
				ret_data_json["security_qty"] = Fix_GetLong( api_session, FID_JCCL, i ); // ��ֲ��� Long //��Ҫ��֤ȯ����
				ret_data_json["can_sell"] = Fix_GetLong( api_session, FID_KMCSL, i ); // ���������� Long
				ret_data_json["can_sub"] = Fix_GetLong( api_session, FID_KSGSL, i ); // ���깺���� Long
				ret_data_json["can_red"] = Fix_GetLong( api_session, FID_KSHSL, i ); // ��������� Long
				ret_data_json["non_tradable"] = Fix_GetLong( api_session, FID_FLTSL, i ); // ����ͨ���� Long
				ret_data_json["frozen_qty"] = Fix_GetLong( api_session, FID_DJSL, i ); // �������� Long
				ret_data_json["sell_qty"] = Fix_GetLong( api_session, FID_DRMCCJSL, i ); // ���������ɽ����� Long
				ret_data_json["sell_money"] = Fix_GetDouble( api_session, FID_DRMCCJJE, i ); // ���������ɽ���� Numric 16,2 // ���ԣ���������ȡ
//				ret_data_json["sell_money"] = 0.0;
				ret_data_json["buy_qty"] = Fix_GetLong( api_session, FID_DRMRCJSL, i ); // ��������ɽ����� Long
				ret_data_json["buy_money"] = Fix_GetDouble( api_session, FID_DRMRCJJE, i ); // ��������ɽ���� Numric 16,2 // ���ԣ���������ȡ
//				ret_data_json["buy_money"] = 0.0;
				ret_data_json["sub_qty"] = Fix_GetLong( api_session, FID_SGCJSL, i ); // �깺�ɽ����� Long // ���ԣ���δ����
//				ret_data_json["sub_qty"] = 0;
				ret_data_json["red_qty"] = Fix_GetLong( api_session, FID_SHCJSL, i ); // ��سɽ����� Long // ���ԣ���δ����
//				ret_data_json["red_qty"] = 0;
				// FID_ZQSL 724 N ����
				// FID_DRMCWTSL 543 N ������������
				// FID_WJSSL 758 N �ǽ�������
				// FID_JGSL 1722 N �ѽ�������
				// FID_KCRQ 603 N ��������
				// FID_ZXSZ 760 R ������ֵ // FID_EXFLG = 1 ʱ��Ч
				// FID_JYDW 591 N ������λ����ծȯ��Ч��1 ��ʾ�ţ�10 ��ʾ�� // FID_EXFLG = 1 ʱ��Ч
				// FID_ZXJ 729 R ���¼� // FID_EXFLG = 1 ʱ��Ч
				// FID_LXBJ 961 R ��Ϣ���ۣ���ծȯ��Ч // FID_EXFLG = 1 ʱ��Ч
				// FID_MRJJ 636 R ������� // FID_EXFLG = 1 ʱ��Ч
				// FID_CCJJ 1209 R �ɱ���(������Ӱ��ɱ��۵��㷨)���ɱ��㷨һ // FID_EXFLG = 1 ʱ��Ч
				// FID_BBJ 1098 R ������(�����˰���ǰ���������ķ���) // FID_EXFLG = 1 ʱ��Ч
				// FID_FDYK 761 R ����ӯ��(�м��뱣���۵ļ�����) // FID_EXFLG = 1 ʱ��Ч
				// FID_LJYK 1210 R �ۼ�ӯ��(δ�����ĸ���ӯ��+��������ʵ�ֵ�ӯ��) // FID_EXFLG = 1 ʱ��Ч
				// FID_TBCBJ 671 R ̯���ɱ���(����Ҫ̯���ɱ����㷨)���ɱ��㷨�� // FID_EXFLG = 1 ʱ��Ч
				// FID_TBBBJ 748 R ̯��������(�����˵�ǰ���������ķ���) // FID_EXFLG = 1 ʱ��Ч
				// FID_TBFDYK 749 R ̯������ӯ�� // FID_EXFLG = 1 ʱ��Ч
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130005_304103( int32_t api_session, Request* request, std::string& results ) { // ��ѯ�ͻ�����ί�� // ������� VIP �ӿ����� FID_CODE��FID_MESSAGE��FID_ZQLB��FID_DJZJ��FID_CJSJ��FID_ZJZH
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130005, ret_numb, request ); // 130005 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int
//				ret_data_json["otc_code"] = 0;
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
//				ret_data_json["otc_info"] = "";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["holder"] = Fix_GetItem( api_session, FID_GDH, m_field_value_short, FIELD_VALUE_SHORT, i ); // �ɶ��� Char 10
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, FID_JYS, m_field_value_short, FIELD_VALUE_SHORT, i ); // ���������� Char 2 // "SH"��"SZ"��"HK"
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["currency"] = Fix_GetItem( api_session, FID_BZ, m_field_value_short, FIELD_VALUE_SHORT, i ); // ���� Char 3
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["symbol"] = Fix_GetItem( api_session, FID_ZQDM, m_field_value_short, FIELD_VALUE_SHORT, i ); // ֤ȯ���� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_type"] = Fix_GetItem( api_session, FID_ZQLB, m_field_value_short, FIELD_VALUE_SHORT, i ); // ֤ȯ��� Char 2 // ���ԣ���������ȡ
//				ret_data_json["security_type"] = "A0";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_name"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_ZQMC, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ֤ȯ���� Char 8
				ret_data_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // ί�к� Int
				// ����û�� FID_DDJYXZ ��������ָ������ �ֶΣ��۹�ί�в�ѯ�õ��� FID_DDLX ��Ϊ�㣬���Ը۹�ͳһ��Ϊ �޼� ����
				int64_t entr_type = Fix_GetLong( api_session, FID_DDLX, i ); // �������� Int
				if( 0 == entr_type ) { // �޼�
					ret_data_json["entr_type"] = 1; // ������֤����֤��Ϊ 0 �޼�
				}
				else if( 1 == entr_type || 104 == entr_type ) { // �м�
					ret_data_json["entr_type"] = 2; // ������֤Ϊ 1 �мۣ�������֤Ϊ 104 �м�
				}
				else {
					return false;
				}
				ret_data_json["exch_side"] = Fix_GetLong( api_session, FID_WTLB, i ); // ί����� Int
				ret_data_json["price"] = Fix_GetDouble( api_session, FID_WTJG, i ); // ί�м۸� Nurmic 9,3
				ret_data_json["amount"] = Fix_GetLong( api_session, FID_WTSL, i ); // ί������ Int
				ret_data_json["fill_price"] = Fix_GetDouble( api_session, FID_CJJG, i ); // �ɽ��۸� Numric 9,3
				ret_data_json["fill_amount"] = Fix_GetLong( api_session, FID_CJSL, i ); // �ɽ����� Int
				ret_data_json["fill_money"] = Fix_GetDouble( api_session, FID_CJJE, i ); // �ɽ���� Numric 16,2
				ret_data_json["report_ret"] = Fix_GetLong( api_session, FID_SBJG, i ); // �걨��� Int
				ret_data_json["cxl_qty"] = Fix_GetLong( api_session, FID_CDSL, i ); // �������� Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["cxl_flag"] = Fix_GetItem( api_session, FID_CXBZ, m_field_value_short, FIELD_VALUE_SHORT, i ); // ������־ Char 1
				ret_data_json["frozen"] = Fix_GetDouble( api_session, FID_DJZJ, i ); // �����ʽ� Numric 16,2 // ���ԣ���������ȡ
//				ret_data_json["frozen"] = 0.0;
				ret_data_json["settlement"] = Fix_GetDouble( api_session, FID_QSZJ, i ); // �����ʽ� Numric 16,2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["brow_index"] = Fix_GetItem( api_session, FID_BROWINDEX, m_field_value_short, FIELD_VALUE_SHORT, i ); // ������ѯ����ֵ Char 128
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["report_time"] = Fix_GetItem( api_session, FID_SBSJ, m_field_value_short, FIELD_VALUE_SHORT, i ); // �걨ʱ�� Char 8
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["order_time"] = Fix_GetItem( api_session, FID_WTSJ, m_field_value_short, FIELD_VALUE_SHORT, i ); // ί��ʱ�� Char 8
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fill_time"] = Fix_GetItem( api_session, FID_CJSJ, m_field_value_short, FIELD_VALUE_SHORT, i ); // �ɽ�ʱ�� Char 8 // ���ԣ���������ȡ
//				ret_data_json["fill_time"] = "";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["account"] = Fix_GetItem( api_session, FID_ZJZH, m_field_value_short, FIELD_VALUE_SHORT, i ); // �ʽ��˺� Char 20 // ���ԣ�""
//				ret_data_json["account"] = "";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["message"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_JGSM, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ���˵�� Char 64
				// FID_NODE 641 C ί��վ����Ϣ
				// FID_WTPCH 1017 C ���κ�
				// FID_MRSL1 843 N ͶƱ�޳�����
				// FID_MRSL2 845 N ͶƱ��������
				// FID_MRSL3 847 N ͶƱ��Ȩ����
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130006_304110( int32_t api_session, Request* request, std::string& results ) { // ��ѯ�ͻ����ճɽ� // ������� VIP �ӿ����� FID_CODE��FID_MESSAGE��FID_ZQLB��FID_S1��FID_ZJZH
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130006, ret_numb, request ); // 130006 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int
//				ret_data_json["otc_code"] = 0;
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
//				ret_data_json["otc_info"] = "";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["holder"] = Fix_GetItem( api_session, FID_GDH, m_field_value_short, FIELD_VALUE_SHORT, i ); // �ɶ��� Char 10
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, FID_JYS, m_field_value_short, FIELD_VALUE_SHORT, i ); // ���������� Char 2 // "SH"��"SZ"��"HK"
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["currency"] = Fix_GetItem( api_session, FID_BZ, m_field_value_short, FIELD_VALUE_SHORT, i ); // ���� Char 3
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["symbol"] = Fix_GetItem( api_session, FID_ZQDM, m_field_value_short, FIELD_VALUE_SHORT, i ); // ֤ȯ���� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_type"] = Fix_GetItem( api_session, FID_ZQLB, m_field_value_short, FIELD_VALUE_SHORT, i ); // ֤ȯ��� Char 2
//				ret_data_json["security_type"] = "A0"; // ���ԣ���������ȡ
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["security_name"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_ZQMC, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ֤ȯ���� Char 8
				ret_data_json["order_id"] = Fix_GetLong( api_session, FID_WTH, i ); // ί�к� Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["trans_id"] = Fix_GetItem( api_session, FID_CJBH, m_field_value_short, FIELD_VALUE_SHORT, i ); // �ɽ���� Char 16
				ret_data_json["exch_side"] = Fix_GetLong( api_session, FID_WTLB, i ); // ί����� Int
				ret_data_json["fill_price"] = Fix_GetDouble( api_session, FID_CJJG, i ); // �ɽ��۸� Numric 9,3
				ret_data_json["fill_amount"] = Fix_GetLong( api_session, FID_CJSL, i ); // �ɽ����� Int
				ret_data_json["fill_money"] = Fix_GetDouble( api_session, FID_CJJE, i ); // �ɽ���� Numric 16,2
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["cxl_flag"] = Fix_GetItem( api_session, FID_CXBZ, m_field_value_short, FIELD_VALUE_SHORT, i ); // ������־ Char 1
				ret_data_json["settlement"] = Fix_GetDouble( api_session, FID_QSJE, i ); // ������ Numric 16,2
				ret_data_json["commission"] = Fix_GetDouble( api_session, FID_S1, i ); // Ӷ�� Numric 16,2 // ���ԣ�0.0
//				ret_data_json["commission"] = 0.0;
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["brow_index"] = Fix_GetItem( api_session, FID_BROWINDEX, m_field_value_short, FIELD_VALUE_SHORT, i ); // ������ѯ����ֵ Char 128
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["account"] = Fix_GetItem( api_session, FID_ZJZH, m_field_value_short, FIELD_VALUE_SHORT, i ); // �ʽ��˺� Char 20 // ���ԣ�""
//				ret_data_json["account"] = "";
				// FID_SBWTH 1011 C �걨ί�к�
				// FID_CJSJ 527 C �ɽ�ʱ��
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130008_104105( int32_t api_session, Request* request, std::string& results ) { // ��ѯETF������Ϣ // ������� VIP �ӿ����� FID_CODE��FID_MESSAGE��FID_LOGICAL��FID_DWJZ
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130008, ret_numb, request ); // 130008 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int
//				ret_data_json["otc_code"] = 0;
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
//				ret_data_json["otc_info"] = "";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fund_name"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_JJMC, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ETF�������� Char 8
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fund_id_1"] = Fix_GetItem( api_session, FID_SGDM, m_field_value_short, FIELD_VALUE_SHORT, i ); // ETF������� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fund_id_2"] = Fix_GetItem( api_session, FID_JJDM, m_field_value_short, FIELD_VALUE_SHORT, i ); // ETF������� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, FID_JYS, m_field_value_short, FIELD_VALUE_SHORT, i ); // ������ Char 2
				ret_data_json["count"] = Fix_GetLong( api_session, FID_COUNT, i ); // ��Ʊ��¼�� Int
				ret_data_json["status"] = Fix_GetLong( api_session, FID_SGSHZT, i ); // ��������״̬ Int // 0 ��ֹ���꣬1 ��������
				ret_data_json["pub_iopv"] = Fix_GetLong( api_session, FID_LOGICAL, i ); // �Ƿ񷢲�IOPV Int // ���ԣ���������ȡ
//				ret_data_json["pub_iopv"] = 0;
				ret_data_json["unit"] = Fix_GetLong( api_session, FID_TZDW, i ); // ��С�깺��ص�λ Int
				ret_data_json["cash_ratio"] = Fix_GetDouble( api_session, FID_XJTDBL, i ); // ����ֽ�������� Numeric 7,5
				ret_data_json["cash_diff"] = Fix_GetDouble( api_session, FID_XJCE, i ); // T���ֽ��� Numeric 11,2
				ret_data_json["iopv"] = Fix_GetDouble( api_session, FID_DWJZ, i ); // T-1�ջ���λ��ֵ Numeric 8,4 // ���ԣ�0.0
//				ret_data_json["iopv"] = 0.0;
				ret_data_json["trade_iopv"] = Fix_GetDouble( api_session, FID_SGSHDWJZ, i ); // T-1�����굥λ��ֵ Numeric 12,2
				// FID_RGDM 1394 C �Ϲ�����
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool GetField::GetField_130009_104106( int32_t api_session, Request* request, std::string& results ) { // ��ѯETF�ɷֹ���Ϣ // ������� VIP �ӿ����� FID_CODE��FID_MESSAGE
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Json::Value results_json;
			int32_t ret_numb = Fix_GetCount( api_session ); // ҵ��ִ�г���ʱ ret_numb == 0
			FillHeadQuery( results_json, 130009, ret_numb, request ); // 130009 // FillHeadQuery
			int32_t i = 0; // ��ѯ��ҵ��ʧ����ȡ�������
			for( ; i < ret_numb; i++ ) {
				Json::Value ret_data_json;
				ret_data_json["otc_code"] = Fix_GetLong( api_session, FID_CODE, i ); // ������ Int
//				ret_data_json["otc_code"] = 0;
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["otc_info"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_MESSAGE, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ����˵�� Char 255
//				ret_data_json["otc_info"] = "";
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["fund_name"] = Fix_GetItem( api_session, FID_JJDM, m_field_value_short, FIELD_VALUE_SHORT, i ); // ETF������� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["stock_code"] = Fix_GetItem( api_session, FID_ZQDM, m_field_value_short, FIELD_VALUE_SHORT, i ); // ETF�ɷݹɴ��� Char 6
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["stock_name"] = basicx::StringToUTF8( Fix_GetItem( api_session, FID_ZQMC, m_field_value_short, FIELD_VALUE_SHORT, i ) ); // ETF�ɷݹ����� Char 8
				ret_data_json["stock_qty"] = Fix_GetLong( api_session, FID_ZQSL, i ); // ETF�ɷݹ����� Int
				memset( m_field_value_short, 0, FIELD_VALUE_SHORT );
				ret_data_json["exchange"] = Fix_GetItem( api_session, FID_JYS, m_field_value_short, FIELD_VALUE_SHORT, i ); // ������ Char 2
				ret_data_json["replace_flag"] = Fix_GetLong( api_session, FID_TDBZ, i ); // �����־ Int // 0 ��Ʊ��1 �����ֽ�2 �����ֽ�3 �ֽ����
				ret_data_json["replace_money"] = Fix_GetDouble( api_session, FID_TDJE, i ); // ������  Numeric 16,2
				ret_data_json["up_px_ratio"] = Fix_GetDouble( api_session, FID_YJBL, i ); // ��۱���  Numeric 16,2
				results_json["ret_data"].append( ret_data_json );
			}
			results = Json::writeString( m_json_writer, results_json );
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}
