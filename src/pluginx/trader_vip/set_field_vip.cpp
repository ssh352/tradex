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

#include "set_field_vip.h"

SetField::SetField() {
	m_syslog = basicx::SysLog_S::GetInstance();

	m_map_set_field_func[120001] = &SetField::SetField_120001_620001;
	m_map_set_field_func[120002] = &SetField::SetField_120002_620021;
	m_map_set_field_func[120003] = &SetField::SetField_120003_620002;
	m_map_set_field_func[120004] = &SetField::SetField_120004_620022;
	m_map_set_field_func[130002] = &SetField::SetField_130002_630002;
	m_map_set_field_func[130004] = &SetField::SetField_130004_630004;
	m_map_set_field_func[130005] = &SetField::SetField_130005_630005;
	m_map_set_field_func[130006] = &SetField::SetField_130006_630006;
	m_map_set_field_func[130008] = &SetField::SetField_130008_601410;
	m_map_set_field_func[130009] = &SetField::SetField_130009_601411;
}

SetField::~SetField() {
}

bool SetField::SetField_120001_620001( int32_t api_session, Request* request ) { // ����ί���µ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			Fix_SetString( api_session, FID_GDH, request->m_req_json["holder"].asCString() ); // �ɶ��� Char 10
			Fix_SetString( api_session, FID_ZQDM, request->m_req_json["symbol"].asCString() ); // ֤ȯ���� Char 6
			std::string exchange = request->m_req_json["exchange"].asCString();
			Fix_SetString( api_session, FID_JYS, exchange.c_str() ); // ���������� Char 2
			int32_t entr_type = 0; // 1 �޼ۡ�2 �м�
			double price = 0.0;
			int32_t exch_side = request->m_req_json["exch_side"].asInt();
			if( 1 == exch_side || 2 == exch_side ) { // 1 ���롢2 ������29 �깺��30 ��ء�37 ���롢38 �ʳ�
				entr_type = request->m_req_json["entr_type"].asInt();
				if( 1 == entr_type ) { // �޼�
					entr_type = 0; // ������֤����֤��Ϊ 0 �޼�
				}
				else if( 2 == entr_type && "SH" == exchange ) { // �м�
					entr_type = 1; // ������֤Ϊ 1 �м�
				}
				else if( 2 == entr_type && "SZ" == exchange ) { // �м�
					entr_type = 104; // ������֤Ϊ 104 �м�
				}
				else {
					return false;
				}
				price = request->m_req_json["price"].asDouble();
			}
			Fix_SetLong( api_session, FID_DDLX, entr_type ); // �������� Int
			Fix_SetLong( api_session, FID_WTLB, exch_side ); // ί����� Int
			Fix_SetDouble( api_session, FID_WTJG, price ); // ί�м۸� Numric 9,3
			Fix_SetLong( api_session, FID_WTSL, request->m_req_json["amount"].asInt() ); // ί������ Int
			// FID_WTPCH ί�����κ� Int
			// FID_DFXW �Է�ϯλ�� Char 6
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_120002_620021( int32_t api_session, Request* request ) { // ����ί�г���
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			Fix_SetLong( api_session, FID_WTH, request->m_req_json["order_id"].asInt() ); // ԭί�к� Int
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_120003_620002( int32_t api_session, Request* request ) { // ����ί���µ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			Fix_SetLong( api_session, FID_COUNT, request->m_req_json["order_numb"].asInt() ); // ί�б��� Int
			Fix_SetString( api_session, FID_FJXX, request->m_req_json["order_list"].asCString() ); // ί����ϸ��Ϣ Char 15000
			// FID_WTPCH ί�����κ� Int
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_120004_620022( int32_t api_session, Request* request ) { // ����ί�г���
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			Fix_SetLong( api_session, FID_WTPCH, request->m_req_json["batch_id"].asInt() ); // ί�����κ� Int
			Fix_SetString( api_session, FID_EN_WTH, request->m_req_json["batch_ht"].asCString() ); // ����ί�кŷ�Χ Char 6000
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130002_630002( int32_t api_session, Request* request ) { // ��ѯ�ͻ��ʽ�
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130004_630004( int32_t api_session, Request* request ) { // ��ѯ�ͻ��ֲ�
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����

			// FID_GDH �ɶ��� Char 10
			// FID_JYS ���������� Char 2
			// FID_ZQDM ֤ȯ���� Char 6
			// FID_EXFLG ��ѯ��չ��Ϣ��־ Int
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130005_630005( int32_t api_session, Request* request ) { // ��ѯ�ͻ�����ί��
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			Fix_SetLong( api_session, FID_WTH, request->m_req_json["order_id"].asInt() ); // ί�к� Int
			Fix_SetString( api_session, FID_BROWINDEX, request->m_req_json["brow_index"].asCString() ); // ������ѯ����ֵ Char 128
			// FID_GDH �ɶ��� Char 10
			// FID_JYS ���������� Char 2
			// FID_WTLB ί����� Int
			// FID_ZQDM ֤ȯ���� Char 6
			// FID_WTPCH ί�����κ� Int
			// FID_FLAG ��ѯ��־��0 ����ί�У�1 �ɳ���ί�У�Int
			// FID_CXBZ ������־ Char 1
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130006_630006( int32_t api_session, Request* request ) { // ��ѯ�ͻ����ճɽ�
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			Fix_SetLong( api_session, FID_WTH, request->m_req_json["order_id"].asInt() ); // ί�к� Int
			Fix_SetString( api_session, FID_BROWINDEX, request->m_req_json["brow_index"].asCString() ); // ������ѯ����ֵ Char 128
			// FID_GDH �ɶ��� Char 10
			// FID_JYS ���������� Char 2
			// FID_WTLB ί����� Int
			// FID_ZQDM ֤ȯ���� Char 6
			// FID_WTPCH ί�����κ� Int
			// FID_FLAG ��ѯ��־��0 ����ί�У�1 �ɳ���ί�У�Int
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130008_601410( int32_t api_session, Request* request ) { // ��ѯETF������Ϣ
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Fix_SetString( api_session, FID_JJDM, request->m_req_json["fund_id_2"].asCString() ); // ������� Char 6
			// FID_JYS ������ Char 2
			// FID_SGDM ETF������� Char 6
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130009_601411( int32_t api_session, Request* request ) { // ��ѯETF�ɷֹ���Ϣ
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Fix_SetString( api_session, FID_JJDM, request->m_req_json["fund_id_2"].asCString() ); // ������� Char 6
			// FID_JYS ������ Char 2
			// FID_TYPE �������� Char 2
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}
